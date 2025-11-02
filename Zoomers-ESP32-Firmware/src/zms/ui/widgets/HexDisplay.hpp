#pragma once

#include <kf/tui.hpp>
#include <kf/aliases.hpp>


namespace zms {

struct HexDisplay final : kf::tui::Widget {

private:
    kf::tui::TextStream::Slice view;

public:
    explicit HexDisplay(kf::tui::TextStream::Slice v) :
        view{v} {}

    void doRender(kf::tui::TextStream &stream) const override {
        for (rs::size i = 0; i < view.len; i += 1) {
            const rs::u8 b = *(view.data + i);
            stream.print(b, HEX);
        }
    }
};

}// namespace zms
