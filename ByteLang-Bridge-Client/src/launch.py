"""
Точка входа
"""
import time

from robot import Robot


def _launch():
    robot = Robot()
    robot.start_poll_task()

    time.sleep(1)

    for i in range(1):

        robot.control_manipulator(0, 1)
        time.sleep(1)

        robot.control_manipulator(0, 0)
        time.sleep(1)

        robot.control_manipulator(1, 0)
        time.sleep(1)

        robot.control_manipulator(0, 0)
        time.sleep(1)

    robot.control_manipulator(None, None)

    robot.poll_task.join(1.0)


if __name__ == '__main__':
    _launch()
