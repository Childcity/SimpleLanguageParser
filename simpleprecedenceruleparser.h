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
#include <QStack>

namespace Gorod {

//class SimplePrecedenceRuleParserEx : public Exception {
//public:
//    explicit SimplePrecedenceRuleParserEx(const QString msg)
//        : Exception(msg)
//    {}

//    SimplePrecedenceRuleParserEx( const SimplePrecedenceRuleParserEx & ) = default;

//    virtual ~SimplePrecedenceRuleParserEx() override;

//    QString what() const override{
//        return QString("Error[RuleParser]:: %1").arg(msg_);
//    }
//};

class SimplePrecedenceRuleParser{
public:
    explicit SimplePrecedenceRuleParser(QString rulesRawJson);

    void Parse();

    QString toJson() const;

    QString toCSVTable();

    QVariantList getRelations() const;

private:
    QString getRuleName(const QVariant &ruleObj) const;

    QVariantList getRuleArr(const QVariant &ruleObj) const;

    bool isTerminal(const QVariant &ruleElem) const;

    bool isTerminal(const QString &ruleName) const;

    QString getRuleElemValue(const QVariant &ruleElem) const;

    void putRelation(const QString &rule, const QVariantMap &relation);

    QVariantList findRuleValueByName(const QString &ruleToFind);

    bool checkOnInfiniteRecursion(const QString &rule, QStringList memory);

    QVariantMap matchRuleRelByName(const QString &ruleName) const;


    QStringList findFirstPlusReletions(const QString &ruleName);

    void transformToFirstPlus();

    void transformToLastPlus();

private:
    QVariantList rulesLst;
    QVariantList relationsLst;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
