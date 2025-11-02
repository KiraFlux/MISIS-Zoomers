#pragma once

#include <kf/tui.hpp>
#include <kf/tools/meta/Singleton.hpp>


namespace zms {

/// @brief Основная страница
struct MainPage final : kf::tui::Page, kf::tools::Singleton<MainPage> {
    friend struct Singleton<MainPage>;

    explicit MainPage() :
        kf::tui::Page{"Main"} {}
};

}// namespace zms
