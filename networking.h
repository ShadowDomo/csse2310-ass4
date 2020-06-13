#ifndef NETWORKING_H
#define NETWORKING_H

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

#define PORT_MAX_CHARS 6 // incl '\0'

/** Error codes for Roc. **/
typedef enum RocErrorCodes {
    NORMAL_END = 0,
    ROC_INCORRECT_NUM_ARGS = 1,
    ROC_INVALID_MAPPER_PORT = 2,
    ROC_MAPPER_REQUIRED = 3,
    ROC_MAPPER_CONNECTION_ERROR = 4,
    ROC_NO_MAP_ENTRY = 5,
    ROC_FAILED_TO_CONNECT = 6
} RocErrorCodes;

/** Error codes for Roc. **/
typedef enum ControlErrorCodes {
    CTRL_INCORRECT_NUM_ARGS = 1,
    CTRL_INVALID_CHARS = 2,
    CTRL_INVALID_PORT = 3,
    CTRL_MAP_CONNECTION_ERROR = 4,
} ControlErrorCodes;

/** Representation of an Airport. **/
typedef struct Airport {
    // Airport ID
    char* id;

    // Airport info
    char* info;

    // port which Airport is on
    int port;
} Airport;

/** Representation of a plane for Roc. **/
typedef struct Plane {
    // plane id
    char* id;
} Plane;

/** State of a mapper program **/
typedef struct WorldState {
    // List of known airports
    Airport** airports;

    // number of airports
    int countAirports;

} WorldState;

/** State of a control program **/
typedef struct ControlState {
    // planes which controller has seen
    Plane** planes;

    // number of planes which controller has seen
    int countPlanes;

    // mapper port given in arg
    int mapperPort;

    // airport ID
    char* airportId;

    // airport info
    char* airportInfo;
} ControlState;

/** Thread parameters for new mapper connection **/
struct Param {
    // file descriptor for new thread
    int* fileDescriptor;
    
    // Mapper program state
    WorldState* worldState;
    
    // Semaphore
    sem_t* guard;
};

/** Thread parameters for new control connection **/
struct ControlParam {
    // File descriptor for new thread
    int* fileDescriptor;
    
    // Control program state
    ControlState* controlState;
    
    // Semaphore
    sem_t* guard;
};

void connect_to_mapper(const ControlState* controlState, int port);

void start_map_thread(WorldState* worldState, int* connectionFd,
        int countMapParams, struct Param* par, sem_t* lock);

void start_control_thread(ControlState* controlState, int* connectionFd,
        int countControlParams, struct ControlParam* controlParam,
        sem_t* lock);

void thread_listener(WorldState* worldState, ControlState* controlState,
        bool hasWorldState, bool hasControlState, int server, int port);

void do_mapper_query(char* input, WorldState* worldState, FILE* writeStream);

void add_mapping(char* input, WorldState* worldState);
void control_exit(ControlErrorCodes errorCode);
void roc_exit(RocErrorCodes errorCode);
void setup_sockets(WorldState* worldState, ControlState* controlState);
int outbound_socket_maker(int mapperPort);
void allocate_airports(WorldState* worldState);

#endif