#include "main.h"
#include "SyntacticAnalyzerRecursiveDown/syntacticalanalyzer.h"
#include "LexicalAnalyzer/lexicalanalyzer.h"
#include "gorodlangexception.h"

#include <QApplication>
#include <QGraphicsView>
#include <QStringList>
#include <QFile>

using namespace Gorod;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString inputPath("prog.gor"), outputPath("inputResultTable.csv");

    if(argc >= 3){
        QStringList args = QApplication::arguments();
        inputPath = args[1]; outputPath = args[2];
    }

    QGraphicsScene scene;
    QGraphicsView view(&scene);

    view.setRenderHints(QPainter::SmoothPixmapTransform);

    QFile inFile(inputPath), outFile(outputPath);
    if (! inFile.open(QIODevice::ReadOnly | QIODevice::Text)
            || ! outFile.open(QIODevice::WriteOnly | QIODevice::Text |QIODevice::Truncate)){
        DEBUGM(inFile.errorString()<<outFile.errorString())
        return 123;
    }

    QTextStream input(&inFile), outputTable(&outFile);

    try {
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

    inFile.close(); outFile.close();
    return a.exec();
}
