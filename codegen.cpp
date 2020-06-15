#include "codegen.h"
#include "node.h"
#include <vector>

Value *NInteger::codeGen(CodeGenContext &context)
{
  return ConstantInt::get(Type::getInt32Ty(context.getContext()), value, true);
}

Value *NString::codeGen(CodeGenContext &context)
{
  return ConstantInt::get(Type::getInt8Ty(context.getContext()), value[1],
                          true);
}

Value *NArray::codeGen(CodeGenContext &context) {}

Value *NArray::getElementPtrInst(CodeGenContext &context,
                                 const std::string &identifier, int i)
{
  std::vector<llvm::Value *> vect;

  vect.push_back(ConstantInt::get(Type::getInt32Ty(context.getContext()), 0));
  vect.push_back(ConstantInt::get(Type::getInt32Ty(context.getContext()), i));

  GetElementPtrInst *elemPtr = GetElementPtrInst::CreateInBounds(
      context.locals()[identifier], vect, "", context.currentBlock());

  return elemPtr;
}

Value *NArray::store(Node *rhs, const std::string &identifier,
                     CodeGenContext &context)
{
  NArray *vec = dynamic_cast<NArray *>(rhs);

  StoreInst *st;
  int i = 0;
  for (Node *a : vec->data)
  {
    st = new StoreInst(a->codeGen(context),
                       getElementPtrInst(context, identifier, i), false,
                       context.currentBlock());

    i++;
  }

  return st;
}

Value *NArrid::codeGen(CodeGenContext &context)
{
  NIdentifier *identifier = dynamic_cast<NIdentifier *>(&id);
  LoadInst *ld = new LoadInst(getElementPtrInst(context, identifier->name), "",
                              false, context.currentBlock());
  return ld;
}

Value *NArrid::getElementPtrInst(CodeGenContext &context,
                                 const std::string &identifier)
{
  std::vector<llvm::Value *> vect;

  vect.push_back(ConstantInt::get(Type::getInt32Ty(context.getContext()), 0));
  vect.push_back(idx.codeGen(context));

  GetElementPtrInst *elemPtr = GetElementPtrInst::CreateInBounds(
      context.locals()[identifier], vect, "", context.currentBlock());

  return elemPtr;
}

Value *NIdentifier::codeGen(CodeGenContext &context)
{
  if (context.locals().find(name) == context.locals().end())
  {
    std::cerr << "unknown var " << name;
    return NULL;
  }

  LoadInst *ld =
      new LoadInst(context.locals()[name], "", false, context.currentBlock());

  return ld;
}

Value *NIdentifier::store(Node *rhs, CodeGenContext &context)
{
  StoreInst *st = new StoreInst(rhs->codeGen(context), context.locals()[name],
                                false, context.currentBlock());

  return st;
}

Value *NAssignment::codeGen(CodeGenContext &context)
{
  NIdentifier *lvalue = dynamic_cast<NIdentifier *>(&lhs);

  if (context.locals().find(lvalue->name) == context.locals().end())
  {
    std::string typee = typeid(rhs).name();
    typee = typee.substr(typee.find('N') + 1);
    AllocaInst *alloc;

    if (strcmp(typee.c_str(), "Integer") == 0 ||
        strcmp(typee.c_str(), "Arrid") == 0 ||
        strcmp(typee.c_str(), "Math") == 0)
    {
      alloc = new AllocaInst(Type::getInt32Ty(context.getContext()), 0,
                             lvalue->name.c_str(), context.currentBlock());

      context.locals()[lvalue->name] = alloc;
      context.locals_type()[lvalue->name] = "Integer";
    }
    else if (strcmp(typee.c_str(), "String") == 0)
    {
      alloc = new AllocaInst(IntegerType::get(context.getContext(), 8), 0,
                             lvalue->name.c_str(), context.currentBlock());
      context.locals()[lvalue->name] = alloc;
      context.locals_type()[lvalue->name] = "String";
    }
    else if (strcmp(typee.c_str(), "Identifier") == 0)
    {
      NIdentifier *rvalue = dynamic_cast<NIdentifier *>(&rhs);
      AllocaInst *alloc;
      std::string ty = context.locals_type()[rvalue->name];

      if (strcmp(ty.c_str(), "Integer") == 0)
      {
        alloc = new AllocaInst(Type::getInt32Ty(context.getContext()), 0,
                               lvalue->name.c_str(), context.currentBlock());
      }
      else if (strcmp(ty.c_str(), "String") == 0)
      {
        alloc = new AllocaInst(IntegerType::get(context.getContext(), 8), 0,
                               lvalue->name.c_str(), context.currentBlock());
      }
      context.locals()[lvalue->name] = alloc;
      context.locals_type()[lvalue->name] = ty;
    }
    else if (strcmp(typee.c_str(), "Array") == 0)
    {
      NArray *vec = dynamic_cast<NArray *>(&rhs);

      ArrayType *arrayType = ArrayType::get(
          Type::getInt32Ty(context.getContext()), vec->data.size());

      alloc = new AllocaInst(arrayType, 0, (lvalue->name + ".addr").c_str(),
                             context.currentBlock());

      context.locals()[lvalue->name] = alloc;
      context.locals_type()[lvalue->name] = "AInteger";

      return vec->store(&rhs, lvalue->name, context);
    }
  }
  return lvalue->store(&rhs, context);
}

