#include "network.h"
#include <stdlib.h>

void send_data(const char *server_address, int port, const void *data, size_t size) {
    // Implémenter la logique d'envoi de données à un serveur distant
    int sock = 0;
    struct sockaddr_in server_addr;

    // Création de la socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur lors de la création de la socket");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Conversion de l'adresse IP en format binaire
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Adresse IP invalide ou non supportée");
        close(sock);
        return;
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Échec de la connexion au serveur");
        close(sock);
        return;
    }

    // Envoi des données
    if (send(sock, data, size, 0) < 0) {
        perror("Erreur lors de l'envoi des données");
    } else {
        printf("Données envoyées avec succès.\n");
    }

    // Fermeture de la socket
    close(sock);
}

void receive_data(int port, void **data, size_t *size) {
    // Implémenter la logique de réception de données depuis un serveur distant
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024] = {0};

    // Création de la socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur lors de la création de la socket serveur");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Liaison de la socket au port donné
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(server_sock);
        return;
    }

    // Mise en écoute de la socket
    if (listen(server_sock, 5) < 0) {
        perror("Erreur lors de la mise en écoute de la socket");
        close(server_sock);
        return;
    }

    printf("En attente de connexions sur le port %d...\n", port);

    // Acceptation d'une connexion entrante
    if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        close(server_sock);
        return;
    }

    printf("Client connecté.\n");

    // Réception des données
    int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Erreur lors de la réception des données");
    } else {
        printf("Données reçues : %s\n", buffer);
    }

    // Fermeture des sockets
    close(client_sock);
    close(server_sock);
}

