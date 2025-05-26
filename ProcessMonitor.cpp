//
// Created by sazonov99 on 5/26/25.
//

#include "ProcessMonitor.h"

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <csignal>
#include <iomanip>
#include <algorithm>

using namespace std::chrono_literals;

ProcessMonitor::ProcessMonitor() {}

bool ProcessMonitor::parseStatLine(const std::string& line, ProcInfo& info) {
    std::istringstream iss(line);
    std::string tmp;
    int field = 1;

    while (field++ < 14 && iss >> tmp);
    if (!(iss >> info.utime >> info.stime)) return false;

    return true;
}

bool ProcessMonitor::parseStatusForVmRSS(const std::vector<std::string>& lines, ProcInfo& info) {
    for (const auto& line : lines) {
        if (line.rfind("VmRSS:", 0) == 0) {
            std::istringstream mem_iss(line);
            std::string label;
            if (mem_iss >> label >> info.rss_kb) {
                return true;
            }
        }
    }
    return false;
}

bool ProcessMonitor::getProcInfoFromStrings(const std::string& stat_content,
                                            const std::vector<std::string>& status_lines,
                                            ProcInfo& info) {
    if (!parseStatLine(stat_content, info)) return false;
    if (!parseStatusForVmRSS(status_lines, info)) return false;
    return true;
}

bool ProcessMonitor::getProcInfo(pid_t pid, ProcInfo& info) {
    std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
    if (!stat_file.is_open()) return false;
    std::string stat_content((std::istreambuf_iterator<char>(stat_file)), std::istreambuf_iterator<char>());
    stat_file.close();

    std::ifstream status_file("/proc/" + std::to_string(pid) + "/status");
    if (!status_file.is_open()) return false;

    std::vector<std::string> status_lines;
    std::string line;
    while (std::getline(status_file, line)) {
        status_lines.push_back(line);
    }
    status_file.close();

    return getProcInfoFromStrings(stat_content, status_lines, info);
}

std::vector<pid_t> ProcessMonitor::findPidsByName(const std::string& name) {
    std::vector<pid_t> pids;

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;

        const std::string filename = entry.path().filename();
        if (!std::all_of(filename.begin(), filename.end(), ::isdigit)) continue;

        pid_t pid = std::stoi(filename);
        std::ifstream cmdline("/proc/" + filename + "/comm");
        if (!cmdline.is_open()) continue;

        std::string proc_name;
        std::getline(cmdline, proc_name);
        if (proc_name == name) {
            pids.push_back(pid);
        }
    }

    return pids;
}

void ProcessMonitor::monitorSingleProcess(const std::string& proc_name, pid_t pid) {
    static int clk_tck = sysconf(_SC_CLK_TCK);
    ProcInfo prev_info;
    bool first_sample = true;

    while (running.load()) {
        ProcInfo info;
        if (!getProcInfo(pid, info)) {
            std::lock_guard<std::mutex> lock(data_mutex);
            ProcKey key(proc_name, pid);
            auto it = proc_data.find(key);
            if (it == proc_data.end() || it->second.alive) {
                info.alive = false;
                proc_data[key] = info;
                updated.store(true);
                cv.notify_one();
            }
            std::this_thread::sleep_for(1s);
            continue;
        }

        if (!first_sample) {
            double delta = static_cast<double>(
                                   info.utime + info.stime - prev_info.utime - prev_info.stime
                           ) / clk_tck;
            info.cpu_percent = delta * 100.0;
        }

        prev_info = info;
        ProcKey key(proc_name, pid);
        bool notify = false;

        {
            std::lock_guard<std::mutex> lock(data_mutex);
            auto it = proc_data.find(key);
            if (it != proc_data.end() && !first_sample) {
                double d_cpu = std::abs(info.cpu_percent - it->second.cpu_percent);
                long d_rss = std::labs(info.rss_kb - it->second.rss_kb);
                if (d_cpu > 5.0 || d_rss > 5120) {
                    info.alive = true;
                    proc_data[key] = info;
                    notify = true;
                }
            } else if (it == proc_data.end()) {
                info.alive = true;
                proc_data[key] = info;
                notify = true;
            }

            if (notify) updated.store(true);
        }

        if (notify) cv.notify_one();

        first_sample = false;
        std::this_thread::sleep_for(1s);
    }
}

void ProcessMonitor::processWatcher(const std::string& proc_name) {
    while (running.load()) {
        auto current_pids = findPidsByName(proc_name);

        {
            std::lock_guard<std::mutex> lock(threads_mutex);
            for (pid_t pid : current_pids) {
                ProcKey key(proc_name, pid);
                if (monitor_threads.find(key) == monitor_threads.end()) {
                    monitor_threads[key] = std::thread(&ProcessMonitor::monitorSingleProcess, this, proc_name, pid);
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(data_mutex);
            if (current_pids.empty()) {
                missing_processes.insert(proc_name);
            } else {
                missing_processes.erase(proc_name);
                process_missing_displayed[proc_name] = false;
            }
        }

        std::this_thread::sleep_for(1s);
    }
}

void ProcessMonitor::start(const std::set<std::string>& process_names) {
    running.store(true);
    for (const auto& name : process_names) {
        std::thread watcher(&ProcessMonitor::processWatcher, this, name);
        watcher.detach();
    }
}

void ProcessMonitor::stop() {
    running.store(false);
    cv.notify_all();

    std::lock_guard<std::mutex> lock(threads_mutex);
    for (auto& [_, thread] : monitor_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}