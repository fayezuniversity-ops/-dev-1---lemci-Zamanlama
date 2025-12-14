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
        p[i].completed = false;
        total_burst_time += p[i].bt;
    }

    sort(p.begin(), p.end(), compareArrival);

    vector<TimeSlice> timeline;
    
    int current_time = 0;
    int completed_count = 0;
    int context_switches = 0;

    for (int i = 0; i < n; ++i) {
        if (p[i].at > current_time) {
            timeline.push_back({"IDLE", current_time, p[i].at});
            current_time = p[i].at;
        }

        if (i > 0) {
            context_switches++;
        }
        
        p[i].ct = current_time + p[i].bt;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        p[i].completed = true;
        completed_count++;

        timeline.push_back({p[i].id, current_time, p[i].ct});
        current_time = p[i].ct;
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