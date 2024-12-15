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
    
    if (strlen(backup_dir) > 1000) {
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
        struct stat file_stat;
        if (stat(file->d_name, &file_stat) != 0) {
            perror("Stat failed, probable file does not exist");
            continue;
        }

        log_element *element = existing_logs.head;
        while (element != NULL && strcmp(element->path, file->d_name)) {
            element = element->next;
        }
        
        if (S_ISREG(file_stat.st_mode)) {
            if (element == NULL) {
                //Fichier non existant dans la cible
                log_element *new_element = malloc(sizeof(log_element));
                new_element->path = malloc(sizeof(file->d_name+1));
                strcpy(new_element->path, file->d_name);
                //Pour chaque md5 faire un record mais il faut dédupliquer là ?????
                //strcpy(new_element->md5, );
            } else if (element->date == file_stat.st_mtime && element->size == file_stat.st_size) {
                //Fichier de même date et même taille...
                //Mais après il faudrait vérifier chaque élément...
            }
        }


        file = readdir(source);
    }

    update_backup_log(log_path, &new_logs);
}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, Md5Entry *hash_table, int chunk_count) {
    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        perror("Error opening writing file\n");
        return;
    }

    if(fwrite(chunk_count, sizeof(int), 1, output_file) != 1) {
        perror("Failed to write chunk number");
        return;
    }

    for (int i = 0; i < chunk_count; ++i) {
        if(fwrite(hash_table[i].md5, sizeof(char), MD5_DIGEST_LENGTH, output_file) != 1) {
            perror("Failed to write MD5 sum");
            return;
        }
        if (find_md5(hash_table, hash_table[i].md5) == i) {
            if (fwrite(chunks[hash_table[i].index].data, sizeof(void), CHUNK_SIZE, output_file) != 1) {
                perror("Failed to write data chunk");
                return;
            }
        }
    }
    fclose(output_file);
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename, const char *output_filename) {
    if (filename == NULL) {
        perror("Invalid arguments");
        return;
    }
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        perror("Stat failed, file does not exist");
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        DIR *file = opendir(filename);
        struct dirent *subfile = readdir(file);
        char subfilename[1024], output_subfilename[1024];
        while (subfile != NULL) {
            strcpy(subfilename, filename);
            strcpy(output_subfilename, output_filename);
            strcat(subfilename, "/");
            strcat(output_subfilename, "/");
            strcat(subfilename, subfile->d_name);
            strcat(output_subfilename, subfile->d_name);

            backup_file(subfilename, output_subfilename);
            readdir(subfile);
        }
        closedir(file);
    } else if (S_ISREG(file_stat.st_mode)) {
        FILE *file = fopen(filename, "r");
        if (!file) {
            perror("Error opening file starting backup");
            return;
        }
        Md5Entry *hash_table = NULL;
        Chunk *chunks = NULL;
        deduplicate_file(file, chunks, hash_table);

        write_backup_file(output_filename, chunks, hash_table, sizeof(chunks));

        fclose(file);
    } else {
        perror("Unknown file type");
    }
}

// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, Md5Entry *hash_table, int chunk_count) {
    if (output_filename == NULL || chunks == NULL || chunk_count == 0) {
        perror("Invalid arguments");
        return;
    }
    
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Error opening file restoring backup");
        return;
    }

    for (int i = 0; i < chunk_count; ++i) {
        if (fwrite(&chunks[hash_table[i].index], sizeof(void), CHUNK_SIZE, output_file) != 1) {
            perror("Error writing file during restoration");
            fclose(output_file);
            return;
        }
    }
    fclose(output_file);
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */
    //Lecture logfile
    if (backup_id == NULL || restore_dir == NULL) {
        perror("Invalid arguments");
        return;
    }
    
    char log_path[1024];
    strcpy(log_path, backup_id);
    strcat(log_path, "/.backup_log");

    log_t logs = {NULL, NULL};
    logs = read_backup_log(log_path);

    log_element *elem = logs.head;
    while (elem != NULL) {
        if (elem->md5 == 0) {
            //Convention pour un dossier
            char dir_path[1024];
            strcpy(dir_path, restore_dir);
            //YYYY-MM-DD-hh:mm:ss -> 19         YYYY-MM-DD-hh:mm:ss.sss -> 23
            strcat(dir_path, &elem->path[19]);
            mkdir(dir_path, 755);
        } else {
            //Enoooooorme flemme D:
        }
        elem = elem->next;
    }
}
