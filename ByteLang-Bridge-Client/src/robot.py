from threading import Thread
from time import sleep
from typing import Final

from serial import SerialException

from bytelang.core.protocol import Protocol
from bytelang.impl.serializer.bytevector import ByteVectorSerializer
from bytelang.impl.serializer.primitive import u8, f32, u32, u16
from bytelang.impl.serializer.struct_ import StructSerializer
from bytelang.impl.serializer.void import VoidSerializer
from bytelang.impl.stream.serials import SerialStream


class Robot(Protocol):

    def __init__(self) -> None:
        self._serial: Final = SerialStream(self._get_serial_port(), 115200)

        super().__init__(self._serial, self._serial, u8, u8)

        # senders

        self.send_millis_request = self.add_sender(VoidSerializer(), "send_millis_request")
        self.set_manipulator = self.add_sender(StructSerializer((u8, u8)), "set_manipulator")

        # receivers

        self.add_receiver(u32, self._on_millis)
        self.add_receiver(ByteVectorSerializer(u16), self._on_log)

        #

        self._task_completed: bool = True
        self._task_result: int = 0

        self.log("Senders: \n" + "\n".join(map(str, self.get_senders())))
        self.log("Receivers: \n" + "\n".join(map(str, self.get_receivers())))

        self.poll_task = Thread(target=self._poll, daemon=True)

    @staticmethod
    def log(message: str) -> None:
        """Записать лог"""
        print(f"Robot: {message}")

    def start_poll_task(self):
        """Запустить задачу опроса порта"""
        self.poll_task.start()

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
                if "USB" in p
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
