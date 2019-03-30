#include "statement.h"
#include "escaping/escape_sequences.h"
#include "win/version.h"
#include "platform.h"
#include <Poco/Base64Encoder.h>
#include <Poco/Exception.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include "string.h"

Statement::Statement(Connection & conn_) : connection(conn_), metadata_id(conn_.environment.metadata_id) {
    ard.reset(new DescriptorClass);
    apd.reset(new DescriptorClass);
    ird.reset(new DescriptorClass);
    ipd.reset(new DescriptorClass);
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

void Statement::composeRequest(Poco::Net::HTTPRequest &request, bool meta_mode) {
	std::ostringstream user_password_base64;
    Poco::Base64Encoder base64_encoder(user_password_base64, Poco::BASE64_URL_ENCODING);
    base64_encoder << connection.user << ":" << connection.password;
    base64_encoder.close();

    request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
    request.setVersion(Poco::Net::HTTPRequest::HTTP_1_1);
    request.setKeepAlive(true);
    request.setChunkedTransferEncoding(true);
    request.setCredentials("Basic", user_password_base64.str());
    Poco::URI uri(connection.url);
	if(meta_mode){
		if(!connection.tables.empty()){
			std::string encoded;
			Poco::URI::encode(connection.tables, "", encoded);
			uri.addQueryParameter("tables",encoded);
		}
			
		if(connection.expand_tags)
			uri.addQueryParameter("expandTags", "true");
		if(connection.meta_columns)
			uri.addQueryParameter("metaColumns", "true");
	}
	
	#if !defined(UNICODE)
	{
		if(connection.environment.code_page > 0){
			std::string contentType = "text/plain; charset=cp" + std::to_string(connection.environment.code_page);
			request.setContentType(contentType);
			uri.addQueryParameter("codePage", std::to_string(connection.environment.code_page));
		}
	    std::string encoded;
		Poco::URI::encode(connection.environment.locale, "", encoded);
		uri.addQueryParameter("locale", encoded);
 	}
	#endif
    
	std::string path = meta_mode ? connection.meta_path : connection.path;
	// quick client start
	path += "/quick";
	// quick client end
    request.setURI(path + "?" + uri.getQuery()); /// TODO escaping
    request.set("User-Agent", "atsd-odbc/" VERSION_STRING " (" CMAKE_SYSTEM ")"
#if defined(UNICODE)
        " UNICODE"
#endif
    );
	LOG(request.getMethod() << " " << connection.session->getHost() << request.getURI() << " Content Type=" << request.getContentType() <<  " body=" << prepared_query);

}

void Statement::processInsert() {
	if(!out || !out->good()){
		    Poco::Net::HTTPRequest request;
			composeRequest(request);
			if (in && in->peek() != EOF || out){
				connection.session->reset();
			}
			
		try {
            out = &connection.session->sendRequest(request);
			converter = std::unique_ptr<Poco::OutputStreamConverter>(new Poco::OutputStreamConverter(*out, windows1251, utf8));
        } catch (const Poco::IOException & e) {
            connection.session->reset();
            LOG("Http request failed: " << e.what() << ": " << e.message());
            throw;
        }
	}
	try{
		LOG("Sending insert: " << prepared_query);
		*converter << prepared_query << "\n";
		converter->flush();
		out->flush();
	} catch(std::exception &e) {
		std::stringstream error_message;
		error_message << "Http request failed: " << e.what();
		try{
			response = std::make_unique<Poco::Net::HTTPResponse>();
			in = &connection.session->receiveResponse(*response);
			Poco::Net::HTTPResponse::HTTPStatus status = response->getStatus();
			if (status != Poco::Net::HTTPResponse::HTTP_OK) {
				error_message << std::endl << "HTTP status code: " << status << std::endl << "Received error:" << std::endl << in->rdbuf() << std::endl;
			}
		} catch(...) {
			
		}	
		connection.session->reset();
		throw std::runtime_error(error_message.str());
	}
}

void Statement::sendRequest(IResultMutatorPtr mutator, bool meta_mode) {
	std::string insert_keyword = "INSERT";
	std::string update_keyword = "UPDATE";
	std::string delete_keyword = "DELETE";
	bool insert = !meta_mode && (
	!strnicmp(insert_keyword.c_str() , prepared_query.c_str(), 6) 
	|| !strnicmp(update_keyword.c_str() , prepared_query.c_str(), 6)
	|| !strnicmp(delete_keyword.c_str() , prepared_query.c_str(), 6)
	);
	if(insert) {
		processInsert();
		return;
	}
	
    Poco::Net::HTTPRequest request;
	composeRequest(request, meta_mode);
	
    if (in && in->peek() != EOF || out){
		connection.session->reset();
		out = nullptr;
	}
        
    // Send request to server with finite count of retries.
    for (int i = 1;; ++i) {
        try {
            connection.session->sendRequest(request) << prepared_query;
            response = std::make_unique<Poco::Net::HTTPResponse>();
            in = &connection.session->receiveResponse(*response);
            break;
        } catch (const Poco::IOException & e) {
            connection.session->reset(); // reset keepalived connection

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
}
