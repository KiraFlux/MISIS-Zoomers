#pragma once

#include <kf/UI.hpp>

#include "zms/Periphery.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница управления хранилищем настроек
struct StoragePage final : kf::UI::Page {

private:

    /// @brief Сохранить настройки
    kf::UI::Button save;

    /// @brief Загрузить настройки
    kf::UI::Button load;

    /// @brief Восстановить значения по умолчанию
    kf::UI::Button restore_defaults;

public:
    explicit StoragePage(Periphery &periphery) :
        Page{"Storage"},
        save{*this, "Save", [&periphery]() { periphery.storage.save(); }},
        load{*this, "Load", [&periphery]() { periphery.storage.load(); }},
        restore_defaults{
            *this,
            "Restore", [&periphery]() {
                periphery.storage.settings = Periphery::defaultSettings();
                periphery.storage.save();
            }
        } {
        link(MainPage::instance());
    }
};

}// namespace zms
