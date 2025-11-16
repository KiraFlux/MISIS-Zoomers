#pragma once

#include <functional>
#include <kf/Logger.hpp>
#include <kf/tools/time/TimeoutManager.hpp>


namespace zms {

/// @brief Работает с пакетами данных с пульта
struct DualJoystickRemoteController {

    /// @brief Пакет данных управления
    struct ControlPacket {
        /// @brief Левый стик, ось X
        float left_x{0};

        /// @brief Левый стик, ось Y
        float left_y{0};

        /// @brief Правый стик, ось X
        float right_x{0};

        /// @brief Правый стик, ось Y
        float right_y{0};
    };

private:
    /// @brief Менеджер тайм-аута пакета
    kf::tools::TimeoutManager packet_timeout_manager;

    /// @brief Флаг отключения
    bool disconnected{true};

    /// @brief Актуальный пакет данных
    ControlPacket packet{};

public:
    /// @brief Обработчик входящего пакета
    std::function<void(const ControlPacket &)> control_handler{nullptr};

    std::function<void(void)> disconnect_handler{nullptr};

    explicit DualJoystickRemoteController(kf::Milliseconds packet_timeout) :
        packet_timeout_manager{packet_timeout} {}

    /// @brief Прокрутка событий
    void poll() {
        if (packet_timeout_manager.expired()) {
            if (not disconnected) {
                kf_Logger_info("disconnected");

                disconnected = true;
                resetControlPacket();

                if (disconnect_handler) disconnect_handler();
            }
        } else {
            disconnected = false;
            if (control_handler) { control_handler(packet); }
        }
    }

    /// @brief Сбросить значение пакета управления
    void resetControlPacket() {
        packet = ControlPacket{};
    }

    /// @brief Обновить пакет
    void updateControlPacket(const ControlPacket &p) {
        packet = p;
        packet_timeout_manager.update();
    }
};

}// namespace zms
