#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

//YYYY-MM-DD-hh:mm:ss -> 19         YYYY-MM-DD-hh:mm:ss.sss -> 23
#define FIRST_SLASH 19

// Renvoie l'heure actuelle sous format YYYY-MM-DD-hh:mm:ss
void get_str_time(char *buffer);
// Fonction pour créer un nouveau backup incrémental
void create_backup(char *source_dir, char *backup_dir);
// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir);
// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_backup_file(const char *output_filename, Chunk *chunks, Md5Entry *hash_table, int chunk_count);
// Fonction pour la sauvegarde de fichier dédupliqué
void backup_file(const char *filename, const char *output_filename);
// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count);
// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir);

#endif // BACKUP_MANAGER_H
