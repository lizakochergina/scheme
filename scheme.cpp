#include "scheme.h"
#include <sstream>

std::shared_ptr<Object> ReadFullString(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    auto res = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("syntax error");
    }
    return res;
}

std::string RepresentAsStr(const std::shared_ptr<Object>& obj, bool brackets) {
    std::string s;
    auto cur = obj;
    if (!cur) {
        return "()";
    } else if (Is<Number>(cur)) {
        return std::to_string(As<Number>(cur)->GetValue());
    } else if (Is<Symbol>(cur)) {
        return As<Symbol>(cur)->GetName();
    } else if (Is<Bool>(cur)) {
        return As<Bool>(cur)->GetState();
    } else {
        if (brackets) {
            s += '(';
        }
        auto cell = As<Cell>(cur);
        s += RepresentAsStr(cell->GetFirst(), true);
        if (cell->GetSecond()) {
            s += ' ';
            if (!Is<Cell>(cell->GetSecond())) {
                s += ". ";
            }
            s += RepresentAsStr(cell->GetSecond(), false);
        }
        if (brackets) {
            s += ')';
        }
    }
    return s;
}

std::string Interpreter::Run(std::string& expr) {
    auto obj = ReadFullString(expr);
    if (!obj) {
        throw RuntimeError("this is void");
    }
    auto res = obj->Eval();
    return RepresentAsStr(res, true);
}
