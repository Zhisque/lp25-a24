#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */
    struct stat info;

    char str_date[20]; 
    struct timespec tp;
    time_t now = time(NULL);
    strftime(str_date, 20, "%Y-%m-%d-%H:%M:%S", localtime(&now));
    
    if (strlen(backup_dir) > 1011) {
        perror("Directory name is too long!");
        return;
    }

    char log_path[1024];
    strcpy(log_path, backup_dir);
    strcat(log_path, "/.backup_log");

    log_t existing_logs = read_backup_log(log_path), new_logs;
    new_logs.head = new_logs.tail = NULL;

    DIR *source = opendir(source_dir);
    if (source == NULL) {
        perror("Source directory does not exist");
        return;
    }

    struct dirent *file = readdir(source);
    while (file != NULL) {
        log_element *element = existing_logs.tail;
        while (element != NULL && strcmp(element->path, file->d_name)) {
            
            element = element->prev;
        }
        file = readdir(source);
    }

    update_backup_log(log_path, &new_logs);
}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    */
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    /*
    */
}


// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    */
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */
}
