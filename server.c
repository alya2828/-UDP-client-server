#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h> 

#define MACADDR_SIZE 17

typedef struct Data
{
    int vlan_id;
    char mac_addr[17];
} Data;

typedef struct Data_List
{
    struct Data data;
    struct Data_List *next;
} Data_List;

typedef struct Client_info_list
{
    struct sockaddr_in client_addr;
    Data_List *last;
    struct Client_info_list *next;
} Client_info_list;

Client_info_list *search_client(Client_info_list *head, struct sockaddr_in client)
{
    Client_info_list *tmp = head;
    while (tmp != NULL)
    {
        printf("Searching client - Port: %d\n", ntohs(tmp->client_addr.sin_port));
        if (tmp->client_addr.sin_addr.s_addr == client.sin_addr.s_addr &&
            tmp->client_addr.sin_port == client.sin_port)
        {
            printf("Client found - Port: %d\n", ntohs(client.sin_port));
            return tmp;
        }
        tmp = tmp->next;
    }
    printf("Client not found.\n");
    return NULL;
}

Client_info_list *add_client(Client_info_list *head, struct sockaddr_in client)
{
    Client_info_list *newItem = (Client_info_list *)malloc(sizeof(Client_info_list));
    if (newItem == NULL)
    {
        printf("Memory allocation failed.\n");
        return head;
    }
    newItem->client_addr = client;
    newItem->last = NULL;
    newItem->next = NULL;

    if (head == NULL)
    {
        head = newItem;
    }
    else
    {
        Client_info_list *tmp = head;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = newItem;
    }

    printf("New client added - Port: %d\n", ntohs(client.sin_port));
    return head;
}

void Print(Data person)
{
    printf("VLANID: %d\n", person.vlan_id);
    printf("MACADDR: %s\n", person.mac_addr);
}

void PrintList(Data_List *head)
{
    Data_List *tmp = head;
    if (NULL == head)
    {
        printf("List empty!\n");
        return;
    }
    do
    {
        Print(tmp->data);
        tmp = tmp->next;
    } while (tmp != NULL);
}

Data_List *read_list(Data_List *head, int *record_count)
{

    FILE *file = fopen("data", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    Data_List *tmp = head;
    Data data;

    while (fscanf(file, "VILAN_id: %d, Mask: %17s\n", &data.vlan_id, data.mac_addr) != EOF)
    {
        Data_List *newNode = (Data_List *)malloc(sizeof(Data_List));
        if (newNode == NULL)
        {
            perror("Memory allocation failed");
            fclose(file);
            exit(1);
        }
        newNode->data = data;
        newNode->next = NULL;
        if (head == NULL)
        {
            // Если head пуст, инициализируем первый элемент
            head = newNode;
            tmp = head;
        }
        else
        {
            // Если head уже инициализирован, добавляем новый элемент
            tmp->next = newNode;
            tmp = newNode;
        }

        (*record_count)++; // Инкрементируем количество записей
    }

    fclose(file);
    return head;
}

void sort_list(Data_List *head)
{
    if (head == NULL)
        return;

    int swapped;
    Data_List *ptr1;
    Data_List *lptr = NULL;

    do
    {
        swapped = 0;
        ptr1 = head;

        while (ptr1->next != lptr)
        {
            if (ptr1->data.vlan_id > ptr1->next->data.vlan_id)
            {
                // Меняем данные узлов, если они не в порядке
                Data temp = ptr1->data;
                ptr1->data = ptr1->next->data;
                ptr1->next->data = temp;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

void FreeList(Data_List *head)
{
    Data_List *current = head;
    while (current != NULL)
    {
        Data_List *temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL; // Устанавливаем указатель на голову списка в NULL
}

char *prepare_data(Client_info_list *client, Data_List *list, int N)
{
    Data_List *current = client->last;

    if (current == NULL)
    {
        current = list; // Если клиент запрашивает данные впервые, начинаем с начала списка
    }

    char *buffer = (char *)malloc(N * (MACADDR_SIZE + 20));
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }
    buffer[0] = '\0';

    int count = 0;
    while (N > 0 && current != NULL)
    {
        char tmp[50];
        snprintf(tmp, sizeof(tmp), "VLANID: %d, MAC: %s\n", current->data.vlan_id, current->data.mac_addr);
        strcat(buffer, tmp);     // Добавляем строку к буферу
        current = current->next; // Переходим к следующему элементу
        N--;                     // Уменьшаем количество оставшихся записей для отправки
        count++;
    }

    client->last = current; // Обновляем позицию клиента в списке

    if (count == 0)
    {
        // Если данных нет для отправки, устанавливаем сообщение
        free(buffer); // Освобождаем память, так как нет данных
        return "No more data.\n";
    }

    return buffer;
}

int main()
{
    int record_count = 0;
    int sockfd;
    int n;
    char line[1000];
    int N;
    Data_List *head = NULL;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    /* Заполняем структуру для адреса сервера */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(51000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Создаем UDP сокет */
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror(NULL);
        exit(1);
    }
    /* Настраиваем адрес сокета */
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror(NULL);
        close(sockfd);
        exit(1);
    }
    // Чтение списка данных и сортировка
    head = read_list(head, &record_count);
    sort_list(head);

    Client_info_list *head_clients = NULL;
    while (1)
    {
        clilen = sizeof(cliaddr);
        if ((n = recvfrom(sockfd, line, 2, 0, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        {
            perror("recvfrom failed");
            close(sockfd);
            FreeList(head);
            exit(1);
        }
        line[n] = '\0';
        printf("Received: %s from client - Port: %d\n", line, ntohs(cliaddr.sin_port));

        N = atoi(line);
        if (N <= 0)
        {
            char *error_message = "Invalid number.\n";
            sendto(sockfd, error_message, strlen(error_message), 0, (struct sockaddr *)&cliaddr, clilen);
            continue;
        }
        if (N >= record_count)
        {
            char *error_number = "No more data.\n";
            sendto(sockfd, error_number, strlen(error_number), 0, (struct sockaddr *)&cliaddr, clilen);
            continue;
        }

        // Поиск клиента
        Client_info_list *client = search_client(head_clients, cliaddr);

        if (client == NULL)
        {
            printf("New client connected.\n");
            head_clients = add_client(head_clients, cliaddr);
            client = search_client(head_clients, cliaddr); // Теперь ищем клиента в списке
            client->last = head;                           // Устанавливаем начало списка для нового клиента
        }

        // Подготовка данных для отправки клиенту
        char *answer = prepare_data(client, head, N);
        if (answer == NULL)
        {
            perror("Error preparing data");
            continue;
        }

        /* Принятый текст отправляем обратно по адресу отправителя */
        if (sendto(sockfd, answer, strlen(answer), 0, (struct sockaddr *)&cliaddr, clilen) < 0)
        {
            perror(NULL);
            close(sockfd);
            exit(1);
        }

        printf("Data sent to client - Port: %d\n", ntohs(cliaddr.sin_port));
        free(answer);
    }

    return 0;
}