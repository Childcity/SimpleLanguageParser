#ifndef SYNTACTICALPARSERBASE_H
#define SYNTACTICALPARSERBASE_H

#include "main.h"
#include "gorodlangexception.h"
#include <QStringList>

namespace Gorod {

class SyntacticalParserBaseException : public Exception {
public:
    explicit SyntacticalParserBaseException(const QString msg, int line, int pos)
        : Exception(msg)
        , line_(line)
        , pos_(pos){}

    SyntacticalParserBaseException( const SyntacticalParserBaseException & ) = default;

    virtual ~SyntacticalParserBaseException() override;

    QString what() const override{
        return QString("Error[Syntactical]:: %1").arg(msg_);
    }

    int getPos() const override {return pos_;}

    int getLine() const override {return line_;}

protected:
    int line_;
    int pos_;
};


class SyntacticalParserBase {
public:
    explicit SyntacticalParserBase(const QVariantList &source)
        : source_(source)
    {}

    virtual ~SyntacticalParserBase();

    QVariantList &getSourceRef() { return source_; }

    int getPos() const { return pos_; }

    void setPos(int pos) { pos_ = pos; }

    int lexemeLine() const { return lexeme()["line"].toInt(); }

    int lexemePosInLine() const { return lexeme()["posInLine"].toInt(); }

    // lexem в текущей позиции указателя
    QVariantMap lexeme() const { return at(pos_);}

    QString lexemeValue() const { return lexeme()["value"].toString();}

    bool isEnd() const { return pos_ >= source_.size(); }

    // передвигает указатель на один символ
    void next() {
        if (! isEnd())
            pos_++;
    }

    // проверяет, можно ли в текущей позиции указателя, распознать
    // одну из строк; указатель не смещается
    bool isMatch(const QStringList &terms) {
        int pos = pos_;
        const QString result = matchNoExcept(terms);
        pos_ = pos;
        return ! result.isNull();
    }

    bool isMatch(const QString &term) {
        return isMatch(QStringList() <<term);
    }

    bool isMatch(const QString &term1, const QString &term2) {
        return isMatch(QStringList() <<term1 <<term2);
    }

    // распознает одну из lexeme; при этом указатель смещается и
    // пропускаются незначащие символы; если ни одну из строк
    // распознать нельзя, то выбрасывается исключение
    QString match(const QStringList &terms) {
        //int pos = pos_;
        const QString result = matchNoExcept(terms);
        if (result.isNull()) {
            QString message = "Ожидалась одна из строк: ";
            bool first = true;
            for (const auto &str : terms) {
                if (! first){
                    message += ", ";
                }
                message += QString("\"%1\"").arg(str);
                first = false;
            }
            DEBUGSYNTX("match fail"<<lexemeValue())
            throw SyntacticalParserBaseException(message//"%1 (pos=%2)").arg().arg(lexemePosInLine())
                    , lexemeLine(), lexemePosInLine());
        }
        return result;
    }

    // то же, что и Match(params string[] a), для удобства
    QString match(const QString &term1, const QString &term2) {
        return match(QStringList() <<term1 <<term2);
    }

    // то же, что и Match(params string[] a), для удобства
    QString match(const QString &s) {
        //int pos = pos_;
        try {
            return match(QStringList()<<s);
        }catch(...){
        DEBUGSYNTX("match fail (wait on symb/str)"<<lexemeValue())
            throw SyntacticalParserBaseException(QString("%1: '%2'")
                                       .arg(s.length() == 1 ? "Ожидался символ" : "Ожидалась строка")
                                       .arg(s)
                , lexemeLine(), lexemePosInLine()
                );
        }
    }

protected:
    // предотвращает возникновение ошибки обращения за пределы
    // массива; в этом случае возвращает Null,
    // что означает конец входной программы
    const QVariantMap at(int index) const {
        return index < source_.length() ? source_.at(index).toMap() : QVariantMap();
    }

    // распознает одну из строк; при этом указатель смещается и
    // пропускаются незначащие lexeme;
    // если ни одну из lexeme распознать нельзя, то возвращается Null
    QString matchNoExcept(const QStringList &terms) {
        int pos = pos_;
        for (const auto &str : terms) {

            //DEBUGSYNTX(lexemeValue()<<str)
            if (lexemeValue() != str){
                pos_ = pos;
            }else{
                next();
                return str;
            }

        }

        return QString();
    }

private:

// разбираемая строка
QVariantList source_;

// позиция указателя
// (указывает на первый символ неразобранной части вход. строки)
int pos_ = 0;

};
}

#endif // SYNTACTICALPARSERBASE_H
