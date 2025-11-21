import time

from robot import Robot


def _launch():
    robot = Robot()
    robot.start_poll_task()

    time.sleep(1)
    robot.send_millis_request(None)
    time.sleep(1)

    for i in range(10):
        robot.control_manipulator(0.0, 1.0)
        time.sleep(1)

        robot.control_manipulator(1.0, 0.0)
        time.sleep(1)

    robot.control_manipulator(None, None)

    time.sleep(2)
    print("Done")


if __name__ == '__main__':
    _launch()
