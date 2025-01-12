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
void create_backup(char *source_dir, char *backup_dir) {
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

    char backup_path[1024];
    strcpy(backup_path, backup_dir);
    strcat(backup_path, "/");
    strcat(backup_path, str_date);

    backup_file(source_dir, backup_path, &existing_logs, &new_logs);

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
void backup_file(const char *filename, const char *output_filename, log_t *old_logs, log_t *new_logs) {
    if (filename == NULL) {
        perror("Invalid argument for filename");
        return;
    }
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) {
        perror("Stat failed, file does not exist");
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        printf("Backing up directory %s into %s\n", filename, output_filename);
        mkdir(output_filename, 755);

        //Ajouter mtime pour l'instant ça marche pas
        unsigned char *zero = "0";
        ajout_log(new_logs, filename, zero, (size_t) file_stat.st_size, "");
        printf("Directory successfully added to logs\n");

        DIR *dir = opendir(filename);
        struct dirent *subfile = readdir(dir);
        char subfilename[1024], output_subfilename[1024];
        while (subfile != NULL) {
            if (strcmp(subfile->d_name, ".") == 0 || strcmp(subfile->d_name, "..") == 0) {
                subfile = readdir(dir);
                continue;
            }
            printf("Prepping for %s\n", subfile->d_name);
            strcpy(subfilename, filename);
            strcpy(output_subfilename, output_filename);
            strcat(subfilename, "/");
            strcat(output_subfilename, "/");
            strcat(subfilename, subfile->d_name);
            strcat(output_subfilename, subfile->d_name);

            printf("Prep finished\n");
            backup_file(subfilename, output_subfilename, old_logs, new_logs);

            subfile = readdir(dir);
        }
        printf("Directory %s finished\n", filename);
        closedir(dir);
    } else if (S_ISREG(file_stat.st_mode)) {
        printf("Backing up file %s into %s\n", filename, output_filename);
        //Ouvrir le fichier
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Error opening file starting backup");
            return;
        }
        
        //Chercher le fichier dans les anciens logs
        int is_same = 1;
        log_element *element = old_logs->head;
        while (element != NULL && strcmp(&element->path[FIRST_SLASH], filename)) {
            printf("%s\n", &element->path[FIRST_SLASH]);
            element = element->next;
        }
        if (element != NULL) printf("%s\n", &element->path[FIRST_SLASH]);
        else printf("empty\n");

        //Chercher les différences entre source et log
        
        Chunk *chunks = NULL;
        Md5Entry hash_table[HASH_TABLE_SIZE];
        int chunk_count = 0;
        if (element != NULL && element->date == file_stat.st_mtime && element->size == (size_t) file_stat.st_size) {
            //Vérifier les Chunks
            chunk_count = deduplicate_file(file, chunks, hash_table);
            for (int i = 0; i < chunk_count; ++i) {
                log_element *elementMD5 = old_logs->head;
                while (elementMD5 != NULL) {
                    if (strcmp(&elementMD5->path[FIRST_SLASH], filename) == 0 && strcmp(&elementMD5->md5, hash_table[i].md5) == 0) {
                        is_same = 0;
                        break;
                    }
                    elementMD5 = elementMD5->next;
                }
            }
        } else {
            is_same = 0;
            chunk_count = deduplicate_file(file, chunks, hash_table);
        }
        printf("SAME: %d", is_same);
        
        if (is_same) {
            ajout_log(new_logs, element->path, element->md5, element->size, element->date);
        } else {
            write_backup_file(output_filename, chunks, hash_table, chunk_count);
            ajout_log(new_logs, output_filename, element->md5, element->size, element->date);
        }
        
        //deduplicate_file(file, chunks, hash_table);

        //write_backup_file(output_filename, chunks, hash_table, sizeof(chunks));
        
        fclose(file);
    } else {
        printf("Unknown file type for %s", filename);
    }
}

// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
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
        if (fwrite(&chunks[i], sizeof(void), CHUNK_SIZE, output_file) != 1) {
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
            
            strcat(dir_path, &elem->path[FIRST_SLASH]);
            mkdir(dir_path, 755);
        } else {
            char backup_filepath[1024];
            strcpy(backup_filepath, backup_id);
            strcat(backup_filepath, "/");
            strcat(backup_filepath, elem->path);

            FILE *backup_file = fopen(backup_filepath, "rb");
            Chunk *chunks = NULL;
            int chunk_count = 0;
            undeduplicate_file(backup_file, &chunks, &chunk_count);

            char output_filepath[1024];
            strcpy(output_filepath, backup_id);
            strcat(output_filepath, &elem->path[FIRST_SLASH]);
            write_restored_file(output_filepath, chunks, chunk_count);
        }
        elem = elem->next;
    }
}

// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir) {
    struct dirent *entry;
    DIR *directory = opendir(backup_dir);

    if (directory == NULL) {
        perror("Unable to open directory");
        return;
    }

    printf("Existing backups in %s :\n", backup_dir);
    while ((entry = readdir(directory)) != NULL) {
        // Exclude ".", "..", and ".logfile"
        if (entry->d_name[0] == '.') {
            continue;
        }
        printf("%s\n", entry->d_name);
    }

    closedir(directory);
}