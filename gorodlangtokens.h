#ifndef GORODLANGTOKENS_H
#define GORODLANGTOKENS_H

#include <QMap>

namespace Gorod {

struct LangTokens {
    enum class Token{LeftBraket, RightBraket, Int, Read, Write, For, To, By, While, Rof, If,
                      Add, Sub, Mul, Div, Power, LeftRoundBracket, RightRoundBracket, Assign, More,
                      Less, DotPoint, Comma, Point, Question, DoubleDot, IDENT, CONSTANT, UNKNOWN, GOROD};

    const static QMap<Token, QString> GetToken() {
         const static QMap<Token, QString> tokens{
            {Token::UNKNOWN,            "unknown"},
            {Token::GOROD,             "Gorod Program"},
            {Token::LeftBraket,         "{"},       {Token::RightBraket,            "}"},
            {Token::LeftRoundBracket,   "("},       {Token::RightRoundBracket,      ")"},
            {Token::Int,                "int"},
            {Token::Read,               "read"},    {Token::Write,                  "write"},

            {Token::For,                "for"},     {Token::To,                     "to"},
            {Token::By,                 "by"},      {Token::While,                  "while"},
            {Token::Rof,                "rof"},

            {Token::If,                 "if"},
            {Token::Assign,             "="},
            {Token::Add, "+"}, {Token::Sub, "-"}, {Token::Mul, "*"}, {Token::Div, "/"},
            {Token::Power, "^"}, {Token::More, ">"}, {Token::Less, "<"},

            {Token::DotPoint,           ";"},       {Token::Comma,                  ","},
            {Token::Point,              "."},       {Token::Question,               "?"},
            {Token::DoubleDot,          ":"},

            {Token::IDENT,              "Ident"},   {Token::CONSTANT,               "Const"}
        };

        return tokens;
    }

    //LangTokens::GetTokens(QList<Token>() <<Token::Add))
    static QStringList GetTokens(QList<Token> tokenList){
        QStringList tokenStrLst;
        for(const auto it : tokenList){
            tokenStrLst <<GetToken(it);
        }
        return tokenStrLst;
    }

    static QString GetToken(Token token){
        return GetToken().value(token);
    }

    static Token GetToken(QString token){
        return GetToken().key(token);
    }

    static bool IsToken(QString str){
        for(const auto &it : GetToken()){
            if(it == str){
                return true;
            }
        }
        return false;
    }
};

}
#endif // GORODLANGTOKENS_H


//enum class VarType{INT, DOUBLE};

//const static QMap<VarType, QString> GetVarType() {
//     const static QMap<VarType, QString> varTypes{
//         {VarType::INT, "int"},
//         {VarType::DOUBLE, "double"}
//     };
//     return varTypes;
//}

//static QString GetVarType(VarType varType){
//    return GetVarType().value(varType);
//}
