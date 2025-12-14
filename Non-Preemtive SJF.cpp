#include <iostream>
#include <vector>
#include <string>


#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <queue>

using namespace std;

struct Process {
    string id;
    int arrivalTime;
    int burstTime;
    string priority;
    int remainingTime;
    double startTime;
    double finishTime;
    double waitingTime;
    double turnaroundTime;
};

struct CompareBurstTime {
    bool operator()(const Process& a, const Process& b) {
        if (a.burstTime != b.burstTime) {
            return a.burstTime > b.burstTime; 
        }
        return a.arrivalTime > b.arrivalTime;
    }
};

bool compareArrival(const Process& a, const Process& b) {
    return a.arrivalTime < b.arrivalTime;
}

vector<Process> readProcessesFromFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);
    string line;

    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string segment;
        vector<string> seglist;

        while (getline(ss, segment, ',')) {
            seglist.push_back(segment);
        }

        if (seglist.size() == 4) {
            Process p;
            p.id = seglist[0];
            p.arrivalTime = stoi(seglist[1]);
            p.burstTime = stoi(seglist[2]);
            p.priority = seglist[3];
            p.remainingTime = p.burstTime;
            p.startTime = -1;
            p.finishTime = -1;
            p.waitingTime = 0;
            p.turnaroundTime = 0;
            processes.push_back(p);
        }
    }
    return processes;
}

