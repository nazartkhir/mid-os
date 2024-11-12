#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int serv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_socket == 0) {
        std::cerr << "Cant create the socket" << std::endl;
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(serv_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Cant bind" << std::endl;
        close(serv_socket);
        return 1;
    }

    listen(serv_socket, 5);
    std::cout << "Waiting for connections" << std::endl;

    sockaddr_in new_addr;
    socklen_t new_addr_size = sizeof(new_addr);

    int cli_socket = accept(serv_socket, (sockaddr *)&new_addr, &new_addr_size);
    if (cli_socket < 0) {
        std::cerr << "Cant accept" << std::endl;
        close(serv_socket);
        return 1;
    }

    char filename[BUFFER_SIZE];
    int bytes_read = read(cli_socket, filename, BUFFER_SIZE);
    if (bytes_read < 0) {
        std::cerr << "Failed to read" << std::endl;
        close(cli_socket);
        close(serv_socket);
        return 1;
    }

    filename[bytes_read] = '\0';

    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        std::cerr << "File not found" << std::endl;
        const char *error_msg = "File not found";
        write(serv_socket, error_msg, strlen(error_msg));
        close(cli_socket);
        close(serv_socket);
        return 1;
    }

    uint64_t file_size = file_stat.st_size;

    write(cli_socket, &file_size, sizeof(file_size));

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        close(cli_socket);
        close(serv_socket);
        return 1;
    }

    char file_buffer[BUFFER_SIZE];
    while (file.read(file_buffer, BUFFER_SIZE) || file.gcount() > 0) {
        if (write(cli_socket, file_buffer, file.gcount()) < 0) {
            std::cerr << "Failed to send" << std::endl;
            file.close();
            close(cli_socket);
            close(serv_socket);
            return 1;
        }
    }

    file.close();
    close(cli_socket);
    close(serv_socket);

    return 0;
}
