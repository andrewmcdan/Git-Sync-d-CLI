#include "cli.h"

CLI::CLI()
{
    // using namespace ftxui;

    auto summary = [&] {
        auto content = ftxui::vbox({
            ftxui::hbox({ ftxui::text(L"- done:   "), ftxui::text(L"3") | ftxui::bold }) | ftxui::color(ftxui::Color::Green),
            ftxui::hbox({ ftxui::text(L"- active: "), ftxui::text(L"2") | ftxui::bold }) | ftxui::color(ftxui::Color::RedLight),
            ftxui::hbox({ ftxui::text(L"- queue:  "), ftxui::text(L"9") | ftxui::bold }) | ftxui::color(ftxui::Color::Red),
        });
        return ftxui::window(ftxui::text(L" Summary "), content);
    };

    auto document = //
        ftxui::vbox({
            ftxui::hbox({
                summary(),
                summary(),
                summary() | ftxui::flex,
            }),
            summary(),
            summary(),
        });

    // Limit the size of the document to 80 char.
    document = document | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 80);

    auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(document));
    Render(screen, document);

    std::cout << screen.ToString() << '\0' << std::endl;
}

CLI::~CLI()
{
}