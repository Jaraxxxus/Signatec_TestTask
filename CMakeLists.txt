cmake_minimum_required(VERSION 3.22)
project(TestTask)

set(CMAKE_CXX_STANDARD 17)

# Заголовки
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Основная программа
add_executable(TaskManager
        main.cpp
        ProcessMonitor.cpp
        UIManager.cpp
)

# Тесты (отдельная цель!)
add_executable(process_monitor_tests
        test/test.cpp
        ProcessMonitor.cpp
        UIManager.cpp
)
