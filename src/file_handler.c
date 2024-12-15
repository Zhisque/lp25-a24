#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
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
            perror("Echec lors de l'allocution de memoire");
            fclose(file);
            return logs;
        }

        char *token;
        token = strtok(line, ";");
        new_element->path = malloc(strlen(token) + 1);
        if (!new_element->path) {
            perror("Echec lors de l'allocution de memoire");
            fclose(file);
            return logs;
        }
        new_element->path = token;

        token = strtok(line, ";");
        sscanf(token,"%zu",new_element->taille);
        
        token = strtok(line, ";");
        new_element->date = malloc(strlen(token) + 1);
        if (!new_element->date) {
            perror("Echec lors de l'allocution de memoire");
            fclose(file);
            return logs;
        }
        new_element->date = token;

        strcpy(new_element->md5, line);


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
    FILE *file = fopen(logfile, "w");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier .backup_log");
        return logs;
    }

    log_element *current = logs->head;
    while (current) {
        write_log_element(current, file);
        current = current->next;
    }

    fclose(file);    
}

void write_log_element(log_element *elt, FILE *logfile){
  /* Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param: elt - un élément log à écrire sur une ligne
   *         logfile - le chemin du fichier .backup_log
   */
    if (!elt || !logfile) {
        perror("Paramètres invalides pour write_log_element");
        return;
    }

    // Écrit l'élément dans le fichier
    fprintf(logfile, "%s;%s;%s;%s\n", elt->path, elt->taille, elt->date, elt->md5);
}

void list_files(const char *path) {
    /* Implémenter la logique pour lister les fichiers présents dans un répertoire
    */
    DIR *dir = opendir(path);
    struct dirent *elem;
    if (!dir) {
        perror("Erreur lors de l'ouverture du dossier");
    }

    while ((elem=readdir(dir)) != NULL) {
        if (strcmp(elem->d_name, ".") == 0 || strcmp(elem->d_name, "..") == 0) {
            continue;
        }

        // Construire le chemin complet
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, elem->d_name);

        // Vérifier si c'est un fichier ou un répertoire
        struct stat path_stat;
        if (stat(full_path, &path_stat) == -1) {
            perror("Erreur lors de la récupération des informations sur le fichier");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            // Si c'est un répertoire, appeler récursivement
            printf("Chemin : %s\n", full_path);
            list_files(full_path);
        }
        else {
            // Si c'est un fichier, l'afficher
            printf("Fichier : %s\n", full_path);
        }
    }

    // Fermer le répertoire
    closedir(dir);
}

void ajout_log(log_t *log, const char *path, unsigned char *md5, size_t taille, char *date){
    log_element new_elem = malloc(sizeof(log_element));
    if (!log_element) {
        perror("Erreur lors de l'ajout d'un nouvel element");
        return;
    }
    new_elem->path = malloc(sizeof(const char));
    strcpy(new_elem->path,path);
    new_elem->md5 = malloc(sizeof(unsigned char));
    strcpy(new_elem->md5,md5);
    new_elem->taille = taille;
    new_elem->date = malloc(sizeof(char));
    strcpy(new_elem->date,date);
    new_elem->next = NULL;
    new_elem->prev = log->tail;
    (log->tail)->next = new_elem;
    log_tail = new_elem;
}