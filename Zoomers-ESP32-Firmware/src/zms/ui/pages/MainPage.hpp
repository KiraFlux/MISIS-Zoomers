#pragma once

#include <kf/UI.hpp>
#include <kf/tools/meta/Singleton.hpp>


namespace zms {

/// @brief Основная страница
struct MainPage final : kf::UI::Page, kf::tools::Singleton<MainPage> {
    friend struct Singleton<MainPage>;

    explicit MainPage() :
        Page{"Main"} {}
};

}// namespace zms
