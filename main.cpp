#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <csignal>
#include "ProcessMonitor.h"
#include "UIManager.h"

ProcessMonitor global_monitor;

void handle_signal(int) {
    global_monitor.stop();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./TaskManager process_name1 [process_name2 ...]\n";
        return 1;
    }
    std::signal(SIGINT, handle_signal);
    std::set<std::string> process_names;
    for (int i = 1; i < argc; ++i) {
        process_names.insert(argv[i]);
    }
    global_monitor.start(process_names);
    UIManager ui(global_monitor);
    ui.run();
    return 0;
}
