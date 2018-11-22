#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8000

int main(int argc, char const *argv[])
{

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server\n";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Bind failed!\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen error!\n");
        exit(EXIT_FAILURE);
    }

    const char *header =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "Content-Length: 4096\n"
        "Accept-Ranges: bytes\n"
        "Connection: close\n"
        "\n";

    // Leer Archivo
    // declarar un puntero al archivo
    FILE *infile;
    char *data;
    long numbytes;

    // abrir un archivo existente para leerlo
    infile = fopen("index.html", "r");

    // finalizar si no existe
    if (infile == NULL)
        return 1;

    // obtener numero de bytes
    fseek(infile, 0L, SEEK_END);
    numbytes = ftell(infile);

    /* reset the file position indicator to 
    the beginning of the file */
    fseek(infile, 0L, SEEK_SET);

    /* grab sufficient memory for the 
    buffer to hold the text */
    data = (char *)calloc(numbytes, sizeof(char));

    /* memory error */
    if (data == NULL)
        return 1;

    /* copiar todo el texto en el buffer */
    fread(data, sizeof(char), numbytes, infile);
    fclose(infile);

    /* confirm we have read the file by
    outputing it to the console */
    printf("The file called test.dat contains this text\n\n%s", data);


    // unir header con data
    char * reply = (char *) malloc(1 + strlen(header)+ strlen(data));
    strcpy(reply, header);
    strcat(reply, data);
    printf("\n\n\n%s", reply);

    //for (;;)
    //{
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept!\n");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);

        send(new_socket, reply, strlen(reply), 0);

        /* free the memory we used for the buffer */
        //free(data);
   // }

    //"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n"
    //send(new_socket, header, strlen(header), 0);
    //char *header = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: text/html\r\n\r\n";
    //send(new_socket, header, strlen(header), 0);
    //send(new_socket, hello, strlen(hello), 0);

    //send(new_socket, reply, strlen(reply), 0);
    // send(new_socket, hello, strlen(hello), 0);
    //printf("Hello message sent\n");

    return 0;
}
