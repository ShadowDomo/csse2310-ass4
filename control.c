#include "mapper.h"

/** Perform error checking on Control program's arguments.
 *
 * @param argc Control program's argument count
 * @param argv Control program's arguments
 * @exit
 *    CTRL_INCORRECT_NUM_ARGS - incorrect number of args
 *    CTRL_INVALID_CHARS - Invalid characters in ID or info
 *    CTRL_INVALID_PORT - Invalid port
 */
void check_control_args(int argc, char** argv) {
    // incorrect number of args
    if (argc != 3 && argc != 4) {
        control_exit(CTRL_INCORRECT_NUM_ARGS);
        return;
    }

    // invalid characters in id or info
    char needles[] = "\r\n:";
    for (int i = 0; i < strlen(needles); ++i) {
        if (strchr(argv[1], needles[i]) != NULL) {
            control_exit(CTRL_INVALID_CHARS);
        }
        if (strchr(argv[2], needles[i]) != NULL) {
            control_exit(CTRL_INVALID_CHARS);
        }
    }

    // invalid port
    if (argc == 4) {
        char* otherHalf;
        int port = (int) strtol(argv[3], &otherHalf, 10);

        if (strlen(otherHalf) != 0) {
            control_exit(CTRL_INVALID_PORT);
            return;
        }

        if (port <= 0 || port > 65535) {
            control_exit(CTRL_INVALID_PORT);
            return;
        }
    }
}

/** Entry point to program. **/
int main(int argc, char** argv) {
    check_control_args(argc, argv);
    ControlState* controlState = malloc(sizeof(ControlState));
    controlState->countPlanes = 0;

    // set airport id
    controlState->airportId = (char*) malloc(sizeof(char) * 80);
    controlState->airportId = argv[1];

    // set airport info
    controlState->airportInfo = (char*) malloc(sizeof(char) * 80);
    controlState->airportInfo = argv[2];

    // mapper port of -1 means none was given
    controlState->mapperPort = -1;

    // set mapper port if exists
    if (argc == 4) {
        char* trash;
        controlState->mapperPort = (int) strtol(argv[3], &trash, 10);
    }
    setup_sockets(0, controlState);

    return 0;
}
