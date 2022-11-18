#include <iostream>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <functional>
#include <math.h>
#include <vector>
#include <queue>
#include <bits/stdc++.h>
#include <ctime>

using namespace std;

#define ll long long
typedef pair<int, int> P;

const int maxn = 1e4 + 10;
const int INF = 1e9 + 10;

clock_t clk;

double get_time() {
    return (double)(clock() - clk) / CLOCKS_PER_SEC;
}

int up(int x, int y) {
    return (x + y - 1) / y;
}

int task_n, mach_n, disk_n;
int egde_n[2];
int best_ans = INF;

int last_ans = -1;

double avg_disk_speed, avg_mach_power;

double disk_k, mach_k;

struct Task {
    int siz;
    int data;
    int k;
    vector<int> affi;
    vector<int> g[2], rg[2];

    int a[4]; // abcd

    int next_max_time = 0;

    int esti_cost_disk_time = 0;
    int esti_cost_mach_time = 0;

    int mach, disk;
    int best_mach, best_disk, best_begin;
} task[maxn];

struct Mach {
    int power;
    int time;
} mach[maxn];

struct Disk {
    int cap;
    int speed;
    int use;
} disk[maxn];

void read_in() {
    scanf("%d", &task_n);
    int j;
    for (int i = 1; i <= task_n; ++i) {
        scanf("%d", &j);
        scanf("%d%d", &task[j].siz, &task[j].data);
        int k, tmp;
        scanf("%d", &task[j].k);
        for(int _ = 0; _ < task[j].k; _++) {
            scanf("%d", &tmp);
            task[j].affi.emplace_back(tmp);
        }
    }
    scanf("%d", &mach_n);
    for (int i = 1; i <= mach_n; ++i) {
        scanf("%d", &j);
        scanf("%d", &mach[j].power);
    }
    scanf("%d", &disk_n);
    for (int i = 1; i <= disk_n; ++i) {
        scanf("%d", &j);
        scanf("%d%d", &disk[j].speed, &disk[j].cap);
    }
    for (int type = 0; type < 2; ++type) {
        scanf("%d", &egde_n[type]);
        int u, v;
        for (int i = 0; i < egde_n[type]; ++i) {
            scanf("%d%d", &u, &v);
            task[u].g[type].emplace_back(v);
            task[v].rg[type].emplace_back(u);
        }
    }
}

int in[maxn];
int tp_seq[maxn];
void init() {
    srand(42);
    clk = clock();

    for (int i = 1; i <= disk_n; ++i) {
        avg_disk_speed += disk[i].speed;
    }
    avg_disk_speed /= (double)disk_n;
    
    for (int i = 1; i <= mach_n; ++i) {
        avg_mach_power += mach[i].power;
    }
    avg_mach_power /= (double)mach_n;

    for (int i = 1; i <= task_n; ++i) {
        task[i].esti_cost_disk_time += (int) ((double)task[i].data / avg_disk_speed);
        task[i].esti_cost_mach_time += (int) ((double)task[i].siz / avg_mach_power);
    }

    for (int i = 1; i <= task_n; ++i) {
        for (auto j : task[i].rg[0]) {
            task[i].esti_cost_disk_time += (int) ((double)task[j].data / avg_disk_speed);
        }
    }

    for (int i = 1; i <= task_n; ++i) in[i] = 0;
    queue<int> que;
    for (int i = 1; i <= task_n; ++i) {
        for (int tp = 0; tp < 2; ++tp)
            for (auto j : task[i].rg[tp])
                ++in[j];
    }
    for (int i = 1; i <= task_n; ++i) {
        if (in[i] == 0) {
            que.push(i);
        }
    }
    int u;
    int cnt = 0;
    while (que.size()) {
        u = que.front();
        que.pop();
        tp_seq[++cnt] = u;
        for (int tp = 0; tp < 2; ++tp)
            for (auto v : task[u].rg[tp]) {
                --in[v];
                if (in[v] == 0) {
                    que.push(v);
                }
            }
    }
}

void reset() {
    for (int i = 1; i <= disk_n; ++i) {
        disk[i].use = 0;
    }
    for (int i = 1; i <= mach_n; ++i) {
        mach[i].time = 0;
    }
    for (int i = 1; i <= task_n; ++i) {
        task[i].a[0] = 0;
    }
}

void install_task(int task_id, int mach_id, int disk_id) {
    disk[disk_id].use += task[task_id].data;
    int tmp = max(0, mach[mach_id].time - task[task_id].a[0]);
    
    task[task_id].disk = disk_id;
    task[task_id].mach = mach_id;

    task[task_id].a[0] += tmp;
    task[task_id].a[1] += tmp;

    task[task_id].a[2] = task[task_id].a[1] + up(task[task_id].siz, mach[mach_id].power);

    task[task_id].a[3] = task[task_id].a[2] + up(task[task_id].data, disk[disk_id].speed);

    mach[mach_id].time = task[task_id].a[3];
}

