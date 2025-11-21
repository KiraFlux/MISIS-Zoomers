#pragma once

#include <kf/tools/meta/Singleton.hpp>
#include <kf/aliases.hpp>

#include "zms/Periphery.hpp"
#include "zms/services/ByteLangBridgeProtocol.hpp"
#include "zms/services/DualJoystickRemoteController.hpp"
#include "zms/services/TextUI.hpp"


namespace zms {

/// @brief Единственная и глобальная точках входа и связывания всех дополнительных сервисов робота
struct Service final : kf::tools::Singleton<Service> {
    friend struct Singleton<Service>;

    /// @brief Менеджер текстового пользовательского интерфейса
    TextUI text_ui{};

    /// @brief Удаленный контроллер
    DualJoystickRemoteController dual_joystick_remote_controller{200};

    /// @brief ByteLang мост
    ByteLangBridgeProtocol bytelang_bridge{};

    /// @brief Инициализация сервисов
    void init() {
        static auto &periphery = zms::Periphery::instance();

        periphery.espnow_peer.value().setReceiveHandler([this](kf::slice<const void> data) {
            /// Действие в меню
            enum Action : kf::u8 {
                None = 0x00,
                Reload = 0x10,
                Click = 0x20,
                Left = 0x30,
                Right = 0x31,
                Up = 0x40,
                Down = 0x41
            };

            auto translateActionToEvent = [](Action code) {
                using Event = kf::UI::Event;

                switch (code) {
                    case Action::Reload: return Event::Update;
                    case Action::Click: return Event::Click;
                    case Action::Left: return Event::ChangeIncrement;
                    case Action::Right: return Event::ChangeDecrement;
                    case Action::Up: return Event::ElementPrevious;
                    case Action::Down: return Event::ElementNext;
                    case Action::None:
                    default: return Event::None;
                }
            };

            switch (data.size) {
                case sizeof(zms::DualJoystickRemoteController::ControlPacket)://
                    dual_joystick_remote_controller.updateControlPacket(*static_cast<const zms::DualJoystickRemoteController::ControlPacket *>(data.ptr));
                    return;

                case sizeof(Action)://
                    text_ui.addEvent(translateActionToEvent(*static_cast<const Action *>(data.ptr)));
                    return;

                default: kf_Logger_warn("Unknown packet: (%d bytes)", data.size);
            }
        });

        dual_joystick_remote_controller.control_handler = [](const DualJoystickRemoteController::ControlPacket &packet) {
            periphery.left_motor.set(packet.left_y + packet.left_x);
            periphery.right_motor.set(packet.left_y - packet.left_x);

            periphery.manipulator.setArm(static_cast<kf::Degrees>(packet.right_y * 45 + 90 + 45));
            periphery.manipulator.setClaw(static_cast<kf::Degrees>(packet.right_x * 90 + 90));
        };

        dual_joystick_remote_controller.disconnect_handler = []() {
            periphery.left_motor.stop();
            periphery.right_motor.stop();

            periphery.manipulator.disable();
        };

        text_ui.send_handler = [](kf::slice<const kf::u8> slice) -> bool {
            const auto send_result = periphery.espnow_peer.value().sendBuffer(
                kf::slice<const void>{
                    slice.ptr,
                    slice.size
                }
            );

            if (not send_result.isOk()) {
                kf_Logger_error("text ui send fail: %s", kf::EspNow::stringFromError(send_result.error().value()));
                return false;
            }

            return true;
        };
    }

    /// @brief Прокрутка событий сервисов
    void poll() {
        text_ui.poll();
        bytelang_bridge.poll();
        dual_joystick_remote_controller.poll();
    };
};

}// namespace zms
