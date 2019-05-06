#include "main.h"
#include "SyntacticAnalyzerRecursiveDown/syntacticalanalyzer.h"
#include "LexicalAnalyzer/lexicalanalyzer.h"
#include "gorodlangexception.h"
#include "simpleprecedenceruleparser.h"
#include "SyntacticAnalyzerRecursiveDown/reversepolishnotationbuilder.h"
#include "Executor/executor.h"

#include <QApplication>
#include <QGraphicsView>
#include <QStringList>
#include <QFile>

using namespace Gorod;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString inputProgPath("prog.gor"), lexerOutPath("LexerTable.csv")
            , inputGrammPath("gorod_v2.json"), outputGrammPath("gorod_v2_Relations.csv");

    if(argc >= 3){
        inputProgPath = argv[1]; lexerOutPath = argv[2];
    }

    if(argc >= 4){
        inputGrammPath = argv[3];
    }

    QGraphicsScene scene;
    QGraphicsView view(&scene);

    view.setRenderHints(QPainter::SmoothPixmapTransform);

    QFile inFileProg(inputProgPath), lexerOutFile(lexerOutPath)
            , inFileGramm(inputGrammPath), outFileGramm(outputGrammPath);
    if (! inFileProg.open(QIODevice::ReadOnly | QIODevice::Text)
            //|| ! lexerOutFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)
            //|| ! inFileGramm.open(QIODevice::ReadOnly | QIODevice::Text)
            //|| ! outFileGramm.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)
            ){

        DEBUGM(inFileProg.errorString() <<lexerOutFile.errorString()
               <<inFileGramm.errorString() <<outFileGramm.errorString());
        return 500;
    }

    QTextStream input(&inFileProg), outputTable(&lexerOutFile)
            , inputRules(&inFileGramm), outputRelations(&outFileGramm);

    try {

        /*
         * Next lines generate table, that represent Simple Precedence Rules
         * and write it to 'gorod_v2_Relations.csv' file
         *
            SimplePrecedenceRuleParser ruleParser(inputRules.readAll());
            ruleParser.Parse();
            DEBUGRl(ruleParser.toJson());
            outputRelations << ruleParser.toCSVTable() <<endl <<endl;
            outputRelations << ruleParser.toJson().replace(";", "!").replace(": ", ": ;");
        */

        const auto lexResult = LexicalAnalyzer::Parse(input.readAll());

        /*
         * Next lines just write 'lexResult' in human readable table
         *
            outputTable <<LexicalAnalyzer::GenerateCSVTable(lexResult) <<endl;
            outputTable <<QJsonDocument::fromVariant(lexResult)
                          .toJson(QJsonDocument::JsonFormat::Indented);
        */

        qDebug()<<"\n";
        DEBUGM("LEXICAL ANALIZE PASSED ( OK )!");
        qDebug()<<"\n";

        const auto astTree = SyntacticalAnalyzer::Parse(lexResult);
        Executor executor;

        QObject::connect(&executor, &Executor::sigReadText, [](qint64 &number){
            QTextStream s(stdin);
            QString value = s.readLine();
            bool ok;
            number = value.toLongLong(&ok);
            if(! ok){
                throw std::runtime_error("NaN");
            }
        });

        QObject::connect(&executor, &Executor::sigWriteText, [](qint64 number){
            QTextStream s(stdout);
            s << number <<endl;
        });

        executor.setIsRunning(true);
        executor.exec(astTree);

        //ReversePolishNotationBuilder rpnBuilder(astTree);
        //rpnBuilder.Generate();

        //DEBUGM(rpnBuilder.toRawJson().data());

        ASTNodeWalker::ShowASTTree(astTree, scene, view);

    } catch (Gorod::Exception &e) {
        DEBUGM(e.what().toUtf8().data());
        return EXIT_FAILURE;
    }

    inFileProg.close(); lexerOutFile.close();
    inFileGramm.close(); outFileGramm.close();
    return app.exec();
}
