//
// Created by sazonov99 on 5/26/25.
//

#ifndef UNTITLED4_UIMANAGER_H
#define UNTITLED4_UIMANAGER_H

class ProcessMonitor;

class UIManager {
public:
    UIManager(ProcessMonitor &monitor);
    void run();

private:
    ProcessMonitor &monitor;
};

#endif // UNTITLED4_UIMANAGER_H
