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
            DEBUGRl(getRuleName(curRule));

            for(auto ruleElem : getRuleArr(curRule)){
                DEBUGRl(isTerminal(ruleElem));
            }
        }

//        auto it = QMapIterator<QString, QString>(rulesMap);
//        while (it.hasNext()) {
//            it.next();
//            DEBUGRl(it.key()<<it.value());
//        }
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

private:
    QVariantList rulesLst;
    QVariantMap relations;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
