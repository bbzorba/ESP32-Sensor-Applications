# ESP32 RTOS Projects

This folder contains multiple ESP32 projects demonstrating the use of FreeRTOS for real-time multitasking and hardware control. Each subproject showcases different aspects of RTOS-based development, including:

- Multithreading and task management
- Synchronization (mutexes, queues, notifications)
- Sensor integration and hardware abstraction
- Real-time data processing

## FreeRTOS Usage

All projects in this folder utilize FreeRTOS, the real-time operating system included with ESP-IDF. FreeRTOS provides:

- Lightweight task scheduling for concurrent execution
- Inter-task communication via queues, semaphores, and notifications
- Deterministic timing and delays for hardware interaction
- Support for multicore ESP32 chips

We use FreeRTOS to structure our applications as independent tasks, each responsible for a specific function (e.g., sensor reading, button handling, data logging). This approach improves reliability, responsiveness, and modularity in embedded systems.

## Getting Started

1. Select a subproject (e.g., `basic_RTOS_multithreading`, `RTOS_multithreading_with_mutex`).
2. Follow the README in that subproject for build and usage instructions.
3. All code is written for ESP-IDF and tested on ESP32 boards.

## References

- [FreeRTOS Documentation](https://www.freertos.org/)
- [ESP-IDF FreeRTOS API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html)

---
For questions or contributions, open an issue or contact the project maintainer.
