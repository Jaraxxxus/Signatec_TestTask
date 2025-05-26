#include "ProcessMonitor.h"
#include "UIManager.h"
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <vector>

UIManager::UIManager(ProcessMonitor &monitor) : monitor(monitor) {}

void UIManager::run() {
    while (monitor.running.load()) {
        std::unique_lock<std::mutex> lock(monitor.data_mutex);
        monitor.cv.wait(lock, [&] {
            return monitor.updated.load() || !monitor.running.load();
        });

        if (!monitor.running.load()) {
            break;
        }

        if (!monitor.updated.exchange(false)) {
            continue;
        }

        std::cout << "\033[2J\033[H";

        if (monitor.proc_data.empty()) {
            std::cout << "Нет отслеживаемых процессов.\n";
        }

        std::set<std::string> alive_names;
        std::set<ProcKey> to_remove;

        for (const auto &[key, info] : monitor.proc_data) {
            const auto &[name, pid] = key;

            if (!info.alive) {
                bool other_alive = false;
                for (const auto &[other_key, other_info] : monitor.proc_data) {
                    if (other_key.first == name && other_key != key && other_info.alive) {
                        other_alive = true;
                        break;
                    }
                }

                if (other_alive) {
                    to_remove.insert(key);
                } else {
                    std::cout << "[PID " << pid << "] " << name << " завершён\n";
                }
            } else {
                alive_names.insert(name);
                std::cout << "[PID " << pid << "] " << name << " CPU: " << std::fixed
                          << std::setprecision(2) << info.cpu_percent << "%"
                          << " RSS: " << (info.rss_kb / 1024) << " MB\n";
            }
        }

        for (const auto &key : to_remove) {
            monitor.proc_data.erase(key);
        }

        for (const auto &name : monitor.missing_processes) {
            if (alive_names.count(name) == 0 &&
                !monitor.process_missing_displayed[name]) {
                std::cout << "Процесс с именем \"" << name << "\" не найден.\n";
                monitor.process_missing_displayed[name] = true;
            }
        }
    }
}
