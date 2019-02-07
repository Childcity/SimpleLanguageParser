#ifndef ASTNODE_H
#define ASTNODE_H

#include "main.h"
#include "gorodlangtokens.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTextStream>
#include <QProcess>
#include <QPixmap>

#include <memory>

static int GlobalIdForASTNode = 0;

namespace Gorod {

/*  AST-дерево представляет собой структурное
    представление исходной программы, очищенное от элементов конкретного
    синтаксиса (в рассматриваемом примере в AST-дерево не попал «разделитель»,
    т.к. он не имеет отношения непосредственно к семантике данного фрагмента
    программы, а лишь к конкретному синтаксису языка). В качестве узлов в AST-
    дереве выступают операторы, к которым присоединяются их аргументы,
    которые в свою очередь также могут быть составными узлами. Часто узлы
    AST-дерева получаются из лексем, выделенных на этапе лексического анализа,
    однако могут встречаться и узлы, которым ни одна лексема не соответствует
*/

class ASTNode: public std::enable_shared_from_this<ASTNode> {
public:
    using NodeType = LangTokens::Token;
    using WeakPtr = std::weak_ptr<ASTNode>;
    using SharedPtr = std::shared_ptr<ASTNode>;

private:
    explicit ASTNode()
        : type_(NodeType::UNKNOWN)
        , parent_(SharedPtr()){GlobalIdForASTNode++;}

    explicit ASTNode(NodeType type, QString text)
        : type_(type)
        , text_(text){GlobalIdForASTNode++;}

public:

    explicit ASTNode(const ASTNode &obj)
        : std::enable_shared_from_this<ASTNode>(){
            type_ = obj.type_;
            text_ = obj.text_;
            parent_ = obj.parent_.lock();
            childs_ = obj.childs_;
    }

    ~ASTNode(){DEBUGAST("~:"<<toString()<<"["<<text_)}

    static SharedPtr GetNewInstance(NodeType type, QString text, SharedPtr &child1, SharedPtr &child2){
        SharedPtr instance(std::make_shared<ASTNode>(ASTNode(type, text)));
        if(child1){
            instance->addChild(child1);
        }
        if(child2){
            instance->addChild(child2);
        }
        return instance;
    }

    static SharedPtr GetNewInstance(NodeType type, SharedPtr &child1, SharedPtr &child2){
        return GetNewInstance(type, LangTokens::GetToken(type), child1, child2);
    }

    static SharedPtr GetNewInstance(NodeType type, QString text, SharedPtr &child1){
        SharedPtr instance(GetNewInstance(type, text));
        if(child1){
            instance->addChild(child1);
        }
        return instance;
    }

    static SharedPtr GetNewInstance(NodeType type, SharedPtr &child1){
        return GetNewInstance(type, LangTokens::GetToken(type), child1);
    }

    static SharedPtr GetNewInstance(NodeType type, QString text){
        SharedPtr instance(std::make_shared<ASTNode>(ASTNode(type, text)));
        return instance;
    }

    static SharedPtr GetNewInstance(NodeType type){
        return GetNewInstance(type, LangTokens::GetToken(type));
    }

    void addChild(SharedPtr child){
        DEBUGAST("adding child:"<<child->text_<<"to root:"<<toString())
        if (child->isParentValid()) {
            DEBUGAST("parent valid in child:"<<child->toString())
            child->getParent().lock()->removeChild(child);
        }
        DEBUGAST("Count of childs removed:"<<childs_.removeAll(child));
        childs_.append(child);
        DEBUGAST("Count of childs in"<<text_<<childs_.size())
        WeakPtr self = shared_from_this(); //shared_from_this() we cannot use here,
                                    //because this object has not been initialized
                                    //yet and there isn't any shared_ptr that point to it
        child->setParent(self);
        DEBUGAST("In parent count of childs:"<<child->getParent().lock()->getChildsCount())
    }

    void removeChild(SharedPtr &child){
        DEBUGAST("Count of childs removed:"<<childs_.removeAll(child));
        if(child->getParent().lock() == shared_from_this()){
            auto nullSharedPtr = SharedPtr();
            child->setAsParent(nullSharedPtr);
        }
    }

    bool isParentValid () const {
        //return (!parent_.owner_before(WeakPtr{})) &&( !WeakPtr{}.owner_before(parent_));
        return parent_.lock() != nullptr;
    }

    WeakPtr getParent() { return parent_; }

    void setParent( WeakPtr &parent) { parent_ = parent; }

    SharedPtr getChild(int index) const { return childs_.at(index); }

    int getChildsCount() const { return childs_.length(); }

    int getChildIndexInParent() {
        auto self = shared_from_this();
        //auto nullSharedPtr = SharedPtr();
        return isParentValid() ? parent_.lock()->childs_.indexOf(self) : -1;
    }

    void setAsParent( SharedPtr &val) {
        auto self = shared_from_this();
        val->addChild(self);
    }

    NodeType getType() const { return type_; }

    QString getUniqueName() const { return uniqName; }

    QString getText() const { return text_; }

//    QList<SharedPtr> getChilds() const { return childs_; }

    QString toString() const { return LangTokens::GetToken(type_); }

private:
    QString uniqName = "node" + QString::number(GlobalIdForASTNode);
    // тип узла (см. описание ниже)
     NodeType type_;

     // текст, связанный с узлом
     QString text_;

     // родительский узел для данного узла дерева
     WeakPtr parent_;

     // потомки (ветви) данного узла дерева
     QList<SharedPtr> childs_;
};

class ASTNodeWalker{
public:

    static void ShowASTTree(const ASTNode::SharedPtr &root, QGraphicsScene &scene,  QGraphicsView &view){

        QByteArray ASTTreeSource;
        QTextStream stream(&ASTTreeSource, QIODevice::ReadWrite);
        stream << "graph {" << endl;
        stream << "\tnode[fontsize=10,margin=0,width=\".4\", height=\".3\"];" << endl;
        stream << "\tsubgraph cluster17{" << endl;

        GetStringSubTree(root, stream);

        stream << "\t}\n}" << endl;
        stream.flush();

        QProcess* p = new QProcess();
        QByteArray a = ASTTreeSource;

        p->setProcessChannelMode(QProcess::MergedChannels);
        p->start("dot", QStringList() << "-Tpng");
        p->write(a);

        QByteArray data;
        QPixmap pixmap = QPixmap();

        while(p->waitForReadyRead(200)){
            data.append(p->readAll());
        }

        pixmap.loadFromData(data);

        scene.addPixmap(pixmap);
        view.show();
    }

private:
    static void GetStringSubTree(const ASTNode::SharedPtr &node, QTextStream &stream){

        if(! node->getParent().lock()){
            stream << "\t\tn" << node->getUniqueName() << "[label=\"" << node->getText() <<"\"];" << endl;

        }else {
            stream << "\t\tn" << node->getUniqueName() << "[label=\"" << node->getText() <<"\"];" << endl;
            stream << "\t\tn" <<node->getParent().lock()->getUniqueName() << "--" <<"n" <<node->getUniqueName() << ";" << endl;
        }

        for (int i = 0; i < node->getChildsCount(); i++) {
            GetStringSubTree(node->getChild(i), stream);
        }
    }

};


}

#endif // ASTNODE_H
