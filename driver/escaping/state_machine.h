#pragma once

#include <string>
#include "string_view.h"
#include "lexer.h"
#include <regex>
#include <list>
#include <algorithm>


#define DECLARE2(NAME, IGNORE) Token::NAME
const std::list<Token> function_list{
#include "function_declare.h"
};
#undef DECLARE2

const std::regex COLUMN_REGEX(".*\\.?\"?(entity|tags|metric|text|value|time|datatime)\\.?.*\"?");
const std::regex COLUMN_WITH_POINT_REGEX("\"(entity|tags|metric)\\..+\"");
const std::list<std::string> TABLE_KEYWORDS_LIST = { //list of keywords after which tables and there allies are declared
        "FROM",
        "JOIN"
};

bool inTableKeywordsList(const std::string& literal) {
    return std::find(TABLE_KEYWORDS_LIST.begin(), TABLE_KEYWORDS_LIST.end(), literal) == TABLE_KEYWORDS_LIST.end();
};

bool matchesColumnRegex(const std::string& literal) {
    return std::regex_match(literal, COLUMN_REGEX);
};

class BaseState {
    public:
        explicit BaseState(const Token& token, StateMachine& stateMachine): token(token), stateMachine(stateMachine) {};
        virtual std::string convert() {
            return token.literal.to_string();
        };
        virtual BaseState& nextState(const Token& nextToken) = 0;
    protected:
        const Token token;
        StateMachine stateMachine;
};

class IntermediateState : BaseState { //techincal state for situations when state cannot be changed with one token and requires one more. e.g. "AS" token
    public:
        explicit IntermediateState(const Token& token, StateMachine& stateMachine, BaseState* parentState): BaseState(token, stateMachine), parentState(parentState) {};
        BaseState& nextState(const Token& nextToken) {
            return (BaseState&) parentState->nextState(nextToken);
        }
    private:
        BaseState* parentState;
};

class SelectState : BaseState {
    public:
        explicit SelectState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState& nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(matchesColumnRegex(literal)) {
                return (BaseState&) ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState&) FunctionOrClauseState(nextToken, stateMachine); 
            }
        };
};

class ColumnState : BaseState {
    public:
        explicit ColumnState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine), nextIsColumnOrFunction(false) {};
        BaseState& nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(inTableKeywordsList(literal)) {
                return (BaseState&) TableState(nextToken, stateMachine);
            } else if (literal == "AS") {
                return (BaseState&) IntermediateState(nextToken, stateMachine, this);
            } else if(literal == ",") {
                nextIsColumnOrFunction = true; //if next token is ",", then next state cannot be allies in select
                return (BaseState&) IntermediateState(nextToken, stateMachine, this);
            } else if (matchesColumnRegex(literal)) {
                if(!nextIsColumnOrFunction || stateMachine.isInAlliesList(literal)) {
                    return (BaseState&) AlliesState(nextToken, stateMachine);
                } else {
                    return (BaseState&) ColumnState(nextToken, stateMachine);
                }
            } else {
                return (BaseState&) FunctionOrClauseState(nextToken, stateMachine); 
            }
        }
        std::string convert() {
            std::string literal = token.literal.to_string();
            std::smatch match;
            if (std::regex_search(literal, match, COLUMN_WITH_POINT_REGEX)) {
					std::string prefix = match.prefix().str(); //table name with "." symbol
					std::string column = literal.substr(prefix.size(), literal.size() - prefix.size());
					size_t delimeter_pos = column.find(".");
					column.replace(delimeter_pos, 1, "\".\"");
					return prefix + column;
				} else {
					return literal;
				}
        }
    private:
        bool nextIsColumnOrFunction;
};

class AlliesState : BaseState {
    public:
        explicit AlliesState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {
            if(!stateMachine.isInAlliesList(token.literal.to_string())) {
                stateMachine.addAllies(token.literal.to_string());
            }
        };
        BaseState& nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == ",") {
                return (BaseState&) IntermediateState(nextToken, stateMachine, this);
            } else if(stateMachine.isInAlliesList(literal)) {
                return (BaseState&) AlliesState(nextToken, stateMachine);
            } else if(inTableKeywordsList(literal)) {
                return (BaseState&) TableState(nextToken, stateMachine);
            } else if(matchesColumnRegex(literal)) {
                return (BaseState&) ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState&) FunctionOrClauseState(nextToken, stateMachine);
            }
        }
};

class FunctionOrClauseState : BaseState {
    public:
        explicit FunctionOrClauseState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState& nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == ",") {
                return (BaseState&) IntermediateState(nextToken, stateMachine, this);
            } else if(stateMachine.isInAlliesList(literal)) {
                return (BaseState&) AlliesState(nextToken, stateMachine);
            } else if(inTableKeywordsList(literal)) {
                return (BaseState&) TableState(nextToken, stateMachine);
            } else if(matchesColumnRegex(literal)) {
                return (BaseState&) ColumnState(nextToken, stateMachine);
            } else {
                return (BaseState&) FunctionOrClauseState(nextToken, stateMachine);
            }
        }
};

class TableState : BaseState {
    public:
        explicit TableState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState& nextState(const Token& nextToken) {
            const std::string literal = nextToken.literal.to_string();
            if(literal == "AS" || matchesColumnRegex(literal)) {
                return (BaseState&) TableState(nextToken, stateMachine);
            } else {
                return (BaseState&) FunctionOrClauseState(nextToken, stateMachine);
            }
        }
};

class StateMachine {
    public:
        explicit StateMachine(const StringView& queryView, Lexer* lex): level(0), queryView(queryView), currentState(nullptr), lex(lex) {}
        explicit StateMachine(Lexer* lex): level(0), lex(lex), currentState(nullptr) {} 
        std::string run() {
            std::string modified_query;
            Token current_token(lex->Consume());
            if(to_upper(current_token.literal) != "SELECT") {
                return queryView.to_string();
            }
            currentState = (BaseState*) new SelectState(current_token, *this);
            bool read_from_state = true;
            for( ; (current_token.type != Token::EOS) && (level >= 0); current_token = lex->Consume()) {
                if(read_from_state) {
                    modified_query += currentState->convert();
                } else if(current_token.type == Token::LPARENT){
                    level++;
                    modified_query += "(";
                } else if(current_token.type == Token::RPARENT) {
                    level--;
                    modified_query += ")";
                } else {
                    modified_query += current_token.literal.to_string();
                }

                modified_query += " ";
                Token next_token = lex->LookAhead(1);
                read_from_state = false;
                if(to_upper(next_token.literal) == "SELECT") {
                    StateMachine subqueryMachine(lex);
                    modified_query += subqueryMachine.run();
                    level--;
                } else if(next_token.type == Token::IDENT || next_token.type == Token::COMMA || (std::find(function_list.begin(), function_list.end(), next_token) != function_list.end())) {
                    currentState = &(currentState->nextState(next_token));
                    read_from_state = true;
                }
            }
        };
        bool isInAlliesList(std::string str) {
            return std::find(alliesList.begin(), alliesList.end(), str) != alliesList.end();
        }
        void addAllies(std::string allies) {
            alliesList.push_back(allies);
        }
    private:
        Lexer* lex;
        int level;
        const StringView queryView;
        BaseState* currentState;
        std::list<std::string> alliesList;
};