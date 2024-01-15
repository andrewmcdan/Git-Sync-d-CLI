#include "cli.h"

CLI::CLI()
{
    using namespace ftxui;
    // this->ipc = ipc;
    int custom_loop_count = 0;
    bool rendererUpdated = false;
    bool quit = false;
    std::string logOutput = "";
    
    auto _screen = ScreenInteractive::Fullscreen();
    this->hbox1 = hbox({
        vbox({
            text("Vertical 1") | color(Color::Red) | center | border,
            text("Vertical 2") | color(Color::Green) | center | border,
            text("Vertical 3") | color(Color::Blue) | center | border,
        }) | border,
        vbox({
            text("Vertical 4") | color(Color::Yellow) | center | border,
            text("Vertical 5") | color(Color::Magenta) | center | border,
            text("Vertical 6") | color(Color::Cyan) | center | border,
        }) | border,
    }) | border;
    int frame_count = 0;
    int event_count = 0;
    auto rendererLambda = ([&] {
        frame_count++;
        return vbox({
                    paragraphAlignJustify("This demonstrates using a custom ftxui::Loop. It runs at 100 iterations per seconds. The FTXUI events are all processed once per iteration and a new frame is rendered as needed"),
                    text(logOutput),
                    separator(),
                    text("ftxui event count: " + std::to_string(event_count)),
                    text("ftxui frame count: " + std::to_string(frame_count)),
                    text("Custom loop count: " + std::to_string(custom_loop_count)),
                    separator(),
                    ([&] {
                        std::string s = "";
                        for (int i = 0; i < frame_count; i++) {
                            s += "This is a test string. ";
                        }
                        return paragraphAlignJustify(s);
                    })(),
                    this->hbox1,
               })
            | border;
    });
    auto _renderer = Renderer(rendererLambda);
    
    _renderer |= CatchEvent([&](Event e) -> bool {
        if (e == Event::Character('q')) {
            quit = true;
        }
        event_count++;
        return false;
    });

    Loop loop(&_screen, _renderer);
    int logFnIdx = LOGGER::Log::addLogFunction([&](std::string log) { 
        logOutput = log;        
        _screen.PostEvent(Event::Custom);
    });
    while (!loop.HasQuitted() && !quit) {
        custom_loop_count++;
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(custom_loop_count > 100 && !rendererUpdated){
            this->hbox1 = vbox({
                text("Vertical 1 updated") | color(Color::Red) | center | border,
                text("Vertical 2 updated") | color(Color::Green) | center | border,
                text("Vertical 3 updated") | color(Color::Blue) | center | border,
            }) | border;
            rendererUpdated = true;
        }
        if(custom_loop_count % 50 == 0){
            _screen.PostEvent(Event::Custom);
        }
        if(quit)_screen.Exit();
    }
    LOGGER::Log::removeLogFunction(logFnIdx);
}

CLI::~CLI(){}
