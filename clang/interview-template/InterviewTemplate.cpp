#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

class InterviewASTVisitor
        : public RecursiveASTVisitor<InterviewASTVisitor> {
public:
    explicit InterviewASTVisitor(ASTContext *Context)
            : Context(Context) {}

    bool VisitFunctionDecl(FunctionDecl *Decl) {
        // TODO:
        return true;
    }

private:
    ASTContext *Context;
};

class InterviewASTConsumer : public clang::ASTConsumer {
public:
    explicit InterviewASTConsumer(ASTContext *Context)
            : Visitor(Context) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    InterviewASTVisitor Visitor;
};

class InterviewAction : public clang::ASTFrontendAction {
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
        return std::unique_ptr<clang::ASTConsumer>(
                new InterviewASTConsumer(&Compiler.getASTContext()));
    }
};

static llvm::cl::OptionCategory MyToolCategory("interview-template");
int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<InterviewAction>().get());
}