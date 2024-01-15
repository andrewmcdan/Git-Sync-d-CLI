#pragma once
#ifndef _CLI_H_
#define _CLI_H_

#include <memory>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <string>

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include "ftxui/component/loop.hpp"
#include "ftxui/component/screen_interactive.hpp"

#ifdef _WIN32
#include "../windows/ipc.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
#include "../unix/ipc.h"
#endif

class CLI {
public:
    CLI();
    ~CLI();
    void run();
private:
    INTERPROCESS::IPC* ipc;
    std::thread cli_thread;
    ftxui::Element hbox1;
};

#endif // !_CLI_H_