#pragma once

#include <string>
#include "string_view.h"
#include "lexer.h"
#include <regex>
#include <list>
#include <algorithm>
#include <memory>
//#include "../log/log.h"


#define DECLARE2(NAME, IGNORE) Token::NAME
const std::list<Token::Type> function_list{
#include "function_declare.h"
};
#undef DECLARE2

const std::regex COLUMN_REGEX(".*\\.?\"?(ENTITY|TAGS|METRIC|TEXT|VALUE|TIME|DATETIME|[0-9]+)\\.?.*\"?");
const std::regex COLUMN_WITH_POINT_REGEX("\"((e|E)(n|N)(t|T)(i|I)(t|T)(y|Y)|(t|T)(a|A)(g|G)(s|S)|(m|M)(e|E)(t|T)(r|R)(i|I)(c|C))\\..+\"");
const std::list<std::string> TABLE_KEYWORDS_LIST = { //list of keywords after which tables and there allies are declared
        "FROM",
        "JOIN"
};

class StateMachine;

bool inTableKeywordsList(const std::string& literal);
bool matchesColumnRegex(const std::string& literal);

class BaseState {
    public:
        explicit BaseState(const Token& token, StateMachine& stateMachine): token(token), stateMachine(stateMachine) {};
        virtual std::string convert() {
            return token.literal.to_string();
        };
		virtual BaseState* nextState(const Token& nextToken) {
			return this;
		}
    protected:
        const Token token;
        StateMachine& stateMachine;
};

/*class IntermediateState : BaseState { //techincal state for situations when state cannot be changed with one token and requires one more. e.g. "AS" token
    public:
		explicit IntermediateState(const Token& token, StateMachine& stateMachine, BaseState* parentState) : BaseState(token, stateMachine), parentState(*parentState) {}
        BaseState* nextState(const Token& nextToken) {
            return (BaseState*) parentState.nextState(nextToken);
        }
    private:
        BaseState parentState;
}; */

class SelectState : public BaseState {
    public:
        explicit SelectState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState* nextState(const Token& nextToken);
};

class ColumnState : public BaseState {
    public:
        explicit ColumnState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine), nextIsColumnOrFunction(false) {};
		explicit ColumnState(const Token& token, StateMachine& stateMachine, bool nextIsColumnOrFunction) : BaseState(token, stateMachine), nextIsColumnOrFunction(nextIsColumnOrFunction) {}
        BaseState* nextState(const Token& nextToken);
        std::string convert();
    private:
        bool nextIsColumnOrFunction;
};

class AlliesState : public BaseState {
    public:
        explicit AlliesState(const Token& token, StateMachine& stateMachine);
        BaseState* nextState(const Token& nextToken);
};

class FunctionOrClauseState : public BaseState {
    public:
        explicit FunctionOrClauseState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState* nextState(const Token& nextToken);
};

class TableState : public BaseState {
    public:
        explicit TableState(const Token& token, StateMachine& stateMachine): BaseState(token, stateMachine) {};
        BaseState* nextState(const Token& nextToken);
};

class StateMachine {
    public:
        explicit StateMachine(const StringView& queryView, Lexer* lex): level(0), queryView(queryView), currentState(nullptr), lex(lex) {}
        explicit StateMachine(Lexer* lex): level(0), lex(lex), currentState(nullptr) {} 
        std::string run();
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
        std::unique_ptr<BaseState> currentState;
        std::list<std::string> alliesList;
};