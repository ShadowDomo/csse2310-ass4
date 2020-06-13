#include "mapper.h"

/** Checks args given to roc program.
 *
 * @param argc Program argument count
 * @param argv Program arguments
 * @exit
 *  ROC_INCORRECT_NUM_ARGS - incorrect number of args supplied
 *  ROC_INVALID_MAPPER_PORT - invalid mapper port
 *  ROC_MAPPER_REQUIRED - No valid mapper port was given when needed
 */
void check_args(int argc, char** argv) {
    // incorrect number of args
    if (argc < 3) {
        roc_exit(ROC_INCORRECT_NUM_ARGS);
        return;
    }
    
    // mapper is not dash but is not a valid port either
    if (strcmp(argv[2], "-") != 0) {
        char* otherHalf;
        int mapperPortNum = (int) strtol(argv[2], &otherHalf, 10);
        
        if (strlen(otherHalf) != 0) {
            roc_exit(ROC_INVALID_MAPPER_PORT);
            return;
        }
        if (mapperPortNum <= 0 || mapperPortNum > 65535) {
            roc_exit(ROC_INVALID_MAPPER_PORT);
            return;
        }
    }
    
    // destination is not a valid port number (so the mapper is required)
    // but no valid mapper port was given
    for (int i = 3; i < argc; ++i) {
        char* otherHalf;
        int mapperPortNum = (int) strtol(argv[3], &otherHalf, 10);
        
        if (mapperPortNum <= 0 || mapperPortNum > 65535 ||
                strlen(otherHalf) != 0) {
            if (strcmp(argv[2], "-") == 0) {
                roc_exit(ROC_MAPPER_REQUIRED);
                return;
            }
        }
    }
}

/** Prints roc state to stdout.
 *
 * @param worldState The roc program state
 */
void print_log(WorldState* worldState) {
    for (int i = 0; i < worldState->countAirports; ++i) {
        Airport* airport = worldState->airports[i];
        printf("%s", airport->info);
    }
    fflush(stdout);
}

/** Connects to each destination and stores destination info.
 *
 * @param worldState The roc program state
 * @param planeId The planeId of the roc program
 * @return True if the connection failed. False otherwise
 * @exit
 *   ROC_NO_MAP_ENTRY - Mapper has no value for one of the queried destinations
 */
bool connect_to_destinations(WorldState* worldState, char* planeId) {
    bool failedToConnect = false;
    for (int i = 0; i < worldState->countAirports; ++i) {
        Airport* airport = worldState->airports[i];
        int server = outbound_socket_maker(airport->port);
        
        if (server == -1) {
            failedToConnect = true;
            airport->info = malloc(sizeof(char) * 80);
            strncpy(airport->info, "", 1);
        } else {
            // write to server
            int server2 = dup(server);
            FILE* writer = fdopen(server, "w");
            FILE* reader = fdopen(server2, "r");
            
            // write roc id
            fprintf(writer, "%s\n", planeId);
            fflush(writer);
            // get control id
            char input[80];
            if (fgets(input, 80, reader) != NULL) {
                airport->info = malloc(sizeof(char) * 80);
                strncpy(airport->info, input, 80);
            } else {
                roc_exit(ROC_NO_MAP_ENTRY);
            }
        }
    }
    return failedToConnect;
}

/** Connects the roc to the mapper and gets port numbers.
 *
 * @param worldState The roc program state
 * @param hasPort True if the program started with a mapper port
 * @param needMapper True if the program needs a mapper
 * @param mapperPortNum The mapper port number
 * @exit
 *   ROC_MAPPER_CONNECTION_ERROR - Error connecting to mapper
 *   ROC_NO_MAP_ENTRY - Mapper has no value for one of the queried destinations
 */
void roc_mapper_connect(WorldState* worldState, bool hasPort,
        bool needMapper, int mapperPortNum) {
    if (hasPort && needMapper) {
        int server = outbound_socket_maker(mapperPortNum);
        
        // if there was error connecting to mapper
        if (server == -1) {
            roc_exit(ROC_MAPPER_CONNECTION_ERROR);
        }
        // make file streams to write and read
        int server2 = dup(server);
        FILE* writer = fdopen(server, "w");
        FILE* reader = fdopen(server2, "r");
        
        for (int i = 0; i < worldState->countAirports; ++i) {
            Airport* airport = worldState->airports[i];
            // was given an id
            if (airport->port == 0) {
                char message[100];
                snprintf(message, 100, "?%s\n", airport->id);
                
                if (fprintf(writer, "%s", message) < 0) {
                    roc_exit(ROC_NO_MAP_ENTRY);
                }
                fflush(writer);
                
                // receive server response
                char input[80];
                if (fgets(input, 80, reader) != NULL) {
                    if (strcmp(input, ";\n") != 0) {
                        int port = (int) strtol(input, (char**) {0}, 10);
                        airport->port = port;
                    } else {
                        roc_exit(ROC_NO_MAP_ENTRY);
                    }
                    
                } else {
                    roc_exit(ROC_NO_MAP_ENTRY);
                }
            }
        }
    }
}

/** Extract destinations from args.
 *
 * @param worldState The roc program state
 * @param argc Program argument count
 * @param argv Program arguments
 * @param need_mapper True if the program needs a mapper
 * @return True if the program needs a mapper
 */
bool roc_find_destinations(WorldState* worldState, int argc, char** argv,
        bool need_mapper) {
    for (int i = 3; i < argc; ++i) {
        allocate_airports(worldState);
        worldState->countAirports++;
        // rest holds the string unconverted part of conversion
        char* rest;
        int result = (int) strtol(argv[i], &rest, 10);
        // note we use i - 3 because destinations start from arg 3
        Airport* airport = worldState->airports[i - 3];
        airport->id = malloc(sizeof(char) * 80);
        strncpy(airport->id, "", 2);
        // if was int
        if (result != 0 && strlen(rest) == 0) {
            airport->port = result;
        } else {
            strncpy(airport->id, argv[i], 80);
            // use port 0 to indicate no port was given
            airport->port = 0;
            need_mapper = true;
        }
    }
    
    return need_mapper;
}

/** Entry point to Roc program
 * @exit
 *   0 - Normal exit
 *   ROC_FAILED_TO_CONNECT - Failed to connect to at least one destination
 * **/
int main(int argc, char** argv) {
    // check args
    check_args(argc, argv);
    // get planeID
    char* planeID = argv[1];
    
    // get mapper port - either number or a dash
    char* mapperPort = argv[2];
    
    // has_port is true if a mapper port was inputted
    bool has_port = true;
    bool need_mapper = false;
    int mapperPortNum;
    if (strcmp(mapperPort, "-") == 0) {
        has_port = false;
    } else {
        mapperPortNum = (int) strtol(mapperPort, (char**) {0}, 10);
    }
    
    // initialize state
    WorldState* worldState = malloc(sizeof(WorldState));
    worldState->countAirports = 0;
    
    // find destinations
    need_mapper = roc_find_destinations(worldState, argc, argv, need_mapper);
    
    // connect to mapper and make streams
    roc_mapper_connect(worldState, has_port, need_mapper, mapperPortNum);
    
    // connect to each destination and get info
    bool failedToConnect = connect_to_destinations(worldState, planeID);
    
    print_log(worldState);
    
    if (failedToConnect) {
        roc_exit(ROC_FAILED_TO_CONNECT);
    }
    return 0;
}