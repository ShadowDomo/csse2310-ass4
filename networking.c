#include "networking.h"


/** Initializes semaphore.
 *
 * @param l Semaphore
 */
void init_lock(sem_t* l) {
    sem_init(l, 0, 1);
}

/** Takes control of semaphore.
 *
 * @param l Semaphore
 */
void take_lock(sem_t* l) {
    sem_wait(l);
}

/** Unlocks semaphore.
 *
 * @param l Semaphore
 */
void release_lock(sem_t* l) {
    sem_post(l);
}

/** Dynamically allocates airports.
 *
 * @param worldState Mapper program state
 */
void allocate_airports(WorldState* worldState) {
    if (worldState->countAirports == 0) {
        worldState->airports = malloc(sizeof(Airport*));
        worldState->airports[0] = malloc(sizeof(Airport));
    } else {
        worldState->airports = realloc(worldState->airports,
                sizeof(Airport*) * (worldState->countAirports + 1));
        worldState->airports[worldState->countAirports] = malloc(
    }
}

/** Adds an Airport identified by id and port.
 *
 * @param id Airport id
 * @param port Airport port
 * @param worldState Mapper program state
 */
void add_airport(char* id, int port, WorldState* worldState) {
    // reallocate airport array
    allocate_airports(worldState);
    
    worldState->airports[worldState->countAirports]->port = port;
    worldState->airports[worldState->countAirports]->id = (char *) malloc(
            sizeof(char) * strlen(id) + 1);
    strcpy(worldState->airports[worldState->countAirports]->id, id);
    
    worldState->countAirports++;
}

/** Retrieves airport given an id.
 *
 * @param worldState The mapper program state
 * @param id The id of the airport to be searched for
 * @return Pointer to airport known by id
 */
Airport* get_airport(WorldState* worldState, char* id) {
    for (int i = 0; i < worldState->countAirports; ++i) {
        if (strcmp(worldState->airports[i]->id, id) == 0) {
            return worldState->airports[i];
        }
    }

    
    // if we get here, there is no airport with given id
    return 0;
}

/** Exits control program with given error code.
 *
 * @param errorCode Error code to exit with
 * @error
 *   1 - Incorrect number of args
 *   2 - Invalid characters in ID or info
 *   3 - Port specified but not a strictly positive number < 65536
 *   4 - Could not conenct to mapper
 */
void control_exit(ControlErrorCodes errorCode) {
    switch (errorCode) {
        case CTRL_INCORRECT_NUM_ARGS:
            fprintf(stderr, "Usage: control2310 id info [mapper]");
            break;
        case CTRL_INVALID_CHARS:
            fprintf(stderr, "Invalid char in parameter");
            break;
        case CTRL_INVALID_PORT:
            fprintf(stderr, "Invalid port");
            break;
        case CTRL_MAP_CONNECTION_ERROR:
            fprintf(stderr, "Can not connect to map");
            break;
    }
    
    fprintf(stderr, "\n");
    fflush(stderr);
    
    exit(errorCode);
}

/** Exits roc program with given error code.
 *
 * @param errorCode Error code to display
 * @Error codes
 *   1 - Incorrect number of args
 *   2 - Mapper is not dash but is not a valid port either
 *   3 - A destination is not a valid port, but no valid mapper was was given
 *   4 - Error connecting to the mapper port
 *   5 - Mapper has no value for one of the queried destinations
 *   6 - Could not connect to a destination port
 */
void roc_exit(RocErrorCodes errorCode) {
    switch (errorCode) {
        case ROC_INCORRECT_NUM_ARGS:
            fprintf(stderr, "Usage: roc2310 id mapper {airports}");
            break;
        case ROC_INVALID_MAPPER_PORT:
            fprintf(stderr, "Invalid mapper port");
            break;
        case ROC_MAPPER_REQUIRED:
            fprintf(stderr, "Mapper required");
            break;
        case ROC_MAPPER_CONNECTION_ERROR:
            fprintf(stderr, "Failed to connect to mapper");
            break;
        case ROC_NO_MAP_ENTRY:
            fprintf(stderr, "No map entry for destination");
            break;
        case ROC_FAILED_TO_CONNECT:
            fprintf(stderr, "Failed to connect to at least one destination");
            break;
        case NORMAL_END:
            exit(0);
    }
    
    fprintf(stderr, "\n");
    fflush(stderr);
    
    exit(errorCode);
}

/** Lexicographical qsort comparison func.
 *
 * @param a Airport one
 * @param b Airpor two
 * @return < 1 if Airport one goes before Airport two, 0 if equal, else > 1
 */
int cmp_func(const void* a, const void* b) {
    Airport* first = *(Airport**) a;
    Airport* second = *(Airport**) b;
    
    return strcmp(first->id, second->id);
}

/** Prints names and their corresponding ports in lexicographical order.
 *
 * @param worldState The mapper program state
 * @param writeStream Filestream to write to
 */
void print_mappings(WorldState* worldState, FILE* writeStream) {
    int countAirports = worldState->countAirports;
    
    if (countAirports == 0) {
        return;
    }
    int dog = worldState->countAirports;
    
    // sort lexiocographically
    qsort(&worldState->airports[0], countAirports, sizeof(Airport*), cmp_func);
    
    for (int i = 0; i < countAirports; ++i) {
        fprintf(writeStream, "%s:%d\n", worldState->airports[i]->id,
                worldState->airports[i]->port);
        fflush(writeStream);
    }
}

/** Checks input received by the mapper.
 *
 * @param input
 * @param lock
 * @return 0 if a ? was seen
 *         1 if a ! was seen
 *         2 if a @ was seen
 *         -1 otherwise.
 */
void check_string(char* input, sem_t* lock, WorldState* worldState,
        FILE* writeStream) {
    take_lock(lock);
   
    // clean '\n'
    int lenInput = (int) strlen(input);
    input[lenInput - 1] = '\0';
    /** Send the port number for the airport called ID **/
    if (strncmp(input, "?", 1) == 0) {
        do_mapper_query(input, worldState, writeStream);
        release_lock(lock);
        return;
    }
    /** Add airport called ID with PORT as the port number **/
    if (strncmp(input, "!", 1) == 0) {
        // ensure correct format
        char* colonLocation = strchr(input, ':');
        if (colonLocation == NULL) {
            release_lock(lock);
            return;
        }
        
        add_mapping(input, worldState);
        release_lock(lock);
        return;
    }
    /** Send back all names and their corresponding ports **/
    if (strncmp(input, "@", 1) == 0) {
        if (strlen(input) != 2) {
            print_mappings(worldState, writeStream);
            
        }
        release_lock(lock);
        return;
    }
    
    release_lock(lock);
}

/** Adds a mapping of id: portnumber to the mapper.
 *
 * @param input String containing id: portnumber to be decoded
 * @param worldState The mapper program state
 * @param colonLocation Location of the colon in the input string
 */
void add_mapping(char* input, WorldState* worldState) {
    char* colonLocation = strchr(input, ':');
    int indexOfDelim = colonLocation - input;
    int indexOfPort = indexOfDelim + 1;
    
    // get id
    char id[80];
    snprintf(id, indexOfDelim, "%s", input + 1);
    
    // get the port number
    char port[6];
    snprintf(port, PORT_MAX_CHARS, "%s", input + indexOfPort);
    
    char* trash;
    int portNum = (int) strtol(port, &trash, 10);
    
    // if ! used for an ID which already has entry
    for (int i = 0; i < worldState->countAirports; ++i) {
        Airport* airport = worldState->airports[i];
       
        // dont add mapping if id previously used
        if (strcmp(airport->id, id) == 0) {
            return;
        }
    }
    // if the port is valid
    if (strlen(trash) == 0 && strlen(port) != 0) {
        add_airport(id, portNum, worldState);
    }
}

/** Checks input received and performs ? query
 *
 * @param input String input which was received
 * @param worldState The mapper program stat
 * @param writeStream Filestream to write to
 */
void do_mapper_query(char* input, WorldState* worldState, FILE* writeStream) {
    // extract id from input
    char id[80];
    snprintf(id, 80, "%s", input + 1);
    
    // Send back the port number for the airport called id
    Airport* airport = get_airport(worldState, id);
    
    // if there is no mapping
    if (airport == NULL) {
        fprintf(writeStream, ";\n");
    } else {
        // if there is an entry corresponding to that ID
        int portNumber = airport->port;
        fprintf(writeStream, "%d\n", portNumber);
    }
    
    fflush(writeStream);
}

/** For control qsort.
 *
 * @param a Plane one
 * @param b Plane two
 * @return < 1 if first goes before second. 0 if equal. > 1 otherwise.
 */
int control_cmp_func(const void* a, const void* b) {
    Plane* first = *(Plane**) a;
    Plane* second = *(Plane**) b;
    
    return strcmp(first->id, second->id);
}

/** Checks input received by control.
 *
 * @param input String to be checked
 * @param lock Semaphore
 * @param controlState Control program state
 * @param writeStream Filestream to write to
 */
void check_control_string(char* input, sem_t* lock, ControlState* controlState,
        FILE* writeStream) {
    take_lock(lock);
    int countPlanes = controlState->countPlanes;
    // send back lexiographic order of rocs
    if (strncmp(input, "log", 3) == 0) {
        // sort lexiocographically
        qsort(&controlState->planes[0], countPlanes,
              sizeof(Plane*), control_cmp_func);
        
        for (int i = 0; i < countPlanes; ++i) {
            fprintf(writeStream, "%s\n", controlState->planes[i]->id);
            fflush(writeStream);
        }
        fprintf(writeStream, ".\n");
        fflush(writeStream);
        
        fclose(writeStream);
        return;
    }
    
    // consider the text to be the plane's id - send back control's info
    fprintf(writeStream, "%s\n", controlState->airportInfo);
    fflush(writeStream);
    
    int lenInput = (int) strlen(input);
    input[lenInput - 1] = '\0';
    
    // add plane to list
    if (controlState->countPlanes == 0) {
        controlState->planes = malloc(sizeof(Plane*));
        controlState->planes[0] = malloc(sizeof(Plane));
    } else {
        controlState->planes = realloc(controlState->planes,
                sizeof(Plane*) * (controlState->countPlanes + 1));
        controlState->planes[controlState->countPlanes] =
                malloc(sizeof(Plane));
    }
    
    controlState->planes[controlState->countPlanes]->id = malloc(
            sizeof(char) * 80);
    strcpy(controlState->planes[controlState->countPlanes]->id, input);
    controlState->countPlanes++;
    
    release_lock(lock);
}

/** Control thread doer
 *
 * @param v Thread parameters
 * @return need for thread function
 */
void* control_doer(void* v) {
    // TODO change name
    struct ControlParam* p = (struct ControlParam*) v;
    take_lock(p->guard);
    ControlState* controlState = p->controlState;
    int fd2 = dup(*p->fileDescriptor);
    FILE* writeStream = fdopen(*p->fileDescriptor, "w");
    FILE* readStream = fdopen(fd2, "r");
    release_lock(p->guard);
    while (true) {
        char input[80];
        if (fgets(input, 80, readStream) != NULL) {
            // check string values
            check_control_string(input, p->guard, controlState, writeStream);
        }
    }
}

/** Handles a connection to a process.
 *
 * @param v Struct of thread parameters
 * @return need for thread function
 */
void* mapper_doer(void* v) {
    // TODO change name
    struct Param* p = (struct Param*) v;
    WorldState* worldState = p->worldState;
    take_lock(p->guard);
    int fd2 = dup(*p->fileDescriptor);
    FILE* writeStream = fdopen(*p->fileDescriptor, "w");
    FILE* readStream = fdopen(fd2, "r");
    release_lock(p->guard);
    while (true) {
        char input[80];
        if (fgets(input, 80, readStream) != NULL) {
            // check string values
            check_string(input, p->guard, worldState, writeStream);
        }
    }
}

/** Makes socket and returns file descriptor to communicate on.
 *
 * @param mapperPort Port to make socket on
 * @return File descriptor to communicate on for given port
 */
int outbound_socket_maker(int mapperPort) {
    // no mapper port was given
    if (mapperPort == -1) {
        return 0;
    }
    
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1) {
        return -1;
    }
    
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    char port[6];
    sprintf(port, "%d", mapperPort);
    
    // error resolving host
    if (getaddrinfo("localhost", port, &hints, &ai)) {
        return -1;
    }
    
    // error connecting
    if (connect(client, (struct sockaddr*) ai->ai_addr,
            sizeof(struct sockaddr)) < 0) {
        return -1;
    }
    
    return client;
}

