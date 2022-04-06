#pragma once

#include <memory>
#include <map>
#include <vector>
#include <iostream>
#include <type_traits>
#include "tokenizer.h"
#include "error.h"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::shared_ptr<Object> Eval() {
    }
    virtual std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) {
        throw RuntimeError("not a function");
    }
};

template <class T>
bool Is(const std::shared_ptr<Object>& obj);

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj);

class Func : public Object {
public:
    virtual std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) {
    }
};

class Number : public Object {
public:
    Number(const ConstantToken& token) : value_(token.value) {
    }
    int64_t GetValue() const {
        return value_;
    }
    std::shared_ptr<Object> Eval() override {
        return shared_from_this();
    }

private:
    int64_t value_;
};

class Symbol : public Object {
public:
    Symbol(const SymbolToken& token) : name_(token.name) {
    }
    const std::string& GetName() const {
        return name_;
    }
    std::shared_ptr<Object> Eval() override {
        return symbol_map[name_];
    }

private:
    std::string name_;
    static std::map<std::string, std::shared_ptr<Func>> symbol_map;
};

class Bool : public Object {
public:
    Bool(const BoolToken& token) {
        if (token == BoolToken::TRUE) {
            state_ = "#t";
        } else {
            state_ = "#f";
        }
    }
    Bool(const std::string& state) : state_(state) {
    }
    Bool(bool state) {
        if (state) {
            state_ = "#t";
        } else {
            state_ = "#f";
        }
    }
    const std::string& GetState() const {
        return state_;
    }
    std::shared_ptr<Object> Eval() override {
        return std::make_shared<Bool>(state_);
    }

    bool IsTrue() const {
        if (state_ == "#t") {
            return true;
        } else {
            return false;
        }
    }

private:
    std::string state_;
};

class Cell : public Object {
public:
    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    void SetFirst(const std::shared_ptr<Object> other) {
        first_ = other;
    }
    void SetSecond(const std::shared_ptr<Object> other) {
        second_ = other;
    }

    std::shared_ptr<Object> Eval() override {
        if (first_) {
            auto evalueted = first_->Eval();
            if (evalueted) {
                return evalueted->Apply(second_);
            }
        }
        throw RuntimeError("cannot evaluate");
    }

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    if (obj && Is<T>(obj)) {
        return std::dynamic_pointer_cast<T>(obj);
    } else {
        throw RuntimeError("cannot cast");
    }
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    if (!obj) {
        return false;
    }
    try {
        auto ptr = dynamic_cast<T&>(*obj);
        return true;
    } catch (const std::bad_cast& e) {
        return false;
    }
}

void GetVector(const std::shared_ptr<Object>& args, std::vector<std::shared_ptr<Object>>& obj);
void GetRawVector(const std::shared_ptr<Object>& args, std::vector<std::shared_ptr<Object>>& obj);
std::shared_ptr<Object> GetObjFrowVector(std::vector<std::shared_ptr<Object>>& obj, size_t i);
std::shared_ptr<Object> MakeArgsForList(std::shared_ptr<Object>& obj);

template <class T>
bool ValidateObj(std::vector<std::shared_ptr<Object>>& obj) {
    for (auto& el : obj) {
        if (!Is<T>(el)) {
            return false;
        }
    }
    return true;
}

