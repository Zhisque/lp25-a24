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
        perror("Directory name is too long!\n");
        return;
    }

    char log_path[1024];
    strcpy(log_path, backup_dir);
    strcat(log_path, "/.backup_log");

    log_t existing_logs = read_backup_log(log_path), new_logs;
    new_logs.head = new_logs.tail = NULL;

    DIR *source = opendir(source_dir);
    if (source == NULL) {
        perror("Source directory does not exist\n");
        return;
    }

    struct dirent *file = readdir(source);
    while (file != NULL) {
        log_element *element = existing_logs.tail;
        while (element != NULL && strcmp(element->path, file->d_name)) {
            element = element->prev;
        }
        if (element == NULL) {
            //Fichier non existant dans la cible
        } /*else if (element->date) {

        }   Pas fini :(    */


        file = readdir(source);
    }

    update_backup_log(log_path, &new_logs);
}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening file %s\n", output_filename);
        return;
    }

    for (int i = 0; i < chunk_count; ++i) {
        if(fwrite(chunks[i].md5, sizeof(char), MD5_DIGEST_LENGTH, output_file) != 1) {
            perror("Failed to write MD5 sum");
            return;
        }
        if (fwrite(chunks[i].data, sizeof(void), CHUNK_SIZE, output_file) != 1) {
            perror("Failed to write data chunk");
            return;
        }
    }
    fclose(output_file);
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file starting backup");
        return;
    }
    Md5Entry *hash_table = NULL;
    Chunk *chunks = NULL;
    deduplicate_file(file, chunks, hash_table);

    char output_filename[1024];//D'où vient le nom de l'output ???
    write_backup_file(output_filename, chunks, 0);//Comment avoir le nombre ? Quand s'arrête la hash_table ?

    fclose(file);
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