Value *NMath::codeGen(CodeGenContext &context)
{
  Instruction::BinaryOps instr;

  Value *l = lhs.codeGen(context);
  Value *r = rhs.codeGen(context);

  IRBuilder<> builder(context.currentBlock());
  switch (op)
  {
  case 1:
    return builder.CreateAdd(l, r, "tmp");
  case 2:
    return builder.CreateSub(l, r, "tmp");
  case 3:
    return builder.CreateMul(l, r, "tmp");
  case 4:
    return builder.CreateSDiv(l, r, "tmp");
  default:
    break;
  }
}

Value *NIf::codeGen(CodeGenContext &context)
{
  BasicBlock *then = BasicBlock::Create(context.getContext(), "",
                                        context.currentBlock()->getParent());

  BasicBlock *cond_false = BasicBlock::Create(
      context.getContext(), "", context.currentBlock()->getParent());

  BasicBlock *end = BasicBlock::Create(
      context.getContext(), "end_if", context.currentBlock()->getParent());

  IRBuilder<> builder(context.currentBlock());

  Value *comparison = cond.codeGen(context);
  Value *zero =
      ConstantInt::get(Type::getInt32Ty(context.getContext()), 0, true);
  Value *cellValIsZero = builder.CreateICmpNE(comparison, zero);

  builder.CreateCondBr(cellValIsZero, then, cond_false);

  std::map<std::string, Value *> currLocals = context.locals();

  context.pushBlock(then);
  context.setLocals(currLocals);

  block.codeGen(context);

  IRBuilder<> builder2(context.currentBlock());
  builder2.CreateBr(end);

  context.popBlock();

  context.popBlock();

  context.pushBlock(cond_false);
  context.setLocals(currLocals);

  context.pushBlock(cond_false);
  else_block.codeGen(context);
  context.pushBlock(end);
  context.popBlock();
}

