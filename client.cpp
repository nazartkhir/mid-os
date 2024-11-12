#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char* filename = argv[1];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Cant create the socket" << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);


    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    send(sock, filename, strlen(filename), 0);

    uint64_t file_size;
    read(sock, &file_size, sizeof(file_size));
    std::cout << "File size: " << file_size << " bytes" << std::endl;

    char buffer[BUFFER_SIZE];
    uint64_t total_received = 0;
    while (total_received < file_size) {
        ssize_t bytes_received = read(sock, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) break;
        std::cout << std::string(buffer, bytes_received);
        total_received += bytes_received;
    }

    close(sock);
    return 0;
}
