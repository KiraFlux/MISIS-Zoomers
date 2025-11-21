"""
Точка входа
"""
import time

from robot import Robot


def _launch():
    robot = Robot()
    robot.start_poll_task()

    for i in range(0, 180, 1):
        robot.set_manipulator((0xFF, i))
        time.sleep(0.005)

    for i in range(180, 0, -2):
        robot.set_manipulator((0xFF, i))
        time.sleep(0.01)

    for i in range(180, 90, -1):
        robot.set_manipulator((i, 0xFF))
        time.sleep(0.01)

    for i in range(90, 180, +1):
        robot.set_manipulator((i, 0xFF))
        time.sleep(0.01)

    robot.set_manipulator((0xFF, 0xFF))

    robot.poll_task.join(1.0)


if __name__ == '__main__':
    _launch()
