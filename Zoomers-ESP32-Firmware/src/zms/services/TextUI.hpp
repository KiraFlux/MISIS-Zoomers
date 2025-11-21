#pragma once

#include <functional>
#include <kf/Logger.hpp>
#include <kf/UI.hpp>

#include "zms/Periphery.hpp"
#include "zms/ui/pages/EncoderConversionSettingsPage.hpp"
#include "zms/ui/pages/EncoderTunePage.hpp"
#include "zms/ui/pages/MainPage.hpp"
#include "zms/ui/pages/MotorPwmSettingsPage.hpp"
#include "zms/ui/pages/MotorTunePage.hpp"
#include "zms/ui/pages/StoragePage.hpp"


namespace zms {

/// @brief ZMS Text UI
struct TextUI final {

    /// @brief Обработчик отправки UI
    std::function<bool(kf::slice<const kf::u8>)> send_handler{nullptr};

private:
    /// @brief Страница управления хранилищем настроек
    StoragePage storage_page;

    //

    /// @brief Страница настройки драйверов моторов
    MotorTunePage left_motor_tune_page, right_motor_tune_page;

    /// @brief Страница настройки ШИМ драйвера моторов
    MotorPwmSettingsPage motor_pwm_settings_page;

    //

    /// @brief Страница настройки энкодеров
    EncoderTunePage left_encoder_tune_page, right_encoder_tune_page;

    /// @brief Страница настройки конвертации энкодеров
    EncoderConversionSettingsPage encoder_conversion_settings_page;

    //

public:
    /// @brief Публичный конструктор для сервиса
    explicit TextUI() :
        TextUI{Periphery::instance()} {}

    /// @brief Добавить событие в очередь
    /// @param event
    void addEvent(kf::UI::Event event) {
        auto &page_manager = kf::UI::instance();

        page_manager.addEvent(event);
    }

    /// @brief Опрос событий
    void poll() const {
        auto &page_manager = kf::UI::instance();
        const bool update_required = page_manager.pollEvents();

        if (not update_required) { return; }

        const auto slice = page_manager.render();

        if (nullptr == send_handler) {
            kf_Logger_warn("sender is null");
            return;
        }

        const auto send_ok = send_handler(slice);

        if (not send_ok) {
            kf_Logger_error("send failed");
            return;
        }

        kf_Logger_debug("%d bytes send", slice.size);
    }

private:
    explicit TextUI(Periphery &p) :

        storage_page{p},

        left_motor_tune_page{
            "Motor L",
            p.left_motor,
            p.storage.settings.motor_pwm,
            p.storage.settings.left_motor
        },

        right_motor_tune_page{
            "Motor R",
            p.right_motor,
            p.storage.settings.motor_pwm,
            p.storage.settings.right_motor
        },

        motor_pwm_settings_page{
            p.storage.settings.motor_pwm
        },

        left_encoder_tune_page{
            "Encoder L",
            p.left_encoder,
            p.storage.settings.left_encoder
        },

        right_encoder_tune_page{
            "Encoder R",
            p.right_encoder,
            p.storage.settings.left_encoder
        },

        encoder_conversion_settings_page{
            p.storage.settings.encoder_conversion
        } {

        kf::UI::instance().bind(MainPage::instance());
    }
};

}// namespace zms
