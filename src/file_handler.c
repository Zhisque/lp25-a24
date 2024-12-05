#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include "file_handler.h"
#include "deduplication.h"

// Fonction permettant de lire un élément du fichier .backup_log
log_t read_backup_log(const char *logfile){
    /* Implémenter la logique pour la lecture d'une ligne du fichier ".backup_log"
    * @param: logfile - le chemin vers le fichier .backup_log
    * @return: une structure log_t
    */
    log_t logs = {NULL, NULL};
    FILE *file = fopen(logfile, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier .backup_log");
        return logs;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        log_element *new_element = malloc(sizeof(log_element));
        if (!new_element) {
            perror("Failed to allocate memory");
            fclose(file);
            return logs;
        }

        char *token;
        token = strtok(line, ";")
        new_element->path = malloc(strlen(token) + 1);
        if (!new_element->path) {
            perror("Failed to allocate memory");
            fclose(file);
            return logs;
        }
        new_element->path = token;
        token = strtok(line, ";")
        new_element->date = malloc(strlen(token) + 1);
        if (!new_element->date) {
            perror("Failed to allocate memory");
            fclose(file);
            return logs;
        }
        new_element->date = token;
        new_element->md5 = line;

        new_element->next = NULL;
        new_element->prev = logs.tail;

        if (logs.tail) {
            logs.tail->next = new_element;
        } else {
            logs.head = new_element;
        }
        logs.tail = new_element;
    }

    fclose(file);
    return logs;
}

// Fonction permettant de mettre à jour une ligne du fichier .backup_log
void update_backup_log(const char *logfile, log_t *logs){
    /* Implémenter la logique de modification d'une ligne du fichier ".bakcup_log"
    * @param: logfile - le chemin vers le fichier .backup_log
    *         logs - qui est la liste de toutes les lignes du fichier .backup_log sauvegardée dans une structure log_t
    */
    FILE *file = fopen(logfile, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier .backup_log");
        return;
    }
    FILE *temp_file = fopen("temp_backup_log", "w");
    if (!temp_file) {
        perror("Erreur lors de la création du fichier temporaire");
        fclose(file);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char path[256];
        sscanf(line, "%255s", path);

        // Vérifie si ce chemin existe dans `logs`
        log_element *current = logs->head;
        int found = 0;
        while (current) {
            if (strcmp(path, current->path) == 0) {
                // Écrit l'élément mis à jour dans le fichier temporaire
                write_log_element(current, temp_file);
                found = 1;
                break;
            }
            current = current->next;
        }

        // Si l'élément n'est pas trouvé, il est ignoré (supprimé)
    }

    // Ajoute les nouveaux éléments qui n'étaient pas dans l'ancien log
    log_element *current = logs->head;
    while (current) {
        int found = 0;
        rewind(file); // Parcourt à nouveau l'ancien fichier
        while (fgets(line, sizeof(line), file)) {
            char path[256];
            sscanf(line, "%255s", path);

            if (strcmp(path, current->path) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            write_log_element(current, temp_file);
        }
        current = current->next;
    }

    fclose(file);
    fclose(temp_file);

    // Remplace l'ancien fichier par le fichier temporaire
    remove(logfile);
    rename("temp_backup_log", logfile);
}

void write_log_element(log_element *elt, FILE *logfile){
  /* Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param: elt - un élément log à écrire sur une ligne
   *         logfile - le chemin du fichier .backup_log
   */
}

void list_files(const char *path){
  /* Implémenter la logique pour lister les fichiers présents dans un répertoire
  */
}


