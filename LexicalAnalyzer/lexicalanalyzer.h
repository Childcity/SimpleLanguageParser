#ifndef LEXICALANALYZER_H
#define LEXICALANALYZER_H



#include "main.h"
#include "LexicalAnalyzer/LexicalParserBase/lexicalparserbase.h"
#include "gorodlangtokens.h"

#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Gorod {

class LexicalException : public LexicalParserBaseException {
public:
    explicit LexicalException(const QString msg, int line, int pos)
        : LexicalParserBaseException(msg, line, pos){}

    LexicalException( const LexicalException & ) = default;

    ~LexicalException() override;

    QString what() const override{
        return LexicalParserBaseException::what();
    }
};

class LexicalAnalyzer : public LexicalParserBase{
    using Token = LangTokens::Token;
public:
    explicit LexicalAnalyzer(const QString &source)
        : LexicalParserBase(source)
    {}

    virtual ~LexicalAnalyzer();

    // Number. Here we if cur symbol is number
    qint64 Number() {
        DEBUGlex("-> Number"<<getCurrentSymbol())
        QString number;

//        if(getCurrentSymbol() == '-' || getCurrentSymbol() == '+'){
//            number += getCurrentSymbol();
//            next();
//        }

        while (getCurrentSymbol().isDigit()) {
            number += getCurrentSymbol();
            next();
        }

        if(number.isEmpty() || (number.size() == 1 && (! number[0].isDigit()))){
                throw LexicalException(QString("waiting on number (pos=%1) \"%2\"")
                                       .arg(getPosInLine()).arg(getSource().midRef(getPosInLine(), 3))
                                       , getLine(), getPosInLine()
                                       );
            }

        DEBUGlex("<- Number"<<number)
        return number.toLongLong();
    }

    //Ident. Here we if cur symbol is letter
    QString Ident() {
        DEBUGlex("-> Ident"<<getCurrentSymbol())
        QString identifier;

        while (getCurrentSymbol().isLetterOrNumber() || getCurrentSymbol() == '_') {
            identifier += getCurrentSymbol();
            next();
        }

        DEBUGlex("<- Ident"<<getCurrentSymbol())
        return identifier;

    }

    QVariantList Parse(){
        skip();
        while (! isEnd()) {

            QVariantMap lexem({
                                  {"line", getLine()},
                                  {"posInLine", getPosInLine()}
                              });

            if(getCurrentSymbol().isDigit()){
                qint64 number = Number();
                lexem["type"] = "constant";
                lexem["value"] = number;
                lexem["id"] = calcConstantId(number);

            }else if (LangTokens::IsToken(getCurrentSymbol())){
                DEBUGlex("-> token"<<getCurrentSymbol())
                lexem["type"] = "token";
                lexem["value"] = getCurrentSymbol();
                next();
                DEBUGlex("<- token"<<getCurrentSymbol())

            }else if (getCurrentSymbol().isLetter()){
                QString word = Ident();

                lexem["value"] = word;

                if(LangTokens::IsToken(word)){
                    DEBUGlex("-> token long"<<word)
                    lexem["type"] = "token";
                }else{
                    DEBUGlex("-> idetifier"<<word)
                    lexem["type"] = "ident";
                    lexem["id"] = calcIdentId(word);
                }

                DEBUGlex("<- token or identifier"<<word)
            }else{
                throw LexicalException(QString("unexpected Token (pos=%1) \"%2\"")
                                       .arg(getPosInLine()).arg(getSource().midRef(getPosInLine(), 3))
                                       , getLine(), getPosInLine()
                                       );
            }

            parsedList_ += lexem;
            skip();
        }

        return parsedList_;
    }

    static QVariantList Parse(QString source){
        LexicalAnalyzer lexer(source);
        return lexer.Parse();
    }

    static QString GenerateCSVTable(const QVariantList &parsedList){
        QString outCSVTable;
        QList<int> existedId;

        outCSVTable += "   CONSTANT    ;  ID  \n";
        for(const auto &it : parsedList){
            const auto item = it.toMap();
            if(item["type"] == "constant" && ! existedId.contains(item["id"].toInt())){
                outCSVTable += item["value"].toString() + ";" + item["id"].toString() + "\n";
                existedId += item["id"].toInt();
            }
        }

        existedId.clear();
        outCSVTable += "\n\n   IDENT    ;  ID  ;   TYPE   \n";
        for(const auto &it : parsedList){
            const auto item = it.toMap();
            if(item["type"] == "ident" && ! existedId.contains(item["id"].toInt())){
                outCSVTable += item["value"].toString() + ";" + item["id"].toString() + ";int\n";
                existedId += item["id"].toInt();
            }
        }

        outCSVTable += "\n\n   LINE    ;  SUBSTR  ;   ID   ;idnt/const ID\n";
        for(const auto &it : parsedList){
            const auto item = it.toMap();
            QString subStr = item["value"].toString();
            QString tokenId;

            if (item["type"] == "constant") {
                tokenId = QString::number(static_cast<int>(LangTokens::GetToken("CONSTANT")));
            }else if (item["type"] == "ident") {
                tokenId = QString::number(static_cast<int>(LangTokens::GetToken("IDENT")));
            }else{ //item["type"] == "token"
                tokenId = QString::number(static_cast<int>(LangTokens::GetToken(subStr)));
            }

            outCSVTable += item["line"].toString() + ";"
                    + (subStr == ";" ? "!" : subStr) + ";"
                    + tokenId + ";"
                    + item["id"].toString() + "\n";
        }

        return outCSVTable;
    }

private:
    bool isNumber(){
        if(getCurrentSymbol().isDigit())
            return true;

//        if(getCurrentSymbol() == '-' || getCurrentSymbol() == '+'){
//            DEBUGlex("isNumber"<<getCurrentSymbol());
//            if(getSource().at(getPos() + 1).isDigit()){
//                DEBUGlex("isNumber (getPos() + 1)"<<getSource().at(getPos() + 1))
//                return true;
//            }
//        }

        return false;
    }

    int calcIdentId(const QString &ident){
        for(auto const &it : parsedList_){
            const auto item = it.toMap();
            if(item["type"] ==  "ident" && item["value"] == ident){
                return item["id"].toInt();
            }
        }

        return identCount_++;
    }

    int calcConstantId(const qint64 &constant){
        for(auto const &it : parsedList_){
            const auto item = it.toMap();
            if(item["type"] ==  "constant" && item["value"].toLongLong() == constant){
                return item["id"].toInt();
            }
        }

        return constantCount_++;
    }

private:
    QVariantList parsedList_;
    int identCount_ = 0;
    int constantCount_ = 0;
};


}

#endif // LEXICALANALYZER_H
