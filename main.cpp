#include "main.h"
#include "SyntacticAnalyzerRecursiveDown/syntacticalanalyzer.h"
#include "LexicalAnalyzer/lexicalanalyzer.h"
#include "gorodlangexception.h"
#include "simpleprecedenceruleparser.h"

#include <QApplication>
#include <QGraphicsView>
#include <QStringList>
#include <QFile>

using namespace Gorod;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString inputProgPath("prog.gor"), outputProgPath("inputResultTable.csv"), inputEBNFPath("gorod.ebnf");

    if(argc >= 3){
        inputProgPath = argv[1]; outputProgPath = argv[2];
    }

    if(argc >= 4){
        inputEBNFPath = argv[3];
    }

    QGraphicsScene scene;
    QGraphicsView view(&scene);

    view.setRenderHints(QPainter::SmoothPixmapTransform);

    QFile inFileProg(inputProgPath), outFileProg(outputProgPath), inFileEBNF(inputEBNFPath);
    if (! inFileProg.open(QIODevice::ReadOnly | QIODevice::Text)
            || ! outFileProg.open(QIODevice::WriteOnly | QIODevice::Text |QIODevice::Truncate)
            || ! inFileEBNF.open(QIODevice::ReadOnly | QIODevice::Text)){
        DEBUGM(inFileProg.errorString()<<outFileProg.errorString())
        return 500;
    }

    QTextStream input(&inFileProg), outputTable(&outFileProg), inputRules(&inFileEBNF);

    try {

        SimplePrecedenceRuleParser ruleParser(inputRules.readAll());

        return 0;
        const auto lexResult = LexicalAnalyzer::Parse(input.readAll());

        outputTable <<LexicalAnalyzer::GenerateCSVTable(lexResult) <<endl;
        outputTable <<QJsonDocument::fromVariant(lexResult)
                      .toJson(QJsonDocument::JsonFormat::Indented);

        qDebug()<<"\n";
        DEBUGM("LEXICAL ANALIZE PASSED ( OK )!");
        qDebug()<<"\n";

        const auto astTree = SyntacticalAnalyzer::Parse(lexResult);
        ASTNodeWalker::ShowASTTree(astTree, scene, view);
    } catch (Gorod::Exception &e) {
        DEBUGM(e.what().toUtf8().data());
        return EXIT_FAILURE;
    }

    inFileProg.close(); outFileProg.close();
    return a.exec();
}
