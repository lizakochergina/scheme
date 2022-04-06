#include "object.h"

std::map<std::string, std::shared_ptr<Func>> Symbol::symbol_map = {
    {"boolean?", std::make_shared<IsBool>()},
    {"not", std::make_shared<Not>()},
    {"and", std::make_shared<And>()},
    {"or", std::make_shared<Or>()},
    {"quote", std::make_shared<Quote>()},
    {"pair?", std::make_shared<IsPair>()},
    {"null?", std::make_shared<IsNull>()},
    {"list?", std::make_shared<IsList>()},
    {"cons", std::make_shared<Cons>()},
    {"car", std::make_shared<Car>()},
    {"cdr", std::make_shared<Cdr>()},
    {"list", std::make_shared<List>()},
    {"list-ref", std::make_shared<ListRef>()},
    {"list-tail", std::make_shared<ListTail>()},
    {"number?", std::make_shared<IsNumber>()},
    {"=", std::make_shared<IsEqual>()},
    {">", std::make_shared<IsDecrease>()},
    {"<", std::make_shared<IsIncrease>()},
    {">=", std::make_shared<IsNonIncrease>()},
    {"<=", std::make_shared<IsNonDecrease>()},
    {"+", std::make_shared<Sum>()},
    {"-", std::make_shared<Sub>()},
    {"*", std::make_shared<Prod>()},
    {"/", std::make_shared<Div>()},
    {"max", std::make_shared<Max>()},
    {"min", std::make_shared<Min>()},
    {"abs", std::make_shared<Abs>()}};

void GetVector(const std::shared_ptr<Object>& args, std::vector<std::shared_ptr<Object>>& obj) {
    if (args) {
        if (!Is<Cell>(args)) {
            obj.push_back(args->Eval());
        }
        if (!As<Cell>(args)->GetFirst()) {
            throw RuntimeError("invalid arg, trying to evaluate null cell");
        }
        obj.push_back(As<Cell>(args)->GetFirst()->Eval());
        GetVector(As<Cell>(args)->GetSecond(), obj);
    }
}

void GetRawVector(const std::shared_ptr<Object>& args, std::vector<std::shared_ptr<Object>>& obj) {
    if (args) {
        if (!Is<Cell>(args)) {
            obj.push_back(args);
            return;
        }
        obj.push_back(As<Cell>(args)->GetFirst());
        GetRawVector(As<Cell>(args)->GetSecond(), obj);
    }
}

std::shared_ptr<Object> GetObjFrowVector(std::vector<std::shared_ptr<Object>>& obj, size_t i) {
    if (i >= obj.size()) {
        return nullptr;
    } else {
        auto cell = std::make_shared<Cell>();
        cell->SetFirst(obj[i]);
        cell->SetSecond(GetObjFrowVector(obj, i + 1));
        return cell;
    }
}

std::shared_ptr<Object> MakeArgsForList(std::shared_ptr<Object>& obj) {
    auto quoted_expr_cell = std::make_shared<Cell>();
    quoted_expr_cell->SetFirst(obj);

    auto quote_cell = std::make_shared<Cell>();
    quote_cell->SetFirst(std::make_shared<Symbol>(SymbolToken{"quote"}));
    quote_cell->SetSecond(quoted_expr_cell);

    auto args = std::make_shared<Cell>();
    args->SetFirst(quote_cell);

    return args;
}