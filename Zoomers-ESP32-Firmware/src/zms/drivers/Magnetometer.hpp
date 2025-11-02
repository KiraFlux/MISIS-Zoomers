//#pragma onde
//
//#include <QMC5883LCompass.h>
//
//#include "zms/aliases.hpp"
//
//namespace zms {
//
//struct Magnetomener {
//
//private:
//    QMC5883LCompass compass;
//
//public:
//    explicit Magnetomener() {}
//
//    void init() {
//        compass.init();
//    }
//
//    rs::f32 read() {
//        compass.read();
//        int x = compass.getX();
//        int y = compass.getY();
//        int z = compass.getZ();
//
//        return std::hypot(x * x, y * y, z * z);
//    }
//};
//
//}// namespace zms