/** Sets up sockets.
 *
 * @param worldState The mapper program state
 * @param controlState The control program state
 */
void setup_sockets(WorldState* worldState, ControlState* controlState) {
    bool hasWorldState = true;
    bool hasControlState = true;
   
    // setup socket
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo("localhost", 0, &hints, &ai);
    
    // create socket and bind to port
    int serv = socket(AF_INET, SOCK_STREAM, 0);
    bind(serv, (struct sockaddr*) ai->ai_addr, sizeof(struct sockaddr));
    
    // which port did we get
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    getsockname(serv, (struct sockaddr*) &ad, &len);
    int port = ntohs(ad.sin_port);
    
    if (worldState == NULL) {
        hasWorldState = false;
    }
    if (controlState == NULL) {
        hasControlState = false;
    } else {
        if (controlState->mapperPort != -1) {
            connect_to_mapper(controlState, port);
        }
    }
    
    // allow up to 10 requests to queue
    listen(serv, 10);
    
    thread_listener(worldState, controlState, hasWorldState, hasControlState,
                    serv, port);
}

/** Initialises socket and listens for incoming connections.
 *
 * @param worldState The mapper program state
 * @param controlState The control program state
 * @param hasWorldState True, if program is a mapper. False if control
 * @param hasControlState True if program is control. False if mapper
 * @param server The current server (localhost)
 * @param port Ephemeral port to listen on
 */
