#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 2048 
#define SERVER_IP "127.0.0.1"
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    int n, N;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    if (inet_aton(SERVER_IP, &servaddr.sin_addr) == 0)
    {
        fprintf(stderr, "Invalid IP address\n");
        close(sockfd);
        exit(1);
    }

    while (1)
    {
        printf("Enter N (number of records to request) or q to quit: ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            perror("Error reading input");
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "q") == 0)
        {
            break;
        }

        N = atoi(buffer);
        if (N <= 0)
        {
            printf("Invalid number. Please enter a positive integer.\n");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "%d", N);

        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("Error sending data");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
        if (n < 0)
        {
            perror("Error receiving data");
            break;
        }
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
        // Если сервер сообщил о нехватке данных
        if (strcmp(buffer, "No more data.\n") == 0)
        {
            break;
        }
    }

    close(sockfd);
    return 0;
}
