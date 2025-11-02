#pragma once

#include <kf/tui.hpp>

#include "zms/Periphery.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница управления хранилищем настроек
struct StoragePage final : kf::tui::Page {

private:
    /// Сохранить настройки
    kf::tui::Button save;

    /// Загрузить настройки
    kf::tui::Button load;

    /// Восстановить значения по умолчанию
    kf::tui::Button restore_defaults;

public:
    explicit StoragePage(Periphery &periphery) :
        Page{"Storage"},
        save{"Save", [&periphery]() { periphery.storage.save(); }},
        load{"Load", [&periphery]() { periphery.storage.load(); }},
        restore_defaults{
            "Restore", [&periphery]() {
                periphery.storage.settings = Periphery::defaultSettings();
                periphery.storage.save();
            }
        } {
        link(MainPage::instance());
        add(save);
        add(load);
        add(restore_defaults);
    }
};

}// namespace zms