void thread_listener(WorldState* worldState, ControlState* controlState,
        bool hasWorldState, bool hasControlState, int server, int port) {
    int connectionFd;
    sem_t lock;
    init_lock(&lock);
    printf("%u\n", port);
    fflush(stdout);
    // listen for connections
    int countMapParams = 0;
    int countControlParams = 0;
    struct Param* par = malloc(sizeof(struct Param));
    struct ControlParam* controlParam = malloc(sizeof(struct ControlParam));
    while (connectionFd = accept(server, 0, 0), connectionFd >= 0) {
        if (hasWorldState) {
            start_map_thread(worldState, &connectionFd, countMapParams, par,
                    &lock);
        } else if (hasControlState) {
            start_control_thread(controlState, &connectionFd,
                    countControlParams, controlParam, &lock);
        }
    }
}

/** Starts a thread for the control program.
 *
 * @param controlState The state of the control program
 * @param connectionFd File descriptor to communicate on
 * @param countControlParams Number of control threads
 * @param controlParam Array of thread parameters to pass to thread
 * @param lock Semaphore
 */
void start_control_thread(ControlState* controlState, int* connectionFd,
        int countControlParams,
        struct ControlParam* controlParam, sem_t* lock) {
    take_lock(lock);
    pthread_t threadId;
    if (countControlParams != 0) {
        controlParam = realloc(controlParam, sizeof(struct ControlParam) *
                (countControlParams + 1));
    }
    controlParam[countControlParams].controlState = malloc(
            sizeof(WorldState*));
    controlParam[countControlParams].controlState = controlState;
    
    controlParam[countControlParams].fileDescriptor = malloc(sizeof(int*));
    controlParam[countControlParams].fileDescriptor = connectionFd;
    
    controlParam[countControlParams].guard = malloc(sizeof(sem_t*));
    controlParam[countControlParams].guard = lock;
    countControlParams++;
    release_lock(lock);
    pthread_create(&threadId, 0, control_doer,
            &controlParam[countControlParams - 1]);
}

