#include "deduplication.h"

#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <dirent.h>


unsigned int hash_md5(unsigned char *md5) {
    unsigned int hash = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash = (hash << 5) + hash + md5[i];
    }
    return hash % HASH_TABLE_SIZE;
}


void compute_md5(void *data, size_t len, unsigned char *md5_out) {
    MD5_CTX md5; // creer
    MD5_Init(&md5); // initialiser struct
    MD5_Update(&md5, data, len); // inserer donnee
    MD5_Final(md5_out, &md5); // Finito pipo
    // ou ? MD5((unsigned char *)data, len, md5_out);
}


int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        if (memcmp(hash_table[i].md5, md5, MD5_DIGEST_LENGTH) == 0) {
            return hash_table[i].index;
        }
    }
    return -1;
}


void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    unsigned int indice = hash_md5(md5);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        unsigned int try = (indice + i) % HASH_TABLE_SIZE; // % -> modulo pour eviter toutes sorties
        if (hash_table[try].index == -1) { // reste de la place ?
            memcpy(hash_table[try].md5, md5, MD5_DIGEST_LENGTH);
            hash_table[try].index = index;
            return;
        }
    }
    fprintf(stderr, "La table est complete !\n");
}


void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    unsigned char buffer[CHUNK_SIZE];
    int chunk_index = 0, j = 0;
    size_t lecture_chunk;

    // Initialiser la table de hachage avec des -1
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].index = -1;
    }

    while ((lecture_chunk = fread(buffer, sizeof(unsigned long), CHUNK_SIZE, file)) > 0) {

        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, lecture_chunk, md5);

        while (j < chunk_index && memcmp(chunks[j].md5, md5, MD5_DIGEST_LENGTH) != 0){
            j++;
        }
        if (memcmp(chunks[j].md5, md5, MD5_DIGEST_LENGTH) == 0) { // il existe deja !
            // ajout dans table de hashage
            add_md5(hash_table, md5, j);

            chunk_index++;
        }
        else{
            // nouveau dans table de chunks unique
            chunks[chunk_index].data = malloc(lecture_chunk);
            memcpy(chunks[chunk_index].data, buffer, lecture_chunk);
            memcpy(chunks[chunk_index].md5, md5, MD5_DIGEST_LENGTH);

            // nouveau dans table de hashage
            add_md5(hash_table, md5, chunk_index);

            chunk_index++;
        }
    }
    if (feof(file)){
        return; // fin fichier valide
    }
    else if (ferror(file)){
        printf("probleme dans la lecture du fichier");
        return;
    }
}


// Fonction permettant de charger un fichier dédupliqué en table de chunks
// en remplaçant les références par les données correspondantes


/* @param: file est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
    *           chunks représente le tableau de chunk qui contiendra les chunks restauré depuis filename
    *           chunk_count est un compteur du nombre de chunk restauré depuis le fichier filename
*/


void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    *chunk_count = 0;
    unsigned char md5[MD5_DIGEST_LENGTH];
    size_t lecture_chunk;

    //compte du nombre de chunks unique
    while ((lecture_chunk = fread(md5, sizeof(unsigned char), MD5_DIGEST_LENGTH, file)) > 0) {

        // Incrémenter le compteur de chunks
        *chunk_count = *chunk_count + 1;

    } //                        ----verif----
    if (feof(file)){
        return;
    }
    else if (ferror(file)){
        printf("probleme dans la lecture du fichier");
        return;
    }

    fseek(file, 0, SEEK_SET); // ce positionner au debut du fichier

    *chunks = malloc(*chunk_count * sizeof(Chunk));

    for (int i = 0; i < *chunk_count; i++) {
        fread((*chunks)[i].md5, 1, MD5_DIGEST_LENGTH, file);

        size_t data_size; // lire taille donnees
        fread(&data_size, sizeof(size_t), 1, file);


        (*chunks)[i].data = malloc(data_size);
        fread((*chunks)[i].data, 1, data_size, file);
    }
}
