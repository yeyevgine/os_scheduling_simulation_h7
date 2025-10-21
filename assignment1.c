#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int start_time;
    int finish_time;
    int done;
} Process;

static void swap(Process *a, Process *b)
{
    Process t = *a;
    *a = *b;
    *b = t;
}
static void copy_procs(Process *dst, const Process *src, int n)
{ 
    for(int i = 0; i < n; i++) {
        dst[i] = src[i]; 
    }
}

static void sort_by_arrival_then_pid(Process *p, int n){
    for(int i = 0; i < n-1; i++)
        for(int j = 0; j < n-1-i; j++)
            if( (p[j].arrival_time > p[j+1].arrival_time) || (p[j].arrival_time == p[j+1].arrival_time && p[j].pid > p[j+1].pid) ){
                    swap(&p[j], &p[j+1]);
                }
                
}

static void sort_by_pid(Process *p, int n){
    for(int i = 0; i < n-1; i++)
        for(int j = 0; j < n-1-i; j++)
            if(p[j].pid > p[j+1].pid) {
                swap(&p[j], &p[j+1]);
            }
}
static void sort_by_waiting_time_then_pid(Process *p, int n){
    for(int i = 0; i < n-1; i++)
        for(int j = 0; j < n-1-i; j++)
            if( (p[j].waiting_time > p[j+1].waiting_time) ||
                (p[j].waiting_time == p[j+1].waiting_time && p[j].pid > p[j+1].pid) ) {
                    swap(&p[j], &p[j+1]);
                }
                
}

typedef enum { 
	ORDER_PID, 
	ORDER_WT 
} TableOrder;

static void print_table_and_avgs(Process *p, int n, TableOrder order){
    double sumWT = 0;
    double sumTAT = 0;
    double sumRT= 0;

    Process *tmp = (Process*)malloc(n*sizeof(Process));

    copy_procs(tmp, p, n);
    if(order ==ORDER_PID) {
        sort_by_pid(tmp, n);
    } else {
        sort_by_waiting_time_then_pid(tmp, n);
    }         

    printf("PID    AT     BT     WT     TAT    RT\n");
    for(int i = 0; i < n; i++){
        printf("%-7d %-6d %-6d %-6d %-6d %-6d\n",
               tmp[i].pid, tmp[i].arrival_time, tmp[i].burst_time, tmp[i].waiting_time, tmp[i].turnaround_time, tmp[i].response_time);
        sumWT += tmp[i].waiting_time;
        sumTAT += tmp[i].turnaround_time;
        sumRT += tmp[i].response_time;
    }
    free(tmp);

    printf("\nAverage Waiting Time: %.2f\n", sumWT/n);
    printf("Average Turnaround Time: %.2f\n", sumTAT/n);
    printf("Average Response Time: %.2f\n", sumRT/n);
}


typedef struct {
    int pid;
    int start;
    int finish;
    int idle;
} Slot;


static void print_gantt(Slot *slots, int m){
    printf("Gantt Chart: ");
    for(int i = 0; i < m; i++){
        printf("| %s%d ", slots[i].idle?"IDLE ":"P", slots[i].idle?0:slots[i].pid);
    }
    printf("|\n");
}


/* FCFS*/
static void run_fcfs(Process *procs, int n){
    Process *p=(Process*)malloc(n*sizeof(Process));
    copy_procs(p, procs,n);
    sort_by_arrival_then_pid(p, n);

    int time =0;
    Slot *slots = (Slot*)malloc((2*n+5)*sizeof(Slot));
    int sc = 0;

    for(int i = 0; i < n; i++){
        if(time < p[i].arrival_time){
            slots[sc++] = (Slot){.pid=0,.start=time,.finish=p[i].arrival_time,.idle =1};
            time = p[i].arrival_time;
        }

        p[i].start_time = time;
        p[i].waiting_time = p[i].start_time-p[i].arrival_time;
        p[i].response_time = p[i].waiting_time;
        p[i].finish_time = p[i].start_time+p[i].burst_time;
        p[i].turnaround_time = p[i].finish_time-p[i].arrival_time;

        slots[sc++] = (Slot){.pid=p[i].pid,.start=p[i].start_time,.finish=p[i].finish_time,.idle = 0};
        time =p[i].finish_time;
    }

    printf("=== First Come First Serve(FCFS) ===\n");
    print_gantt(slots, sc);

    for(int i =0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(procs[j].pid == p[i].pid){
                procs[j] = p[i];
                break;
            }
        }
    }

    print_table_and_avgs(procs, n,ORDER_PID);

    free(slots);
    free(p);
}



