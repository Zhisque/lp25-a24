#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_handler.h"
#include "deduplication.h"
#include "backup_manager.h"
#include "network.h"

int main(int argc, char *argv[]) {
    // Analyse des arguments de la ligne de commande

    // Implémentation de la logique de sauvegarde et restauration
    // Exemples : gestion des options --backup, --restore, etc.
    char mode = ' ', source[1024] = "", dest[1024] = "", source_serv[1024], dest_serv[1024];
	int verbose = 0, source_serv_port = -1, dest_serv_port = -1;

    int opt = 0;
	struct option my_opts[] = {
		{.name="backup",.has_arg=0,.flag=0,.val='b'},
		{.name="restore",.has_arg=0,.flag=0,.val='r'},
		{.name="dry-run",.has_arg=0,.flag=0,.val='u'},
        {.name="d-server",.has_arg=1,.flag=0,.val='w'},
        {.name="d-port",.has_arg=1,.flag=0,.val='x'},
        {.name="s-server",.has_arg=1,.flag=0,.val='y'},
        {.name="s-port",.has_arg=1,.flag=0,.val='z'},
        {.name="dest",.has_arg=1,.flag=0,.val='d'},
        {.name="source",.has_arg=1,.flag=0,.val='s'},
        {.name="verbose",.has_arg=0,.flag=0,.val='v'},
		{.name=0,.has_arg=0,.flag=0,.val=0}, // last element must be zero
	};
	while((opt = getopt_long(argc, argv, "", my_opts, NULL)) != -1) {
		switch (opt) {
			case 'b':
            case 'r':
			case 'u':
				if (mode == ' ') { 
                    mode = opt;
                } else { 
                    perror("Cannot use different modes");
                    return EXIT_FAILURE;
                }
				break;
            case 'w':
				strcpy(dest_serv, optarg);
				break;
            case 'x':
				sscanf(optarg, "%d", &dest_serv_port);
				break;
            case 'y':
				strcpy(source_serv, optarg);
				break;
            case 'z':
				sscanf(optarg, "%d", &source_serv_port);
				break;
            case 'd':
				strcpy(dest, optarg);
				break;
            case 's':
				strcpy(source, optarg);
				break;
            case 'v':
				verbose = 1;
				break;
			default:
				perror("Invalid argument");
				return EXIT_FAILURE;
		}
	}
	
	if (source_serv[0] == '\0' ^ source_serv_port == -1) {
		perror("Must give both server adress and port for source");
		return EXIT_FAILURE;
	}
	if (dest_serv[0] == '\0' ^ dest_serv_port == -1) {
		perror("Must give both server adress and port for destination");
		return EXIT_FAILURE;
	}

	//A continuer

    return EXIT_SUCCESS;
}