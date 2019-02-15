#ifndef SIMPLEPRECEDENCERULEPARSER_H
#define SIMPLEPRECEDENCERULEPARSER_H

#include "main.h"
#include "gorodlangtokens.h"
#include "gorodlangexception.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QFile>

namespace Gorod {

class SimplePrecedenceRuleParserEx : public Exception {
public:
    explicit SimplePrecedenceRuleParserEx(const QString msg)
        : Exception(msg)
    {}

    SimplePrecedenceRuleParserEx( const SimplePrecedenceRuleParserEx & ) = default;

    virtual ~SimplePrecedenceRuleParserEx() override;

    QString what() const override{
        return QString("Error[RuleParser]:: %1").arg(msg_);
    }
};

class SimplePrecedenceRuleParser{
public:
    explicit SimplePrecedenceRuleParser(QString rulesRawJson)
        : rulesLst(QJsonDocument::fromJson(rulesRawJson.toLatin1()).toVariant().toList())
    {
        for(const auto &curRule : rulesLst){


            const auto ruleArr = getRuleArr(curRule);
            for (QVariantList::const_iterator ruleElemIt = ruleArr.begin(); ruleElemIt != ruleArr.end(); ++ruleElemIt){
                // if exist next rule

                if((ruleElemIt + 1) != ruleArr.end() && (! (*(ruleElemIt + 1)).isNull())){
                    putRelation(getRuleElemValue(*ruleElemIt), QVariantMap({
                                                                               {getRuleElemValue(*(ruleElemIt + 1)), "="}
                                                                           }));
                }
            }

//            if(ruleElemIt->userType() == QMetaType::QVariantList){

//            }

        }

        DEBUGRl(QJsonDocument::fromVariant(relationsLst).toJson(QJsonDocument::JsonFormat::Indented));
        QFile file("relations.csv");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << toCSVTable();
    }

    QString toCSVTable() const {
        QStringList tokenRuleColLst;
        QByteArray buff;
        QTextStream csv(&buff, QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        csv.setFieldAlignment(QTextStream::AlignCenter);
        csv.setFieldWidth(7);

        for(const auto &curRule : rulesLst){
            tokenRuleColLst <<getRuleName(curRule);
        }
        for(const auto &curToken : Gorod::LangTokens::GetToken()){
            tokenRuleColLst <<curToken;
        }

        tokenRuleColLst.sort();

        // Build table HEADER
        csv << right << ";";
        for(const auto &curRuleToken : tokenRuleColLst){
            csv << (curRuleToken == ";" ? "!" : curRuleToken) << right << ";";
        }
        csv << endl;


        for(const auto &curRule : relationsLst){
            csv <<getRuleName(curRule) << right << ";";

            const QVariantMap curRulMap = curRule.toMap().begin().value().toMap();
            for(auto it : tokenRuleColLst){
                if(curRulMap.contains(it)){
                    csv << curRulMap[it].toString();
                }
                csv << right << ";";
            }
            csv << endl;
        }

        return buff;
    }

private:
    QString getRuleName(const QVariant ruleObj) const {
        return ruleObj.toMap().begin().key();
    }

    QVariantList getRuleArr(const QVariant ruleObj) const {
        return ruleObj.toMap().begin().value().toList();
    }

    bool isTerminal(const QVariant ruleElem) const {
        return ruleElem.toMap()["type"] == "term";
    }

    QString getRuleElemValue(const QVariant ruleElem) const {
        return ruleElem.toMap()["value"].toString();
    }

    void putRelation(QString rule, QVariantMap relation){
        // find relation
        for(auto &ruleIt : relationsLst){
            if(getRuleName(ruleIt) == rule){
                //DEBUGRl(QJsonDocument::fromVariant(ruleIt).toJson(QJsonDocument::JsonFormat::Indented));
                static_cast<QVariantMap*>(
                            // get link to first element in map and convert it to QVarianMap
                            static_cast<QVariantMap*>(ruleIt.data())->first().data()
                            )->unite(relation);
                //DEBUGRl(QJsonDocument::fromVariant(ruleIt).toJson(QJsonDocument::JsonFormat::Indented));
                return;
            }
        }

        // create new rule, if we can't find rule in list
        relationsLst += QVariantMap({
                                        {rule, relation}
                                    });
    }

private:
    QVariantList rulesLst;
    QVariantList relationsLst;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
