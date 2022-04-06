#pragma once

#include "tokenizer.h"
#include "parser.h"

#include <string>

std::shared_ptr<Object> ReadFullString(const std::string& str);

class Interpreter {
public:
    std::string Run(std::string& expr);
};

std::string RepresentAsStr(const std::shared_ptr<Object>& obj);
