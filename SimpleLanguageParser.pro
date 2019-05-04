QT += gui widgets core
CONFIG += console

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = build/
OBJECTS_DIR = obj/
MOC_DIR     = moc/
UI_DIR      = ui/

SOURCES += \
        main.cpp \
    ASTree/astnode.cpp \
    gorodlangtokens.cpp \
    LexicalAnalyzer/LexicalParserBase/lexicalparserbase.cpp \
    LexicalAnalyzer/lexicalanalyzer.cpp \
    SyntacticAnalyzerRecursiveDown/SyntacticalParserBase/syntacticalparserbase.cpp \
    SyntacticAnalyzerRecursiveDown/syntacticalanalyzer.cpp \
    gorodlangexception.cpp \
    simpleprecedenceruleparser.cpp \
    SyntacticAnalyzerRecursiveDown/reversepolishnotationbuilder.cpp \
    Executor/executor.cpp

HEADERS += \
    lexicalparserbase.h \
    ASTree/astnode.h \
    main.h \
    gorodlangtokens.h \
    lexerouttable.h \
    lexicalanalyzer.h \
    syntacticalparserbase.h \
    syntacticalanalyzer.h \
    LexicalAnalyzer/LexicalParserBase/lexicalparserbase.h \
    LexicalAnalyzer/lexicalanalyzer.h \
    SyntacticAnalyzerRecursiveDown/SyntacticalParserBase/syntacticalparserbase.h \
    SyntacticAnalyzerRecursiveDown/syntacticalanalyzer.h \
    gorodlangexception.h \
    simpleprecedenceruleparser.h \
    SyntacticAnalyzerRecursiveDown/reversepolishnotationbuilder.h \
    Executor/executor.h
