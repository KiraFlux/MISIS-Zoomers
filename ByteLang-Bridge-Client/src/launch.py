"""
Точка входа
"""
import time

from robot import Robot


def _launch():
    robot = Robot()
    robot.start_poll_task()

    time.sleep(2)

    for i in range(-100, 101, 4):
        p = 1 - abs(i) / 100
        print(p)

        robot.set_motors(p, -p)
        time.sleep(0.02)

    for i in range(-100, 101, 2):
        p = 1 - abs(i) / 100
        print(p)

        robot.set_motors(-p, p)
        time.sleep(0.02)

    for i in range(-100, 101, 4):
        p = 1 - abs(i) / 100
        print(p)

        robot.set_motors(p, -p)
        time.sleep(0.02)

    robot.set_motors(0, 0)

    robot.poll_task.join(1)


if __name__ == '__main__':
    _launch()
