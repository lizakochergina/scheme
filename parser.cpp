#include "parser.h"
#include <memory>

std::shared_ptr<Object> Read(Tokenizer *tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    auto token = tokenizer->GetToken();
    tokenizer->Next();

    if (token == Token{BracketToken::OPEN}) {
        return ReadList(tokenizer, true);
    } else if (token == Token{BracketToken::CLOSE}) {
        throw SyntaxError("");
    } else if (token == Token{BoolToken::TRUE}) {
        return std::make_shared<Bool>(std::get<BoolToken>(token));
    } else if (token == Token{BoolToken::FALSE}) {
        return std::make_shared<Bool>(std::get<BoolToken>(token));
    } else if (token == Token{DotToken{}}) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("");
        } else {
            return Read(tokenizer);
        }
    } else if (token == Token{QuoteToken{}}) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("");
        } else {
            auto cell = std::make_shared<Cell>();
            cell->SetFirst(std::make_shared<Symbol>(SymbolToken{"quote"}));
            auto tmp = std::make_shared<Cell>();
            tmp->SetFirst(Read(tokenizer));
            tmp->SetSecond(nullptr);
            cell->SetSecond(tmp);
            return cell;
        }
    } else {
        if (std::holds_alternative<SymbolToken>(token)) {
            return std::make_shared<Symbol>(std::get<SymbolToken>(token));
        } else if (std::holds_alternative<ConstantToken>(token)) {
            return std::make_shared<Number>(std::get<ConstantToken>(token));
        }
    }
}
std::shared_ptr<Object> ReadList(Tokenizer *tokenizer, bool with_close_bracket) {
    auto cell = std::make_shared<Cell>();
    if (!tokenizer->IsEnd()) {
        if (tokenizer->GetToken() == Token{DotToken{}}) {
            throw SyntaxError("dot can't be here");
        } else if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {  // CLOSE
            tokenizer->Next();
            return nullptr;
        } else {
            cell->SetFirst(Read(tokenizer));
            if (tokenizer->GetToken() == Token{DotToken{}}) {
                cell->SetSecond(Read(tokenizer));
            } else if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
                cell->SetSecond(nullptr);
            } else {
                auto second = ReadList(tokenizer, false);
                cell->SetSecond(second);
            }

            if (!with_close_bracket) {
                return cell;
            } else if (tokenizer->IsEnd()) {
                throw SyntaxError("");
            } else if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
                tokenizer->Next();
                return cell;
            } else {
                throw SyntaxError("");
            }
        }
    } else {
        throw SyntaxError("");
    }
}