/** Starts a thread for a connection to the mapper program.
 *
 * @param worldState The Mapper state
 * @param connectionFd File descriptor to communicate with
 * @param countMapParams Thread index
 * @param par Thread parameter
 * @param lock Semaphore
 */
void start_map_thread(WorldState* worldState, int* connectionFd,
        int countMapParams, struct Param* par, sem_t* lock) {
    take_lock(lock);
    
    pthread_t threadId;
    
    if (countMapParams != 0) {
        par = realloc(par,
                sizeof(struct Param) * (countMapParams + 1));
    }
    par[countMapParams].fileDescriptor = malloc(sizeof(int*));
    par[countMapParams].fileDescriptor = connectionFd;
    
    par[countMapParams].guard = malloc(sizeof(sem_t*));
    par[countMapParams].guard = lock;
    
    par[countMapParams].worldState = malloc(sizeof(WorldState*));
    par[countMapParams].worldState = worldState;
    countMapParams++;
    release_lock(lock);
    pthread_create(&threadId, 0, mapper_doer, &par[countMapParams - 1]);
}

/** Connects to mapper and sends id with port.
 *
 * @param controlState The state of the control program
 * @param port The port to connect to
 * @exit
 *    CTRL_MAP_CONNECTION_ERROR - Error connecting to mapper
 */
void connect_to_mapper(const ControlState* controlState, int port) {
    char message[100];
    snprintf(message, 100, "!%s:%d\n", controlState->airportId, port);
    int client = outbound_socket_maker(controlState->mapperPort);
    
    // connection error
    if (client == -1) {
        control_exit(CTRL_MAP_CONNECTION_ERROR);
    }
    // write to server
    FILE* writer = fdopen(client, "w");
    fprintf(writer, "%s", message);
    fflush(writer);
    
    fclose(writer);
}
