#pragma once

#include <kf/UI.hpp>

#include "zms/drivers/Encoder.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница настроек преобразований энкодера
struct EncoderConversionSettingsPage final : kf::UI::Page {

    explicit EncoderConversionSettingsPage(Encoder::ConversionSettings &settings) :
        Page{"Enc Conversion"} {
        link(MainPage::instance());
    }
};

}// namespace zms
