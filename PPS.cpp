#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

using namespace std;

struct Process {
    string id;
    int at;
    int bt;
    int priority; 
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

int findHighestPriorityProcess(vector<Process>& processes, int currentTime, const vector<bool>& process_available) {
    int highest_priority = numeric_limits<int>::max();
    int best_process_index = -1;

    for (size_t i = 0; i < processes.size(); ++i) {
        if (!processes[i].completed && processes[i].at <= currentTime) {
            if (processes[i].priority < highest_priority) { 
                highest_priority = processes[i].priority;
                best_process_index = i;
            } else if (processes[i].priority == highest_priority) {
                if (processes[i].at < processes[best_process_index].at) {
                    best_process_index = i; 
                }
            }
        }
    }
    return best_process_index;
}

int main() {
    int n;
    
    cout << "Enter Number of Processes: ";
    cin >> n;

    vector<Process> p(n);
    int total_burst_time = 0;

    for(int i = 0; i < n; i++) {
        cout << "Enter Process ID (e.g. P001): ";
        cin >> p[i].id;
        cout << "Enter Arrival Time: ";
        cin >> p[i].at;
        cout << "Enter Burst Time: ";
        cin >> p[i].bt;
        cout << "Enter Priority (Lower number = Higher priority): ";
        cin >> p[i].priority;
        p[i].remaining_bt = p[i].bt;
        p[i].completed = false;
        total_burst_time += p[i].bt;
    }

    sort(p.begin(), p.end(), compareArrival);

    vector<TimeSlice> timeline;
    vector<bool> process_available(n, true);
    
    int current_time = 0;
    int completed_count = 0;
    int context_switches = 0;
    int previous_process_index = -1; 
    int total_time = 0;
    for(const auto& proc : p) total_time = max(total_time, proc.at + proc.bt); // Initial rough max time

    while(completed_count < n) {
        int current_p_idx = findHighestPriorityProcess(p, current_time, process_available);

        if (current_p_idx == -1) {
            int next_arrival_time = numeric_limits<int>::max();
            for(const auto& proc : p) {
                if(!proc.completed && proc.at > current_time) {
                    next_arrival_time = min(next_arrival_time, proc.at);
                }
            }
            
            if(next_arrival_time != numeric_limits<int>::max()) {
                if(timeline.empty() || timeline.back().process_id != "IDLE" || timeline.back().end_time != current_time) {
                    timeline.push_back({"IDLE", current_time, next_arrival_time});
                } else {
                    timeline.back().end_time = next_arrival_time;
                }
                current_time = next_arrival_time;
            } else {
                break; 
            }
        } else {
            if (current_p_idx != previous_process_index && previous_process_index != -1 && p[previous_process_index].remaining_bt > 0) {
                 context_switches++;
            }
            
            int start_time = current_time;
            
            int next_event_time = numeric_limits<int>::max();
            
            for(int i = 0; i < n; i++) {
                if(!p[i].completed && p[i].at > current_time) {
                    next_event_time = min(next_event_time, p[i].at);
                }
            }

            int time_to_run = min(p[current_p_idx].remaining_bt, next_event_time - current_time);
            
            p[current_p_idx].remaining_bt -= time_to_run;
            current_time += time_to_run;

            if (!timeline.empty() && timeline.back().process_id == p[current_p_idx].id && timeline.back().end_time == start_time) {
                timeline.back().end_time = current_time;
            } else {
                timeline.push_back({p[current_p_idx].id, start_time, current_time});
            }

            if(p[current_p_idx].remaining_bt == 0) {
                p[current_p_idx].completed = true;
                p[current_p_idx].ct = current_time;
                p[current_p_idx].tat = p[current_p_idx].ct - p[current_p_idx].at;
                p[current_p_idx].wt = p[current_p_idx].tat - p[current_p_idx].bt;
                completed_count++;
            }

            previous_process_index = current_p_idx;
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
    cout << "Average Waiting Time: " << fixed << setprecision(2) << (total_wt / n) << endl;
    
    cout << "Maximum Turnaround Time: " << max_tat << endl;
    cout << "Average Turnaround Time: " << fixed << setprecision(2) << (total_tat / n) << endl;

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
    double total_time_elapsed = timeline.empty() ? 0 : timeline.back().end_time;
    double cpu_eff;

    if (total_time_elapsed > 0) {
        cpu_eff = (double)total_burst_time / (total_time_elapsed) * 100.0;
    } else {
        cpu_eff = 0.0;
    }


    cout << "\nTotal Context Switches: " << context_switches << endl;
    cout << "Average CPU Efficiency: " << fixed << setprecision(4) << cpu_eff << "%" << endl;

    return 0;
}