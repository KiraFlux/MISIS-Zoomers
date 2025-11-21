from dataclasses import dataclass
from time import sleep
from typing import Final
from typing import Optional
from typing import Sequence

import serial.tools.list_ports
from serial import Serial as SerialPort

from bytelang.abc.stream import InputStream
from bytelang.abc.stream import OutputStream


@dataclass
class SerialStream(InputStream, OutputStream):
    """Потом ввода-вывода по последовательному порту"""

    def __init__(self, port: str, baud: int) -> None:
        self.port: Final = port
        self._baud = baud

        self._serial_port = SerialPort(port=self.port, baudrate=self._baud, timeout=0.1)
        self._connected = True

    def read(self, size: int) -> bytes:
        if not self._connected:
            raise serial.SerialException("Соединение закрыто")

        try:
            return self._serial_port.read(size)

        except serial.SerialException:
            self._connected = False
            raise

    def write(self, data: bytes) -> None:
        if not self._connected:
            raise serial.SerialException("Соединение закрыто")

        try:
            self._serial_port.write(data)

        except serial.SerialException:
            self._connected = False
            raise

    def close(self):
        """Закрыть подключение"""
        if self._connected:
            self._serial_port.close()
            self._connected = False

    def reconnect(self, port: Optional[str] = None):
        """Попытка переподключитья"""
        self.close()
        sleep(2)
        self._serial_port = SerialPort(port=port or self.port, baudrate=self._baud, timeout=None)
        self._connected = True

        self.reset()

        self._serial_port.read(self._serial_port.in_waiting)

    def reset(self):
        self._serial_port.reset_input_buffer()
        self._serial_port.reset_output_buffer()

    @staticmethod
    def search_ports(exclude: Sequence[str] = ("COM1",)) -> Sequence[str]:
        """Находит порты"""

        return tuple(
            port.device for port in serial.tools.list_ports.comports()
            if port.device not in exclude
        )

    def __str__(self) -> str:
        return f"{self.__class__.__name__}<{self._serial_port.port}>"
