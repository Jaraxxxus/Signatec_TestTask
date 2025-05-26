//
// Created by sazonov99 on 5/26/25.
//

#ifndef UNTITLED4_PROCINFO_H
#define UNTITLED4_PROCINFO_H

#include <utility>
#include <string>

struct ProcInfo {
    long utime = 0;
    long stime = 0;
    long rss_kb = 0;
    double cpu_percent = 0.0;
    bool alive = true;
};

using ProcKey = std::pair<std::string, pid_t>;




#endif //UNTITLED4_PROCINFO_H
