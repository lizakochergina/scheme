#include "tokenizer.h"
#include "error.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}
bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}
bool DotToken::operator==(const DotToken&) const {
    return true;
}
bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : in_(in) {
    Next();
}

void Tokenizer::Next() {
    std::set<char> first_pack{'<', '=', '>', '*', '#'};
    std::set<char> second_pack{'<', '=', '>', '*', '#', '?', '!', '-'};

    char ch = in_->peek();
    if (ch == EOF) {
        is_end_ = true;
        return;
    }

    while (std::isspace(ch)) {
        in_->get();
        ch = in_->peek();
        if (ch == EOF) {
            is_end_ = true;
            return;
        }
    }

    if (ch == '\'') {
        in_->get();
        next_ = QuoteToken{};
        is_end_ = false;
    } else if (ch == '.') {
        in_->get();
        next_ = DotToken{};
        is_end_ = false;
    } else if (ch == '(') {
        in_->get();
        next_ = BracketToken::OPEN;
        is_end_ = false;
    } else if (ch == ')') {
        in_->get();
        next_ = BracketToken::CLOSE;
        is_end_ = false;
    } else if (ch == '*' || ch == '/') {
        in_->get();
        std::string s;
        s += ch;
        next_ = SymbolToken{s};
        is_end_ = false;
    } else if (ch == '+' || ch == '-') {
        in_->get();
        char next_ch = in_->peek();
        if (next_ch == EOF || std::isspace(next_ch)) {
            std::string s;
            s += ch;
            next_ = SymbolToken{s};
        } else if (std::isdigit(next_ch)) {
            std::string num;
            while (std::isdigit(next_ch)) {
                num += next_ch;
                in_->get();
                next_ch = in_->peek();
            }
            if (ch == '+') {
                next_ = ConstantToken{std::stoi(num)};
            } else {
                next_ = ConstantToken{std::stoi(num) * (-1)};
            }
        } else {
            std::string s;
            s += ch;
            next_ = SymbolToken{s};
        }
        is_end_ = false;
    } else if (std::isdigit(ch)) {
        std::string num;
        num += ch;
        ch = in_->get();
        ch = in_->peek();
        while (ch != EOF && !std::isspace(ch) && std::isdigit(ch)) {
            num += ch;
            in_->get();
            ch = in_->peek();
        }
        next_ = ConstantToken{std::stoi(num)};
        is_end_ = false;
    } else if (ch == '#') {
        in_->get();
        ch = in_->peek();
        if (ch == 't') {
            next_ = BoolToken::TRUE;
        } else if (ch == 'f') {
            next_ = BoolToken::FALSE;
        } else {
            throw SyntaxError("");
        }
        in_->get();
        is_end_ = false;
    } else if (std::isalpha(ch) || first_pack.find(ch) != first_pack.end()) {
        std::string s;
        s += ch;
        in_->get();
        ch = in_->peek();
        while (
            ch != EOF && !std::isspace(ch) &&
            (std::isalpha(ch) || std::isdigit(ch) || second_pack.find(ch) != second_pack.end())) {
            s += ch;
            in_->get();
            ch = in_->peek();
        }
        next_ = SymbolToken{s};
        is_end_ = false;
    } else {
        throw SyntaxError("");
    }
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

Token Tokenizer::GetToken() {
    return next_;
}
