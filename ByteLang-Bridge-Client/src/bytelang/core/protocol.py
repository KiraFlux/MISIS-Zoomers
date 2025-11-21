from typing import Any
from typing import Callable
from typing import Final
from typing import Iterable
from typing import TypeVar

from bytelang.abc.serializer import Serializable
from bytelang.abc.serializer import Serializer
from bytelang.abc.stream import InputStream
from bytelang.abc.stream import OutputStream
from bytelang.core.instruction import Instruction
from bytelang.impl.serializer.primitive import PrimitiveSerializer

_T = TypeVar("_T", bound=Serializable)


class Protocol:
    """Протокол P2P общения по потоку с указанной структурой полей"""

    def __init__(
            self,
            input_stream: InputStream,
            output_stream: OutputStream,
            local_code: PrimitiveSerializer[int],
            remote_code: PrimitiveSerializer[int]
    ) -> None:
        self._input_stream: Final = input_stream
        self._output_stream: Final = output_stream

        self._local_instruction_code: Final = local_code
        self._remote_instruction_code: Final = remote_code

        self._receive_handlers: Final = dict[bytes, tuple[Instruction, Callable[[Any], None]]]()
        self._send_handlers: Final = dict[bytes, Instruction]()

    def get_senders(self) -> Iterable[Instruction]:
        """Получить все обработчики на отправку"""
        return self._send_handlers.values()

    def get_receivers(self) -> Iterable[Instruction]:
        """Получить все обработчики на приём"""
        return (i for i, j in self._receive_handlers.values())

    def add_receiver(
            self, /,
            result: Serializer[_T],
            handler: Callable[[_T], None],
            name: str = None
    ) -> None:
        """Зарегистрировать обработчик входящих сообщений"""
        index = len(self._receive_handlers)
        code = self._local_instruction_code.pack(index)
        instruction = Instruction(code, result, name)
        self._receive_handlers[code] = (instruction, handler)

    def add_sender(self, /, signature: Serializer[_T], name: str = None) -> Callable[[_T], None]:
        """Зарегистрировать исходящую инструкцию"""
        index = len(self._send_handlers)
        code = self._remote_instruction_code.pack(index)
        instruction = Instruction(code, signature, name)
        self._send_handlers[code] = instruction

        def _wrapper(value: _T):
            return instruction.send(self._output_stream, value)

        return _wrapper

    def poll(self) -> None:
        """Обработать входящее сообщение"""
        code_size = self._remote_instruction_code.size
        code = self._input_stream.read(code_size)

        if not code or len(code) < code_size:
            return

        if code not in self._receive_handlers:
            return

        instruction, handler = self._receive_handlers[code]
        args_result = instruction.receive(self._input_stream)
        handler(args_result)
