import time

from robot import Robot


def _launch():
    robot = Robot()
    robot.start_poll_task()

    # Даем время на синхронизацию
    print("Waiting for sync...")
    time.sleep(3)

    print("Sending millis requests...")
    for i in range(5):
        robot.send_millis_request(None)
        time.sleep(1)

    # Ждем ответы
    time.sleep(2)
    robot.control_manipulator(None, None)

    print("Test completed")

    robot.poll_task.join()


if __name__ == '__main__':
    _launch()
