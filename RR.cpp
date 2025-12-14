#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <cmath>

using namespace std;

struct Process {
    string id;
    int at;
    int bt;
    int remaining_bt;
    int ct;
    int tat;
    int wt;
    bool completed;
};

struct TimeSlice {
    string process_id;
    int start_time;
    int end_time;
};

bool compareArrival(const Process &a, const Process &b) {
    if (a.at != b.at)
        return a.at < b.at;
    return a.id < b.id;
}

int main() {
    int n, tq;
    
    cout << "Enter Number of Processes: ";
    cin >> n;
    
    cout << "Enter Time Quantum: ";
    cin >> tq;

    vector<Process> p(n);
    int total_burst_time = 0;

    for(int i = 0; i < n; i++) {
        cout << "Enter Process ID (e.g. P001): ";
        cin >> p[i].id;
        cout << "Enter Arrival Time: ";
        cin >> p[i].at;
        cout << "Enter Burst Time: ";
        cin >> p[i].bt;
        p[i].remaining_bt = p[i].bt;
        p[i].completed = false;
        total_burst_time += p[i].bt;
    }

    sort(p.begin(), p.end(), compareArrival);

    vector<TimeSlice> timeline;
    queue<int> ready_queue;
    vector<bool> in_queue(n, false);
    
    int current_time = 0;
    int completed_count = 0;
    int context_switches = 0;

    int idx = 0; 
    while(idx < n && p[idx].at <= current_time) {
        ready_queue.push(idx);
        in_queue[idx] = true;
        idx++;
    }

    if(ready_queue.empty() && idx < n) {
        if(current_time < p[idx].at) {
             timeline.push_back({"IDLE", current_time, p[idx].at});
             current_time = p[idx].at;
        }
        while(idx < n && p[idx].at <= current_time) {
            ready_queue.push(idx);
            in_queue[idx] = true;
            idx++;
        }
    }

    while(completed_count < n) {
        if(ready_queue.empty()) {
            int next_arrival_time = 999999;
            for(int i = 0; i < n; i++) {
                if(!p[i].completed && !in_queue[i]) {
                    if(p[i].at < next_arrival_time) {
                        next_arrival_time = p[i].at;
                    }
                }
            }
            
            if(next_arrival_time != 999999) {
                timeline.push_back({"IDLE", current_time, next_arrival_time});
                current_time = next_arrival_time;
                while(idx < n && p[idx].at <= current_time) {
                    ready_queue.push(idx);
                    in_queue[idx] = true;
                    idx++;
                }
            } else {
                break;
            }
        }

        if(ready_queue.empty()) continue;

        int current_p_idx = ready_queue.front();
        ready_queue.pop();
        context_switches++;

        int exec_time = min(tq, p[current_p_idx].remaining_bt);
        
        timeline.push_back({p[current_p_idx].id, current_time, current_time + exec_time});
        
        p[current_p_idx].remaining_bt -= exec_time;
        current_time += exec_time;

        while(idx < n && p[idx].at <= current_time) {
            if(!in_queue[idx]) {
                ready_queue.push(idx);
                in_queue[idx] = true;
            }
            idx++;
        }

        if(p[current_p_idx].remaining_bt > 0) {
            ready_queue.push(current_p_idx);
        } else {
            p[current_p_idx].completed = true;
            p[current_p_idx].ct = current_time;
            p[current_p_idx].tat = p[current_p_idx].ct - p[current_p_idx].at;
            p[current_p_idx].wt = p[current_p_idx].tat - p[current_p_idx].bt;
            completed_count++;
        }
    }

    cout << "\nTime Table [Gantt Chart]:" << endl;
    for(const auto& slice : timeline) {
        cout << "[ " << slice.start_time << " ] - - " << slice.process_id << " - - [ " << slice.end_time << " ]" << endl;
    }
    
    double total_wt = 0;
    double total_tat = 0;
    int max_wt = -1;
    int max_tat = -1;

    for(int i = 0; i < n; i++) {
        total_wt += p[i].wt;
        total_tat += p[i].tat;
        if(p[i].wt > max_wt) max_wt = p[i].wt;
        if(p[i].tat > max_tat) max_tat = p[i].tat;
    }

    cout << "\nMaximum Waiting Time: " << max_wt << endl;
    cout << "Average Waiting Time: " << (total_wt / n) << endl;
    
    cout << "Maximum Turnaround Time: " << max_tat << endl;
    cout << "Average Turnaround Time: " << (total_tat / n) << endl;

    cout << "\nThroughput for T=[50, 100, 150, 200]:" << endl;
    vector<int> times = {50, 100, 150, 200};
    for(int t : times) {
        int count = 0;
        for(int i = 0; i < n; i++) {
            if(p[i].ct <= t) count++;
        }
        cout << "T = " << t << ": " << count << " Processes Completed" << endl;
    }

    double cs_overhead = context_switches * 0.001;
    double cpu_eff = (double)total_burst_time / (total_burst_time + cs_overhead) * 100.0;

    cout << "\nTotal Context Switches: " << context_switches << endl;
    cout << "Average CPU Efficiency: " << fixed << setprecision(4) << cpu_eff << "%" << endl;

    return 0;
}