#pragma once

#include <kf/tui.hpp>

#include "zms/drivers/EspnowNode.hpp"
#include "zms/ui/widgets/HexDisplay.hpp"


namespace zms {

/// @brief Страница настроек узла Espnow
struct EspnowNodeSettingsPage final : kf::tui::Page {

    HexDisplay display;

    explicit EspnowNodeSettingsPage(EspnowNode::Settings &settings) :
        kf::tui::Page{"Espnow Node"},
        display{
            {
                settings.remote_controller_mac.data(),
                settings.remote_controller_mac.size()
            }
        } {
        link(MainPage::instance());
        add(display);
    }
};

}// namespace zms
