//
// Created by sazonov99 on 5/26/25.
//

#ifndef UNTITLED4_PROCESSMONITOR_H
#define UNTITLED4_PROCESSMONITOR_H

#pragma once

#include "UIManager.h"
#include "ProcInfo.h"
#include <set>
#include <vector>
#include <mutex>
#include <condition_variable>
#include<map>
#include<atomic>
#include <thread>
#include <cctype>

class ProcessMonitor {
public:
    ProcessMonitor();
    void start(const std::set<std::string>& process_names);
    void stop();
    bool parseStatLine(const std::string& line, ProcInfo& info);
    bool parseStatusForVmRSS(const std::vector<std::string>& lines, ProcInfo& info);
    bool getProcInfoFromStrings(const std::string& stat_content,
                                                const std::vector<std::string>& status_lines,
                                                ProcInfo& info);
    bool getProcInfo(pid_t pid, ProcInfo& info);
    std::vector<pid_t> findPidsByName(const std::string& name);
    const std::map<ProcKey, ProcInfo>& getProcDataForTest() const {
        return proc_data;
    }
    const std::set<std::string>& getMissingProcesses() const {
        return missing_processes;
    }

private:
    void processWatcher(const std::string& proc_name);
    void monitorSingleProcess(const std::string& proc_name, pid_t pid);

    std::mutex data_mutex;
    std::condition_variable cv;
    std::atomic<bool> updated{false};
    std::atomic<bool> running{true};

    std::map<ProcKey, ProcInfo> proc_data;
    std::mutex threads_mutex;
    std::map<ProcKey, std::thread> monitor_threads;
    std::set<std::string> missing_processes;
    std::map<std::string, bool> process_missing_displayed;

    friend class UIManager;
};


#endif //UNTITLED4_PROCESSMONITOR_H
