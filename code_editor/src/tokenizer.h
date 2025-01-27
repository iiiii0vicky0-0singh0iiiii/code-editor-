#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

class Tokenizer {
public:
    std::vector<std::string> tokenize(const std::string& input);
};

#endif 
