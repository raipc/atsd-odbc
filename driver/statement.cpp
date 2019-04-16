#include "statement.h"
#include "escaping/escape_sequences.h"
#include "win/version.h"
#include "platform.h"
#include <Poco/Base64Encoder.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include "string.h"

Statement::Statement(Connection & conn_) : connection(conn_), metadata_id(conn_.environment.metadata_id), utf8(), windows1251(), textConverter(windows1251, utf8) {
    ard.reset(new DescriptorClass);
    apd.reset(new DescriptorClass);
    ird.reset(new DescriptorClass);
    ipd.reset(new DescriptorClass);
}

Statement::~Statement(){
    if (longRunningConnection) {
        longRunningConnection->close();
    }
}

bool Statement::getScanEscapeSequences() const {
    return scan_escape_sequences;
}

void Statement::setScanEscapeSequences(bool value) {
    scan_escape_sequences = value;
}

SQLUINTEGER Statement::getMetadataId() const {
    return metadata_id;
}

void Statement::setMetadataId(SQLUINTEGER id) {
    metadata_id = id;
}

const std::string Statement::getQuery() const {
    return query;
}

const TypeInfo & Statement::getTypeInfo(const std::string & type_name) const {
    return connection.environment.types_info.at(type_name);
}

bool Statement::isEmpty() const {
    return query.empty();
}

bool Statement::isPrepared() const {
    return prepared;
}


void Statement::sleep() {
	if(connection.sleep){
		LOG("Waiting for " << SLEEP_SECONDS << "s");
		std::this_thread::sleep_for (std::chrono::seconds(SLEEP_SECONDS));
	}
}

void Statement::obtainLongRunningConnection(const std::string &query) {
    if (!longRunningConnection || longRunningConnection->isClosed()) {
        bool txAllTable = query.find("quik_tx_all") != std::string::npos;
        if(txAllTable){
            longRunningConnection.reset(connection.createHttpConnection());
        } else {
            longRunningConnection.reset(connection.createWebSocket());
        }
    }
}

void Statement::execute() {
    LOG("Statement: " << this << ":" << (&connection) << " query: " << prepared_query);
    for (int i = 1;; ++i) {
        try {
            sleep();
            obtainLongRunningConnection(prepared_query);
            std::string utfString;
            textConverter.convert(prepared_query, utfString);
            longRunningConnection->send(utfString);
            longRunningConnection->checkError();
            connection.sleep = false;
            break;
        } catch (const Poco::Exception &e) {
            connection.sleep = i > SLEEP_AFTER_TRIES;
            if (longRunningConnection) {
                longRunningConnection->close();
            }
            std::stringstream error_message;
            error_message << "Long running connection failed: " << e.what() << ": " << e.message();
            LOG("try=" << i << " " << error_message.str());
            if (i > connection.retry_count) {
                throw std::runtime_error(error_message.str());
            }
        } catch (const std::exception &e) {
            LOG("Error: " << e.what());
            connection.sleep = true;
            throw;
        }
    }
}

void Statement::sendRequest(IResultMutatorPtr mutator, bool meta_mode) {
    Poco::Net::HTTPRequest request;
	connection.composeRequest(request, meta_mode);

    if (in && in->peek() != EOF) {
		connection.session->reset();
	}
        
    // Send request to server with finite count of retries.
    for (int i = 1;; ++i) {
        try {
            sleep();
			std::string utfString;
			textConverter.convert(prepared_query, utfString);
            connection.session->sendRequest(request) << utfString;
            response = std::make_unique<Poco::Net::HTTPResponse>();
            in = &connection.session->receiveResponse(*response);
			connection.sleep = false;
            break;
        } catch (const Poco::Exception &e) {
            connection.session->reset(); // reset keepalived connection
            connection.sleep = i > SLEEP_AFTER_TRIES;
            LOG("Http request try=" << i << "/" << connection.retry_count << " failed: " << e.what() << ": " << e.message());
            if (i > connection.retry_count) {
                throw;
            }
        }
    }

    Poco::Net::HTTPResponse::HTTPStatus status = response->getStatus();

    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
        std::stringstream error_message;
        error_message << "HTTP status code: " << status << std::endl << "Received error:" << std::endl << in->rdbuf() << std::endl;
        LOG(error_message.str());
        throw std::runtime_error(error_message.str());
    }

    result.init(this, std::move(mutator));
}

bool Statement::fetchRow() {
    current_row = result.fetch();
    return current_row.isValid();
}

void Statement::prepareQuery(const std::string & q) {
    query = q;
    if (scan_escape_sequences) {
        prepared_query = replaceEscapeSequences(query);
        if(query != prepared_query){
            LOG("Escaped: " << query << " : " << prepared_query);
        }
    } else {
        prepared_query = q;
    }

    prepared = true;
}

void Statement::setQuery(const std::string & q) {
    query = q;
    prepared_query = q;
}

void Statement::reset() {
    in = nullptr;
    response.reset();
    connection.session->reset();
    diagnostic_record.reset();
    result = ResultSet();

    ard.reset(new DescriptorClass);
    apd.reset(new DescriptorClass);
    ird.reset(new DescriptorClass);
    ipd.reset(new DescriptorClass);
    if(longRunningConnection){
        longRunningConnection->close();
        longRunningConnection->checkError();
    }
}
