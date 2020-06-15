#ifndef NODE_H
#define NODE_H

#include <llvm/IR/Value.h>
#include <iostream>
#include <vector>
#include "main.h"

typedef std::vector<Node *> NodeList;
typedef std::vector<NVariableDeclaration*> VariableList;
class CodeGenContext;

class Node {
 public:
  virtual ~Node() {}

  virtual llvm::Value *codeGen(CodeGenContext &context) { return NULL; };
};

class NInteger : public Node {
 public:
  long long value;
  NInteger(long long value) : value(value) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NIdentifier : public Node {
 public:
  std::string name;
  NIdentifier(const std::string &name) : name(name) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
  llvm::Value *store(Node *rhs, CodeGenContext &context);
};

class NString : public Node {
 public:
  std::string value;
  NString(std::string value) : value(value) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NArray : public Node {
 public:
  NodeList data;
  NArray() {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
  llvm::Value *getElementPtrInst(CodeGenContext &context,
                                 const std::string &identifier, int i);
  llvm::Value *store(Node *rhs, const std::string &identifier,
                     CodeGenContext &context);
};

class NArrid : public Node {
 public:
  Node &id;
  Node &idx;
  NArrid(Node &id, Node &idx) : id(id), idx(idx) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
  llvm::Value *getElementPtrInst(CodeGenContext &context,
                                 const std::string &identifier);
};

class NMethodCall : public Node {
 public:
  Node &id;
  Node &arguments;
  //NodeList data;
  NMethodCall(Node &id, Node &arguments) : id(id), arguments(arguments) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NAssignment : public Node {
 public:
  Node &lhs;
  Node &rhs;
  NAssignment(Node &lhs, Node &rhs) : lhs(lhs), rhs(rhs) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NMath : public Node {
 public:
  int op;
  Node &lhs;
  Node &rhs;
  
  NMath(const int &op, Node &lhs, Node &rhs)
      : op(op), lhs(lhs), rhs(rhs) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NBlock : public Node {
 public:
  NodeList statements;
  NBlock() {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NIf : public Node {
 public:
  Node &cond;
  Node &block;
  Node &else_block;

  NIf(Node &cond, Node &block, Node &else_block)
      : cond(cond), block(block), else_block(else_block) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NLoop : public Node {
 public:
  Node &cond;
  Node &block;
  NLoop(Node &cond, Node &block) : cond(cond), block(block) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NCond : public Node {
 public:
  int op;
  Node &lhs;
  Node &rhs;
  

  NCond(const int &op, Node &lhs, Node &rhs)
      : op(op), lhs(lhs), rhs(rhs)  {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NReturn : public Node {
 public:
  Node *ret;
  NReturn(Node *ret = nullptr) : ret(ret) {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};

class NMethodDeclare : public Node {
 public:
  Node &id;
  Node &arguments;
  Node &stmts;
  //NodeList data;
  NMethodDeclare(Node &id, Node &arguments, Node &stmts) : id(id), arguments(arguments), stmts(stmts)  {}

  virtual llvm::Value *codeGen(CodeGenContext &context);
};



#endif