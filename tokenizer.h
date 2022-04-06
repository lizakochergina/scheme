#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <vector>
#include <exception>
#include <set>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

enum class BoolToken { TRUE, FALSE };

struct ConstantToken {
    int64_t value;

    bool operator==(const ConstantToken& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BoolToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    std::istream* in_;
    Token next_;
    bool is_end_ = false;
};
