//
// Created by sazonov99 on 5/26/25.
//


#include "../ProcessMonitor.h"
#include <cassert>
#include <iostream>

void test_parseStatLine() {
    ProcessMonitor monitor;
    ProcInfo info;
    std::string line = "1234 (name) S 1 2 3 4 5 6 7 8 9 1 99 42";

    bool ok = monitor.parseStatLine(line, info);
    assert(ok);


    std::cout << "info.utime = " << info.utime << std::endl;
    std::cout << "info.stime = " << info.stime << std::endl;

    assert(info.utime == 99);
    assert(info.stime == 42);

    std::cout << "test_parseStatLine passed\n";
}

void test_getProcInfo() {
    ProcessMonitor monitor;
    ProcInfo info;

    bool result = monitor.getProcInfo(9999999, info);
    assert(!result);

    std::cout << "test_getProcInfo passed\n";
}

void test_parseVmRss() {
    std::vector<std::string> status_lines = {
            "Name:\tmyproc",
            "VmRSS:\t2048 kB",
            "State:\tR (running)"
    };
    ProcInfo info;
    assert(ProcessMonitor().parseStatusForVmRSS(status_lines, info));
    assert(info.rss_kb == 2048);
    std::cout << "test_parseVmRss passed\n";

}

void test_getProcInfoFromStrings() {
    std::string stat_line = "1234 (myproc) S 1 2 3 4 5 6 7 8 9 10 11 12";
    std::vector<std::string> status_lines = {
            "Name:\tmyproc",
            "VmRSS:\t3000 kB"
    };
    ProcInfo info;
    assert(ProcessMonitor().getProcInfoFromStrings(stat_line, status_lines, info));
    assert(info.utime == 11);
    assert(info.stime == 12);
    assert(info.rss_kb == 3000);
    std::cout << "test_getProcInfoFromStrings passed\n";

}

void test_findPidsByName() {
    auto pids = ProcessMonitor().findPidsByName("bash");
    assert(!pids.empty());
    std::cout << "test_findPidsByName passed\n";

}

void test_monitorKnownProcess() {
    std::string name = "bash";
    ProcessMonitor monitor;

    monitor.start({name});
    std::this_thread::sleep_for(std::chrono::seconds(3));

    bool found = false;
    for (const auto& [key, info] : monitor.getProcDataForTest()) {
        if (key.first == name && info.alive) {
            found = true;
            break;
        }
    }

    monitor.stop();
    assert(found);
    std::cout << "test_monitorKnownProcess passed\n";
}

void test_nonexistentProcessIsMissing() {
    ProcessMonitor monitor;

    std::set<std::string> names = {"definitelynotexist"};
    monitor.start(names);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto missing = monitor.getMissingProcesses();
    assert(missing.find("definitelynotexist") != missing.end());

    monitor.stop();
    std::cout << "test_nonexistentProcessIsMissing passed\n";
}

void test_startWithEmptyProcessList() {
    ProcessMonitor monitor;

    std::set<std::string> empty;
    monitor.start(empty);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    assert(monitor.getMissingProcesses().empty());
    assert(monitor.getProcDataForTest().empty());

    monitor.stop();
    std::cout << "test_startWithEmptyProcessList passed\n";
}

void test_all() {
    test_parseStatLine();
    test_parseVmRss();
    test_nonexistentProcessIsMissing();
    test_startWithEmptyProcessList();
    test_getProcInfoFromStrings();
    test_findPidsByName();
    test_monitorKnownProcess();
    std::cout << "All tests passed.\n";
}



int main() {
    test_all();
    return 0;
}