class IsBool : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        auto cell = As<Cell>(args);
        auto evalueted = cell->GetFirst()->Eval();
        if (cell->GetSecond()) {
            throw RuntimeError("wrong cnt of elements");
        }
        if (evalueted) {
            if (Is<Bool>(evalueted)) {
                return std::make_shared<Bool>(BoolToken::TRUE);
            } else {
                return std::make_shared<Bool>(BoolToken::FALSE);
            }
        } else {
            return std::make_shared<Bool>(BoolToken::FALSE);
        }
    }
};
class Not : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetRawVector(args, obj);
        if (obj.size() != 1) {
            throw RuntimeError("cnt of args is not valid");
        }
        if (obj[0] && Is<Bool>(obj[0])) {
            auto b = As<Bool>(obj[0]);
            if (b->GetState() == "#t") {
                return std::make_shared<Bool>(BoolToken::FALSE);
            } else {
                return std::make_shared<Bool>(BoolToken::TRUE);
            }
        } else {
            return std::make_shared<Bool>(BoolToken::FALSE);
        }
    }
};
class And : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetRawVector(args, obj);
        if (obj.empty()) {
            return std::make_shared<Bool>(BoolToken::TRUE);
        }
        for (auto& el : obj) {
            auto eval = el->Eval();
            if (Is<Bool>(eval) && !As<Bool>(eval)->IsTrue()) {
                return std::make_shared<Bool>(BoolToken::FALSE);
            }
        }
        return obj.back()->Eval();
    }
};
class Or : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetRawVector(args, obj);
        if (obj.empty()) {
            return std::make_shared<Bool>(BoolToken::FALSE);
        }
        for (auto& el : obj) {
            auto eval = el->Eval();
            if (!Is<Bool>(eval) || As<Bool>(eval)->IsTrue()) {
                return eval;
            }
        }
        return std::make_shared<Bool>(BoolToken::FALSE);
    }
};
class Quote : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        auto cell = As<Cell>(args);
        if (cell->GetSecond()) {
            throw RuntimeError("wrong cnt of elements");
        }
        return cell->GetFirst();
    }
};
class IsPair : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.size() != 1) {
            return std::make_shared<Bool>(BoolToken::FALSE);
        } else {
            if (obj[0] && Is<Cell>(obj[0])) {
                return std::make_shared<Bool>(BoolToken::TRUE);
            } else {
                return std::make_shared<Bool>(BoolToken::FALSE);
            }
        }
    }
};
class IsNull : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.size() != 1) {
            return std::make_shared<Bool>(BoolToken::FALSE);
        } else {
            std::vector<std::shared_ptr<Object>> elems;
            GetRawVector(obj[0], elems);
            if (!elems.empty()) {
                return std::make_shared<Bool>(BoolToken::FALSE);
            }
        }
        return std::make_shared<Bool>(BoolToken::TRUE);
    }
};
class IsList : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.size() != 1) {
            throw RuntimeError("invalid cnt of args");
        }
        if (!obj[0]) {
            return std::make_shared<Bool>(BoolToken::TRUE);
        } else {
            if (Is<Cell>(obj[0])) {
                auto sec = As<Cell>(obj[0])->GetSecond();
                return std::make_shared<IsList>()->Apply(MakeArgsForList(sec));
            } else {
                return std::make_shared<Bool>(BoolToken::FALSE);
            }
        }
    }
};
class Cons : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.empty()) {
            return nullptr;
        } else if (obj.size() == 1) {
            auto cell = std::make_shared<Cell>();
            cell->SetFirst(obj[0]);
            return cell;
        } else if (obj.size() == 2) {
            auto cell = std::make_shared<Cell>();
            cell->SetFirst(obj[0]);
            cell->SetSecond(obj[1]);
            return cell;
        } else {
            throw RuntimeError("too many args");
        }
    }
};
class Car : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        if (Is<Cell>(args) && Is<Cell>(As<Cell>(args)->GetFirst())) {
            auto eval = As<Cell>(As<Cell>(args)->GetFirst())->Eval();
            if (!eval) {
                throw RuntimeError("smth is wrong");
            }
            std::vector<std::shared_ptr<Object>> obj;
            GetRawVector(eval, obj);
            if (!obj.empty()) {
                return obj[0];
            } else {
                throw RuntimeError("too few args");
            }
        } else {
            throw NameError("smth is wrong");
        }
    }
};
class Cdr : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        if (Is<Cell>(args) && Is<Cell>(As<Cell>(args)->GetFirst())) {
            auto eval = As<Cell>(As<Cell>(args)->GetFirst())->Eval();
            if (!eval) {
                throw RuntimeError("smth is wrong");
            }
            if (!Is<Cell>(eval)) {
                throw RuntimeError("too few args");
            } else {
                return As<Cell>(eval)->GetSecond();
            }
        } else {
            throw NameError("smth is wrong");
        }
    }
};
class List : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        return GetObjFrowVector(obj, 0);
    }
};
class ListRef : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.size() == 2 && obj[1]) {
            size_t id = As<Number>(obj[1])->GetValue();
            std::vector<std::shared_ptr<Object>> list;
            GetRawVector(obj[0], list);
            if (id < list.size()) {
                return list[id];
            }
        }
        throw RuntimeError("smth is wrong");
    }
};
class ListTail : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (obj.size() == 2 && obj[1]) {
            size_t id = As<Number>(obj[1])->GetValue();
            std::vector<std::shared_ptr<Object>> list;
            GetRawVector(obj[0], list);
            if (id <= list.size()) {
                return GetObjFrowVector(list, id);
            }
        }
        throw RuntimeError("smth is wrong");
    }
};
class IsNumber : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        auto cell = As<Cell>(args);
        if (cell->GetSecond()) {
            throw RuntimeError("cnt of args is not valid");
        }
        return std::make_shared<Bool>(Is<Number>(cell->GetFirst()));
    }
};
class IsEqual : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() == 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        bool eq = true;
        for (size_t i = 1; i < obj.size(); ++i) {
            if (As<Number>(obj[i - 1])->GetValue() != As<Number>(obj[i])->GetValue()) {
                eq = false;
                break;
            }
        }
        return std::make_shared<Bool>(eq);
    }
};
class IsDecrease : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() == 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        bool eq = true;
        for (size_t i = 1; i < obj.size(); ++i) {
            if (As<Number>(obj[i - 1])->GetValue() <= As<Number>(obj[i])->GetValue()) {
                eq = false;
                break;
            }
        }
        return std::make_shared<Bool>(eq);
    }
};
class IsIncrease : public Func {  // <
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() == 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        bool eq = true;
        for (size_t i = 1; i < obj.size(); ++i) {
            if (As<Number>(obj[i - 1])->GetValue() >= As<Number>(obj[i])->GetValue()) {
                eq = false;
                break;
            }
        }
        return std::make_shared<Bool>(eq);
    }
};
class IsNonIncrease : public Func {  // >=
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() == 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        bool eq = true;
        for (size_t i = 1; i < obj.size(); ++i) {
            if (As<Number>(obj[i - 1])->GetValue() < As<Number>(obj[i])->GetValue()) {
                eq = false;
                break;
            }
        }
        return std::make_shared<Bool>(eq);
    }
};
class IsNonDecrease : public Func {  // <=
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() == 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        bool eq = true;
        for (size_t i = 1; i < obj.size(); ++i) {
            if (As<Number>(obj[i - 1])->GetValue() > As<Number>(obj[i])->GetValue()) {
                eq = false;
                break;
            }
        }
        return std::make_shared<Bool>(eq);
    }
};
class Sum : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }

        int64_t sum = 0;
        for (auto& el : obj) {
            sum += As<Number>(el)->GetValue();
        }
        return std::make_shared<Number>(ConstantToken{sum});
    }
};
class Sub : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() < 2) {
            throw RuntimeError("cnt of args is not valid");
        }

        int64_t sub = As<Number>(obj[0])->GetValue();
        for (size_t i = 1; i != obj.size(); ++i) {
            sub -= As<Number>(obj[i])->GetValue();
        }
        return std::make_shared<Number>(ConstantToken{sub});
    }
};
class Prod : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }

        int64_t prod = 1;
        for (auto& el : obj) {
            prod *= As<Number>(el)->GetValue();
        }
        return std::make_shared<Number>(ConstantToken{prod});
    }
};
class Div : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() < 2) {
            throw RuntimeError("cnt of args is not valid");
        }

        int64_t mul = As<Number>(obj[0])->GetValue();
        for (size_t i = 1; i != obj.size(); ++i) {
            if (As<Number>(obj[i])->GetValue() == 0) {
                throw RuntimeError("division by zero");
            }
            mul /= As<Number>(obj[i])->GetValue();
        }
        return std::make_shared<Number>(ConstantToken{mul});
    }
};
class Max : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.empty()) {
            throw RuntimeError("cnt of args is not valid");
        }

        int64_t max_el = As<Number>(obj[0])->GetValue();
        for (auto& el : obj) {
            max_el = std::max(max_el, As<Number>(el)->GetValue());
        }
        return std::make_shared<Number>(ConstantToken{max_el});
    }
};
class Min : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.empty()) {
            throw RuntimeError("cnt of args is not valid");
        }

        int64_t min_el = As<Number>(obj[0])->GetValue();
        for (auto& el : obj) {
            min_el = std::min(min_el, As<Number>(el)->GetValue());
        }
        return std::make_shared<Number>(ConstantToken{min_el});
    }
};
class Abs : public Func {
    std::shared_ptr<Object> Apply(const std::shared_ptr<Object>& args) override {
        std::vector<std::shared_ptr<Object>> obj;
        GetVector(args, obj);
        if (!ValidateObj<Number>(obj)) {
            throw RuntimeError("type of args is not valid");
        }
        if (obj.size() != 1) {
            throw RuntimeError("cnt of args is not valid");
        }

        return std::make_shared<Number>(ConstantToken{std::abs(As<Number>(obj[0])->GetValue())});
    }
};