void runNonPreemptiveSJF(vector<Process>& allProcesses, const double contextSwitchTime, const vector<int>& throughputTimes) {
    sort(allProcesses.begin(), allProcesses.end(), compareArrival);
    
    priority_queue<Process, vector<Process>, CompareBurstTime> readyQueue;
    
    vector<Process> completedProcesses;
    
    vector<pair<double, string>> ganttChart;
    double currentTime = 0;
    int processIndex = 0;
    int completedCount = 0;
    int totalContextSwitches = 0;
    double totalServiceTime = 0;
    string currentProcessId = "NONE";

    while (completedCount < allProcesses.size()) {
        while (processIndex < allProcesses.size() && allProcesses[processIndex].arrivalTime <= currentTime) {
            readyQueue.push(allProcesses[processIndex]);
            processIndex++;
        }

        if (readyQueue.empty()) {
            if (currentProcessId != "IDLE") {
                if (currentProcessId != "NONE") {
                    
                }
                ganttChart.push_back({currentTime, "IDLE"});
                currentProcessId = "IDLE";
            }
            
            if (processIndex < allProcesses.size()) {
                currentTime = allProcesses[processIndex].arrivalTime;
            } else {
                break; 
            }
            continue;
        }

        Process currentProcess = readyQueue.top();
        readyQueue.pop();
        
        if (currentProcessId == "IDLE") {
            ganttChart.push_back({currentTime, "IDLE"});
        }
        
       
        if (completedCount > 0 || (completedCount == 0 && currentProcessId != "IDLE")) { 
            currentTime += contextSwitchTime;
            totalContextSwitches++;
        }

        currentProcess.startTime = currentTime;

        ganttChart.push_back({currentTime, currentProcess.id});
        currentProcessId = currentProcess.id;
        
        currentTime += currentProcess.burstTime;
        currentProcess.finishTime = currentTime;
        
        currentProcess.turnaroundTime = currentProcess.finishTime - currentProcess.arrivalTime;
        currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
        totalServiceTime += currentProcess.burstTime;

        ganttChart.push_back({currentTime, currentProcess.id});

        completedProcesses.push_back(currentProcess);
        completedCount++;
        currentProcessId = "NONE";
    
    double finalTime = currentTime;

    cout << "\n--- Gantt Chart (Time Table) ---" << endl;
    for (size_t i = 0; i < ganttChart.size(); i += 2) {
        if (i + 1 < ganttChart.size()) {
            string id = ganttChart[i+1].second;
            if (id == "IDLE") {
                 cout << "[ " << fixed << setprecision(3) << ganttChart[i].first << " ] - - IDLE - - [ " << fixed << setprecision(3) << ganttChart[i+1].first << " ]" << endl;
            } else {
                 cout << "[ " << fixed << setprecision(3) << ganttChart[i].first << " ] - - " << id << " - - [ " << fixed << setprecision(3) << ganttChart[i+1].first << " ]" << endl;
            }
        }
    }

    double totalWaitingTime = 0;
    double maxWaitingTime = 0;
    for (const auto& p : completedProcesses) {
        totalWaitingTime += p.waitingTime;
        if (p.waitingTime > maxWaitingTime) {
            maxWaitingTime = p.waitingTime;
        }
    }
    double averageWaitingTime = totalWaitingTime / completedProcesses.size();

    cout << "\n--- Waiting Time ---" << endl;
    cout << "Maximum Waiting Time: " << fixed << setprecision(3) << maxWaitingTime << endl;
    cout << "Average Waiting Time: " << fixed << setprecision(3) << averageWaitingTime << endl;

    double totalTurnaroundTime = 0;
    double maxTurnaroundTime = 0;
    for (const auto& p : completedProcesses) {
        totalTurnaroundTime += p.turnaroundTime;
        if (p.turnaroundTime > maxTurnaroundTime) {
            maxTurnaroundTime = p.turnaroundTime;
        }
    }
    double averageTurnaroundTime = totalTurnaroundTime / completedProcesses.size();

    cout << "\n--- Turnaround Time ---" << endl;
    cout << "Maximum Turnaround Time: " << fixed << setprecision(3) << maxTurnaroundTime << endl;
    cout << "Average Turnaround Time: " << fixed << setprecision(3) << averageTurnaroundTime << endl;

    cout << "\n--- Throughput (Jobs Completed) ---" << endl;
    for (int T : throughputTimes) {
        int completedAtT = 0;
        for (const auto& p : completedProcesses) {
            if (p.finishTime <= T) {
                completedAtT++;
            }
        }
        cout << "Throughput at T=" << T << ": " << completedAtT << endl;
    }

    double totalContextSwitchOverhead = totalContextSwitches * contextSwitchTime;
    double totalIdleTime = 0;
    
    for (size_t i = 0; i < ganttChart.size(); i += 2) {
        if (i + 1 < ganttChart.size() && ganttChart[i+1].second == "IDLE") {
             totalIdleTime += (ganttChart[i+1].first - ganttChart[i].first);
        }
    }
    
    double cpuUtilization = (totalServiceTime / finalTime) * 100.0;
    
    cout << "\n--- Average CPU Utilization ---" << endl;
    cout << "Total Simulation Time: " << fixed << setprecision(3) << finalTime << endl;
    cout << "Total Service Time (Burst Time Sum): " << fixed << setprecision(3) << totalServiceTime << endl;
    cout << "Total Context Switch Overhead: " << fixed << setprecision(3) << totalContextSwitchOverhead << endl;
    cout << "Total Idle Time: " << fixed << setprecision(3) << totalIdleTime << endl;
    cout << "Average CPU Utilization: " << fixed << setprecision(3) << cpuUtilization << "%" << endl;

    cout << "\n--- Total Context Switches ---" << endl;
    cout << "Total Number of Context Switches: " << totalContextSwitches << endl;
}

int main() {
    const double CONTEXT_SWITCH_TIME = 0.001;
    const vector<int> THROUGHPUT_TIMES = {50, 100, 150, 200};

    string filename1 = "odev1_case1.txt";
    cout << "===================================================" << endl;
    cout << "--- Non-Preemptive SJF Scheduling for " << filename1 << " ---" << endl;
    vector<Process> processes1 = readProcessesFromFile(filename1);
    if (!processes1.empty()) {
        runNonPreemptiveSJF(processes1, CONTEXT_SWITCH_TIME, THROUGHPUT_TIMES);
    } else {
        cout << "Error: Could not read process data from " << filename1 << endl;
    }

    string filename2 = "odev1_case2.txt";
    cout << "\n-----------------------------------------------------" << endl;
    cout << "--- Non-Preemptive SJF Scheduling for " << filename2 << " ---" << endl;
    vector<Process> processes2 = readProcessesFromFile(filename2);
    if (!processes2.empty()) {
        runNonPreemptiveSJF(processes2, CONTEXT_SWITCH_TIME, THROUGHPUT_TIMES);
    } else {
        cout << "Error: Could not read process data from " << filename2 << endl;
    }

    return 0;
}