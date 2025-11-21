#pragma once

#include <kf/Logger.hpp>
#include <kf/UI.hpp>

#include "zms/drivers/Encoder.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница индивидуальных настроек энкодера
struct EncoderTunePage final : kf::UI::Page {

private:
    using TicksDisplay = kf::UI::Labeled<kf::UI::Display<Encoder::Ticks>>;
    TicksDisplay ticks_display;

    kf::UI::Labeled<kf::UI::CheckBox> enabled;

    kf::UI::ComboBox<Encoder::PinsSettings::Edge, 2> edge;

public:
    explicit EncoderTunePage(
        const char *encoder_name,
        Encoder &encoder,
        Encoder::PinsSettings &settings
    ) :
        kf::UI::Page{encoder_name},
        ticks_display{
            *this,
            "Pos",
            TicksDisplay::Impl{*this, encoder.position}
        },
        enabled{
            *this,
            "Enabled",
            kf::UI::CheckBox{
                [&encoder, encoder_name](bool e) {
                    kf_Logger_info("%s: %s", encoder_name, e ? "Enabled" : "Disabled");
                    if (e) {
                        encoder.enable();
                    } else {
                        encoder.disable();
                    }
                }
            }
        },
        edge{
            *this,
            {
                {
                    {"Rising", Encoder::PinsSettings::Edge::Rising},
                    {"Falling", Encoder::PinsSettings::Edge::Falling},
                }
            },
            settings.edge
        } {
        link(MainPage::instance());
    }
};

}// namespace zms