Value *NCond::codeGen(CodeGenContext &context)
{
  Instruction::BinaryOps instr;

  Value *l = lhs.codeGen(context);
  Value *r = rhs.codeGen(context);

  IRBuilder<> builder(context.currentBlock());
  switch (op)
  {
  case 1:
    std::cout << 2;
    return builder.CreateIntCast(builder.CreateICmpEQ(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  case 2:
    return builder.CreateIntCast(builder.CreateICmpNE(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  case 3:
    return builder.CreateIntCast(builder.CreateICmpSLT(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  case 4:
    return builder.CreateIntCast(builder.CreateICmpSLE(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  case 5:
    return builder.CreateIntCast(builder.CreateICmpSGT(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  case 6:
    return builder.CreateIntCast(builder.CreateICmpSGE(l, r, "tmp"),
                                 builder.getInt32Ty(), false, "");
  default:
    return NULL;
  }
}

Value *NBlock::codeGen(CodeGenContext &context)
{
  Value *last = NULL;
  for (Node *a : statements)
  {
    last = a->codeGen(context);
  }
  return last;
}

Value *NLoop::codeGen(CodeGenContext &context)
{
  std::map<std::string, Value *> currLocals = context.locals();

  BasicBlock *loopHeader = BasicBlock::Create(
      context.getContext(), "header_loop", context.currentBlock()->getParent());

  BasicBlock *loopBody = BasicBlock::Create(
      context.getContext(), "loop_body", context.currentBlock()->getParent());

  BasicBlock *loopAfter = BasicBlock::Create(
      context.getContext(), "loop_after", context.currentBlock()->getParent());

  IRBuilder<> builder(context.currentBlock());

  builder.CreateBr(loopHeader);

  context.popBlock();
  context.pushBlock(loopHeader);
  context.setLocals(currLocals);

  IRBuilder<> builderHeader(context.currentBlock());

  Value *comparison = cond.codeGen(context);
  Value *zero =
      ConstantInt::get(Type::getInt32Ty(context.getContext()), 0, true);
  Value *cellValIsZero = builderHeader.CreateICmpNE(comparison, zero);

  builderHeader.CreateCondBr(cellValIsZero, loopBody, loopAfter);

  context.pushBlock(loopBody);
  context.setLocals(currLocals);

  block.codeGen(context);

  IRBuilder<> builderBody(context.currentBlock());
  builderBody.CreateBr(loopHeader);

  context.popBlock();

  context.popBlock();

  context.pushBlock(loopAfter);
  context.setLocals(currLocals);
}

Value *NMethodCall::codeGen(CodeGenContext &context)
{
  NIdentifier *ident = dynamic_cast<NIdentifier *>(&id);
  std::string meth_name = ident->name;
  IRBuilder<> builder(context.currentBlock());

  std::vector<Value *> args;
  NArray *vec = dynamic_cast<NArray *>(&arguments);
  for (Node *a : vec->data)
  {
    std::cout << '1' << std::endl;
    args.push_back(a->codeGen(context));
  }

  std::vector<Type *> putsArgs;
  putsArgs.push_back(builder.getInt32Ty());
  putsArgs.push_back(builder.getInt32Ty());

  ArrayRef<Type *> argsRef(putsArgs);

  FunctionType *putsType =
      FunctionType::get(builder.getInt32Ty(), argsRef, false);
  Constant *putsFunc =
      context.getModule()->getOrInsertFunction(meth_name.c_str(), putsType);

  builder.CreateCall(putsFunc, args);
}

Value *NReturn::codeGen(CodeGenContext &context)
{
  ReturnInst::Create(context.getModule()->getContext(), ret->codeGen(context),
                     context.currentBlock());
}

Value *NMethodDeclare::codeGen(CodeGenContext &context)
{
  
  IRBuilder<> builder(context.currentBlock());
  NIdentifier *ident = dynamic_cast<NIdentifier *>(&id);
  std::string meth_name = ident->name;

  std::vector<Value *> args;
  std::vector<Type *> putsArgs;
  NArray *vec = dynamic_cast<NArray *>(&arguments);
  
  for (Node *a : vec->data)
  {
    std::cout << '2' << std::endl;
    args.push_back(a->codeGen(context));
    putsArgs.push_back(builder.getInt32Ty());
  }
  
  
  
  //llvm::ArrayRef< Value > aa =  ArrayRef();

  //FunctionType* functionType = FunctionType(Type::getInt32Ty(context.getContext()), putsArgs, 1);

  //functionType = FunctionType::get(Type::getInt32Ty(context.getContext()), putsArgs, 1);

  Function *function = Function::Create(FunctionType::get(Type::getInt32Ty(context.getContext()), putsArgs, 1), GlobalValue::InternalLinkage, meth_name, context.getModule());
  BasicBlock *bblock = BasicBlock::Create(context.getContext(), "entry", function, 0);

  


  context.pushBlock(bblock);

  stmts.codeGen(context); //SO USANDO PQ O NOTE TEM O CODEGEN
  ReturnInst::Create(context.getContext(), nullptr, bblock);

  context.popBlock();
  //context.popBlock();

  //return function;
}
