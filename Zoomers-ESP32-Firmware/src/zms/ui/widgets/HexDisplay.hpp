#pragma once

#include <kf/tui.hpp>
#include <kf/aliases.hpp>


namespace zms {

struct HexDisplay final : kf::tui::Widget {

private:
    kf::slice<const kf::u8> view;

public:
    explicit HexDisplay(const kf::slice<const kf::u8> &v) :
        view{v} {}

    void doRender(kf::tui::BufferStream &stream) const override {
        for (kf::usize i = 0; i < view.size; i += 1) {
            stream.print(view.ptr[i], HEX);
        }
    }
};

}// namespace zms
