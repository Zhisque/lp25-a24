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
    MD5((unsigned char *)data, len, md5_out);
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


int deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    unsigned char buffer[CHUNK_SIZE];
    int chunk_index = 0, j = 0;
    int nbr_chunk = 0;
    size_t lecture_chunk;

    // Initialiser la table de hachage avec des -1
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].index = -1;
    }

    while ((lecture_chunk = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
        nbr_chunk++; // calculer le nombre de chunks

        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, lecture_chunk, md5);

        while (j < chunk_index && memcmp(chunks[j].md5, md5, MD5_DIGEST_LENGTH) != 0){
            j++; //parcourir pour trouver chunk non unique
        }
        if (memcmp(chunks[j].md5, md5, MD5_DIGEST_LENGTH) == 0) { // il existe deja !
            // ajout dans table de hashage
            add_md5(hash_table, md5, j);
        } else {
            // nouveau dans table de chunks unique
            chunks = realloc(chunks, sizeof(Chunk) * (chunk_index + 1));
            chunks[chunk_index].data = malloc(lecture_chunk);
            memcpy(chunks[chunk_index].data, buffer, lecture_chunk);
            memcpy(chunks[chunk_index].md5, md5, MD5_DIGEST_LENGTH);

            // nouveau dans table de hashage
            add_md5(hash_table, md5, j);

            chunk_index++;
        }
    }
    if (feof(file)){
        return nbr_chunk; // fin fichier valide
    }
    else if (ferror(file)){
        printf("probleme dans la lecture du fichier");
        return -1;
    }
    return nbr_chunk;
}


void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    Md5Entry hash_table[HASH_TABLE_SIZE]; // creation d une nouvelle table de hashage

    //compte du nombre de chunks unique
    fread(chunk_count, sizeof(int), 1, file);

    *chunks = malloc(*chunk_count * sizeof(Chunk));

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].index = -1; // Initialiser les espaces vide avec -1
    }

    int current_chunk_index = 0; // Index pour remplir le tableau chunks
    for (int i = 0; i < *chunk_count; i++) {
        unsigned char md5[MD5_DIGEST_LENGTH];
        fread(md5, sizeof(unsigned char), MD5_DIGEST_LENGTH, file); // Lire le MD5

        // Vérifier si le MD5 est déjà dans la table de hashage
        int md5_chunk_index = find_md5(hash_table, md5); // Trouve l'index si le MD5 exist sinon -1

        if (md5_chunk_index != -1) {
            (*chunks)[i] = (*chunks)[md5_chunk_index]; // Copier la référence du chunk unique
        } else {
            // Chunk unique -> lire les donnees

            (*chunks)[i].data = malloc(CHUNK_SIZE);
            fread((*chunks)[i].data, 1, CHUNK_SIZE, file);
            memcpy((*chunks)[i].md5, md5, MD5_DIGEST_LENGTH);

            // Ajouter ce nouveau chunk à la table de hachage
            add_md5(hash_table, md5, current_chunk_index);

            current_chunk_index++;
        }
    }
}
