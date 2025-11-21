from threading import Thread
from time import sleep
from typing import Final
from typing import Optional

from serial import SerialException

from bytelang.core.protocol import Protocol
from bytelang.impl.serializer.bytevector import ByteVectorSerializer
from bytelang.impl.serializer.primitive import i16
from bytelang.impl.serializer.primitive import i8
from bytelang.impl.serializer.primitive import u16
from bytelang.impl.serializer.primitive import u32
from bytelang.impl.serializer.primitive import u8
from bytelang.impl.serializer.struct_ import StructSerializer
from bytelang.impl.serializer.void import VoidSerializer
from bytelang.impl.stream.serials import SerialStream


class Robot(Protocol):

    def __init__(self) -> None:
        self._serial: Final = SerialStream(self._get_serial_port(), 115200)

        super().__init__(self._serial, self._serial, u8, u8)

        # senders

        self.send_millis_request = self.add_sender(VoidSerializer(), "send_millis_request")
        self._set_manipulator = self.add_sender(StructSerializer((u8, u8)), "set_manipulator")
        self.send_distances_request = self.add_sender(VoidSerializer(), "send_distances")
        self._set_motors = self.add_sender(StructSerializer((i16, i16)), "set_motors")

        # receivers

        self.add_receiver(u32, self._on_millis)
        self.add_receiver(ByteVectorSerializer(u16), self._on_log)
        self.add_receiver(StructSerializer((u16, u16)), self._on_distances)
        self.add_receiver(StructSerializer((i8, i8)), self._on_encoders)

        #

        self._task_completed: bool = True
        self._task_result: int = 0

        self.log("Senders: \n" + "\n".join(map(str, self.get_senders())))
        self.log("Receivers: \n" + "\n".join(map(str, self.get_receivers())))

        self.poll_task = Thread(target=self._poll, daemon=True)

    def set_motors(self, left: float, right: float) -> None:
        """
        Установить скорости моторов
        :param left: Скорость левого
        :param right: Скорость правого
        """

        def _norm(__v: float) -> int:
            a = 1000
            return min(a, max(-a, int(__v * a)))

        self._set_motors((_norm(left), _norm(right)))

    def _on_encoders(self, v) -> None:
        self.log(v)
        return

    def _on_distances(self, v) -> None:
        self.log(v)
        return

    def control_manipulator(self, /, arm: Optional[float] = None, claw: Optional[float] = None) -> None:
        """
        Управлять манипулятором
        :param arm: Звено [0..1]
        :param claw: Захват [0..1]
        """

        def _normalize(__v: Optional[float], __min: int, __max: int) -> int:
            if __v is None:
                return 0xff

            __v = min(1.0, max(0.0, __v))

            return int(__v * (__max - __min)) + __min

        self._set_manipulator((
            _normalize(arm, 180, 90),
            _normalize(claw, 0, 180),
        ))

    @staticmethod
    def log(message: str) -> None:
        """Записать лог"""
        print(f"Robot: {message}")

    def start_poll_task(self):
        """Запустить задачу опроса порта"""
        self.poll_task.start()

    def reset_buffers(self):
        self._serial.reset()

    def _poll(self) -> None:
        while True:
            try:
                while True:
                    self.poll()
                    sleep(0.001)

            except SerialException as e:
                ports = self._get_serial_port()
                self.log(f"Ошибка соединения: {e}. Подключение к {ports}")
                self._serial.reconnect(ports)

            except KeyboardInterrupt:
                self.log("Завершение работы")
                break

    def _get_serial_port(self) -> str:
        while True:
            ports = tuple(
                p
                for p in SerialStream.search_ports()
                if "USB" in p or "COM" in p and "COM1" not in p
            )

            if ports:
                self.log(f"Обнаружен порты {ports}")
                return ports[0]

            sleep(2)
            self.log("Не найдены порты")

    def _on_millis(self, ms: int) -> None:
        self.log(f"ms: {ms}")

    def _on_log(self, data: bytes) -> None:
        log = data.decode(errors="replace").strip('\n')
        self.log(f"ESP: {log}")
