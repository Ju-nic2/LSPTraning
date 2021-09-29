#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/utsname.h>

#define CPUINFO_PROCESSOR 0 
#define CPUDINFO_VENDORID 1
#define CPUINFO_CPUFAMILY 2
#define CPUINFO_CPUMODEL 3
#define CPUINFO_MODELNAME 4
#define CPUINFO_STEPPING 5
#define CPUINFO_CPUMHZ 7 
#define CPUINFO_PYSICALID 9
#define CPUINFO_CPUCORES 12
#define CPUINFO_FLAGS 19
#define CPUINFO_BOBOMIPS 21
#define CPUINFO_ADDRESS 24
#define CPUINFO_END 26

#define CUT 1
#define UNCUT 0

#define LINEBUFSIZE 1024
#define SMALLBUFSIZE 256
#define BUFSIZE 32

#define CPU 0
#define SOCKET 1
#define CORE 2
#define NODE 3
#define CACHE 4
#define ONLINE 5


char *path_sys_cpu = "/sys/devices/system/cpu";
char *path_sys_cpu2 = "/sys/devices/system/cpu/cpu";
char *path_proc_cpuinfo = "/proc/cpuinfo";
char *path_sys_node = "/sys/devices/system/node";
char *path_sys_node2 = "/sys/devices/system/node/node";
enum{
    LITTLE_EDIAN =0,
    BIG_EDIAN  
};
char *byte_order[2] = {"Little Edian","Big Edian"};

//for CPU-op-modes
enum {
    LONG_MODE = 0, //"lm" , 32bit, 64bit
    THUMB,// CPU IS 16 ISA 16bit
};
char *op_modes[2] = {"32bit, 64bit",
                     "16bit"};

// For Virtualization type 
enum {
    VME = 0, //Virtualized server, full 
    SVM, //AMD CPU, AMD-V 
    VMX //Intel-VT technology , VT-x
};

char *virtual_type[3] = { "full",
                          "AMD-V",
                          "VT-x"
                        };
enum {
    ITLB_MYLTIHIT = 0,
    L1TF,
    MDS,
    MELTDOUN,
    SPEC_STORE_BYPASS,
    SPECTRE_V1,
    SPECTRE_V2,
    SRBDS,
    TSX_ASYNC_ABORT,
};
enum{
    L1d = 0,
    L1i,
    L2,
    L3
};
struct counter
{
    int cpuCount; // count procesor - in same pysical_id
    int *onLineList;
    float threadPerCore; //(cpu_cores*socket)/procesor  - in same physical_id
    int corePerSocket; //cpu cores
    int socketCount; //count different pysical_ids
};
    
struct lscpu_type
{
    char *Architecture;
    char *opmode;
    char *byteOrder;
	char *addressSize;

    struct numa *nptr;

    char *vendorID;
    char *cpuFamily;
    char *model;
    char *modelName;
    char *stepping;
    char *cpuMHz;
    char *BogoMIPS;
    char *HypervisorVendor;
    char *virtualTayp;

    char *vulnerabilities[9];
    char *flag;
};

struct cpu_cache
{
    char *L1i;
    char *L1d;
    char *L2;
    char *L3;
};

//get from /cpu/possible
struct lscpu_cpu
{
    int count;
    struct cpu_node *node;
    
};

struct cpu_node
{
    int logical_id;// auto allocate form possible list
    int node; // get from numa node
    int socket;///cpu#/topologyphysical_package_id
    int core; // /cpu#/topology/core_id
    int cache[4];// /cpu#/cache/index#           
    int online;// /cpu/online
};

struct numa
{
    int onlineCount;
    struct numa_node * node;
};

struct numa_node
{
    char* name;
    int* cpulist;
};
