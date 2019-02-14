#ifndef SIMPLEPRECEDENCERULEPARSER_H
#define SIMPLEPRECEDENCERULEPARSER_H

#include "main.h"
#include "gorodlangexception.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

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
        for(auto curRule : rulesLst){

            QVariantMap curRuleRelations;

            const auto ruleArr = getRuleArr(curRule);
            for (QVariantList::const_iterator ruleElemIt = ruleArr.begin(); ruleElemIt != ruleArr.end(); ++ruleElemIt){
                curRuleRelations[getRuleElemValue(*ruleElemIt)] = "=";
            }

            relationsLst += QVariantMap({
                                            {getRuleName(curRule), curRuleRelations}
                                        });

//            for(auto ruleElem : ruleArr){
//                //DEBUGRl();
//                //if(isTerminal(ruleElem))

//            }
        }

        DEBUGRl(QJsonDocument::fromVariant(relationsLst).toJson(QJsonDocument::JsonFormat::Indented));
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

private:
    QVariantList rulesLst;
    QVariantList relationsLst;
};

//        auto it = QMapIterator<QString, QString>(rulesMap);
//        while (it.hasNext()) {
//            it.next();
//            DEBUGRl(it.key()<<it.value());
//        }

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
