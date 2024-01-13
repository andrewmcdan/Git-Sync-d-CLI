#pragma once
#ifndef _CLI_H_
#define _CLI_H_

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include <iostream>
#include <string>
class CLI {
public:
    CLI();
    ~CLI();
    void run();
private:
};

#endif // !_CLI_H_