/*SJF*/
static void run_sjf(Process *procs, int n){
    Process *p = (Process*)malloc(n*sizeof(Process));
    copy_procs(p, procs, n);
    for(int i = 0; i < n; i++){
        p[i].done =0;
    }

    int time = 1e9;
    int idx = -1;

    for(int i= 0; i < n; i++) {
        if(p[i].arrival_time < time){ 
            time = p[i].arrival_time;
            idx = i;
        }
    }

    time = (idx >= 0)? p[idx].arrival_time : 0;

    Slot *slots = (Slot*)malloc((2*n + 5)*sizeof(Slot));
    int sc = 0;
    int completed = 0;

    while(completed < n){
        int pick = -1;
        for(int i = 0; i < n; i++){
            if(p[i].arrival_time <= time && !p[i].done ){
                if(pick == -1) {
                    pick=i;
                } else if( (p[i].burst_time < p[pick].burst_time) || 
                            (p[i].burst_time == p[pick].burst_time && p[i].arrival_time < p[pick].arrival_time) ||
                            (p[i].burst_time == p[pick].burst_time && p[i].arrival_time == p[pick].arrival_time && p[i].pid < p[pick].pid) ) {
                                pick = i;
                            }
            }
        }

        if(pick == -1){
            int nextArr = 1e9;
            for(int i = 0; i < n; i++){
                if(!p[i].done && p[i].arrival_time < nextArr){
                     nextArr = p[i].arrival_time;
                }
            }

            if(nextArr > time){ 
                slots[sc++] = (Slot){.pid = 0, .start =time, .finish = nextArr, .idle = 1};
                time = nextArr;
            }
            continue;
        }

        p[pick].start_time = time;
        p[pick].waiting_time = time - p[pick].arrival_time;
        p[pick].response_time = p[pick].waiting_time;
        p[pick].finish_time = time + p[pick].burst_time;
        p[pick].turnaround_time = p[pick].finish_time - p[pick].arrival_time;

        slots[sc++] = (Slot){.pid = p[pick].pid, .start = p[pick].start_time, .finish = p[pick].finish_time, .idle = 0};
        time = p[pick].finish_time; 
        p[pick].done = 1;
        completed++;
    }

    printf("\n=== Shortest Job First (SJF) ===\n");
    print_gantt(slots, sc);

    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(procs[j].pid == p[i].pid){
                procs[j] = p[i];
                break;
            }
        }
    }

    print_table_and_avgs(procs,n, ORDER_WT);

    free(slots);
    free(p);
}





int main()
{
    int n;
    printf("Enter the number of proceses: ");
    if(scanf("%d",&n) != 1 || n <=0){
        fprintf(stderr, "Invalid number of processes.\n");
        return 1;
    }

    Process *procs = (Process*)malloc(n*sizeof(Process));

    for(int i = 0; i < n; i++){

        procs[i].pid = i+1;
        procs[i].waiting_time = procs[i].turnaround_time = procs[i].response_time = 0;
        procs[i].start_time = 0;
        procs[i].finish_time = 0;
        procs[i].done = 0;
        printf("Enter the arrival time and burst time for process %d: ", i+1);

        if(scanf("%d %d", &procs[i].arrival_time,&procs[i].burst_time) != 2 || procs[i].burst_time < 0){

            fprintf(stderr, "Invalid input.\n");
            free(procs);
            return 1;
        }
    }

    Process *fcfs_out = (Process*)malloc(n*sizeof(Process));
    copy_procs(fcfs_out, procs, n);
    run_fcfs(fcfs_out, n);

    Process *sjf_out = (Process*)malloc(n*sizeof(Process));
    copy_procs(sjf_out, procs, n);
    run_sjf(sjf_out, n);

    free(fcfs_out);
    free(sjf_out);
    free(procs);


    return 0;
}

