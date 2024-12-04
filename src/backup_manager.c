#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

void get_str_time(char *buffer) {
    struct timespec tp;
    time_t now = time(NULL);
    strftime(buffer, 20, "%Y-%m-%d-%H:%M:%S", localtime(&now));
}

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */
    struct stat info;
    if (!is_directory(source_dir)) {
        perror("Le répertoire source est invalide");
        return;
    } else if (!is_directory(backup_dir)) {
        perror("Le répertoire de sauvegarde est invalide");
        return;
    }

    char str_date[20]; 
    get_str_time(str_date);
    
    char *log_path = malloc(strlen(backup_dir) + strlen(".backup_log") + 1);
    log_path = strcat(strcpy(log_path, backup_dir),".backup_log");

    log_t logs;

    update_backup_log(log_path, logs);
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