void work_task_fast(int task_id) {
    int disk_id = -1, max_speed = -1;
    for (int i = 1; i <= disk_n; ++i) {
        if (task[task_id].data + disk[i].use > disk[i].cap) {
            continue;
        }
        if (disk[i].speed > max_speed) {
            max_speed = disk[i].speed;
            disk_id = i;
        }
    }
    // currently a, b not consider machine
    int mach_id = -1, min_c = INF;
    for (auto i : task[task_id].affi) {
        int cost = up(task[task_id].siz, mach[i].power);
        cost += max(0, mach[i].time - task[task_id].a[0]);
        if (cost < min_c) {
            min_c = cost;
            mach_id = i;
        }
    }
    install_task(task_id, mach_id, disk_id);
}

void calc_task_ab(int task_id) {
    for (auto i : task[task_id].rg[1]) {
        task[task_id].a[0] = max(task[task_id].a[0], task[i].a[2]);
    }
    int sum_read = 0;
    for (auto i : task[task_id].rg[0]) {
        task[task_id].a[0] = max(task[task_id].a[0], task[i].a[3]);
        sum_read += up(task[i].data, disk[task[i].disk].speed);
    }
    task[task_id].a[1] = task[task_id].a[0] + sum_read;
}

void topo() {
    for (int i = 1; i <= task_n; ++i) in[i] = 0;
    priority_queue<P> que;
    for (int i = 1; i <= task_n; ++i) {
        for (int tp = 0; tp < 2; ++tp)
            for (auto j : task[i].g[tp])
                ++in[j];
    }
    for (int i = 1; i <= task_n; ++i) {
        if (in[i] == 0) {
            que.push(P(task[i].next_max_time, i));
        }
    }
    int u;
    while (que.size()) {
        u = que.top().second;
        que.pop();
        calc_task_ab(u);
        work_task_fast(u);
        for (int tp = 0; tp < 2; ++tp)
            for (auto v : task[u].g[tp]) {
                --in[v];
                if (in[v] == 0) {
                    que.push(P(task[v].next_max_time, v)); // ?
                }
            }
    }
}

void calc_next_max_time(int task_id) {
    task[task_id].next_max_time = 0;
    for (auto i : task[task_id].g[1]) {
        task[task_id].next_max_time = max(task[task_id].next_max_time, task[task_id].a[2] - task[task_id].a[0] + task[i].next_max_time); // ?
    }
    for (auto i : task[task_id].g[0]) {
        task[task_id].next_max_time = max(task[task_id].next_max_time, task[task_id].a[3] - task[task_id].a[0] + task[i].next_max_time);
    }
}

void r_topo() {
    for (int i = task_n; i; i--) {
        int u = tp_seq[i];
        calc_next_max_time(u);
    }
}

void random_start() {
    last_ans = -1;
    mach_k = rand() % 10;
    disk_k = rand() % 10;
    for (int i = task_n; i; i--) {
        int task_id = tp_seq[i];
        task[task_id].next_max_time = 0;
        int tmp = mach_k * task[task_id].esti_cost_mach_time + disk_k * task[task_id].esti_cost_disk_time;
        for (auto i : task[task_id].g[1]) {
            task[task_id].next_max_time = max(task[task_id].next_max_time, tmp + task[i].next_max_time); // ?
        }
        for (auto i : task[task_id].g[0]) {
            task[task_id].next_max_time = max(task[task_id].next_max_time, tmp + task[i].next_max_time);
        }        
    }
}

bool update() {
    int ans = -1;
    for (int i = 1; i <= task_n; ++i) {
        ans = max(ans, task[i].a[3]);
    }
    if (ans < best_ans) {
        best_ans = ans;
        for (int i = 1; i <= task_n; ++i) {
            task[i].best_begin = task[i].a[0];
            task[i].best_disk = task[i].disk;
            task[i].best_mach = task[i].mach;
        }
    }
    if (last_ans == ans) return false;
    last_ans = ans;
    return true;
}

void output() {
    // cout << best_ans << endl;
    for (int i = 1; i <= task_n; ++i) {
        printf("%d %d %d %d\n", i, task[i].best_begin, task[i].best_mach, task[i].best_disk);
    }
    exit(0);
}

void work() {
    for (int k = 0; k < 5000; ++k) {
        if (get_time() > 14) {
            output();
        }
        reset();
        topo();
        if (update()) {
            r_topo();
        } else {
            random_start();
        }
    }
}

int main() {
    // freopen("1.in", "r", stdin);
    read_in();
    init();
    work();
    output();
    return 0;
}