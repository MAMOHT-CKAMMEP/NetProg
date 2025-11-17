#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <system_error>

// Сетевые заголовки
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class DaytimeUDPClient {
private:
    int sockfd;
    std::string server_ip;
    int port;
    struct sockaddr_in server_addr;
    
public:
    // Конструктор
    DaytimeUDPClient(const std::string& ip = "172.16.40.1", int p = 13) 
        : sockfd(-1), server_ip(ip), port(p) {
        std::memset(&server_addr, 0, sizeof(server_addr));
    }
    
    // Деструктор
    ~DaytimeUDPClient() {
        closeSocket();
    }
    
    // Закрытие сокета
    void closeSocket() {
        if (sockfd != -1) {
            close(sockfd);
            sockfd = -1;
        }
    }
    
    // Инициализация клиента
    bool initialize() {
        // Создание UDP сокета
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            std::cerr << "Ошибка создания сокета: " << std::strerror(errno) << std::endl;
            return false;
        }
        
        // Настройка адреса сервера
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Неверный IP-адрес: " << server_ip << std::endl;
            closeSocket();
            return false;
        }
        
        return true;
    }
    
    // Получение времени от сервера
    bool getTime() {
        if (sockfd < 0) {
            std::cerr << "Сокет не инициализирован" << std::endl;
            return false;
        }
        
        // Таймаут на прием данных (5 секунд)
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            std::cerr << "Ошибка установки таймаута: " << std::strerror(errno) << std::endl;
        }
        
        // Отправка пустого запроса на сервер
        std::string request = "";
        socklen_t addr_len = sizeof(server_addr);
        
        ssize_t sent_bytes = sendto(sockfd, request.c_str(), request.length(), 0,
                                   (struct sockaddr*)&server_addr, addr_len);
        
        if (sent_bytes < 0) {
            std::cerr << "Ошибка отправки запроса: " << std::strerror(errno) << std::endl;
            return false;
        }
        
        
        // Буфер для приема данных
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        
        // Прием ответа от сервера
        ssize_t recv_bytes = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                     (struct sockaddr*)&server_addr, &addr_len);
        
        if (recv_bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Таймаут: сервер не ответил в течение 5 секунд" << std::endl;
            } else {
                std::cerr << "Ошибка приема данных: " << std::strerror(errno) << std::endl;
            }
            return false;
        }
        
        // Преобразование полученных данных в строку
        buffer[recv_bytes] = '\0';
        std::string time_response(buffer);
        
        // Вывод результата
        std::cout << "Текущее время: " << time_response;
        if (time_response.back() != '\n') {
            std::cout << std::endl; // Добавляем перевод строки если его нет
        }
        
        return true;
    }
    
    // Тестирование соединения
    void testConnection() {
        if (!initialize()) {
            std::cerr << "Не удалось инициализировать клиент" << std::endl;
            return;
        }
        
        if (!getTime()) {
            std::cerr << "Не удалось получить время от сервера" << std::endl;
        }
        
        closeSocket();
    }
};

// Функция для вывода справки
void printHelp() {
    std::cout << "Использование:" << std::endl;
    std::cout << "  ./daytime_client [IP] [PORT]" << std::endl;
    std::cout << "  ./daytime_client              - использовать сервер 172.16.40.1:13" << std::endl;
    std::cout << "  ./daytime_client <IP>         - использовать указанный IP и порт 13" << std::endl;
    std::cout << "  ./daytime_client <IP> <PORT>  - использовать указанные IP и PORT" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  ./daytime_client" << std::endl;
    std::cout << "  ./daytime_client 172.16.40.1" << std::endl;
    std::cout << "  ./daytime_client time.nist.gov 13" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string server_ip = "172.16.40.1";
    int port = 13;
    
    // Обработка аргументов командной строки
    if (argc == 2) {
        server_ip = argv[1];
    } else if (argc == 3) {
        server_ip = argv[1];
        port = std::atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Ошибка: неверный номер порта!" << std::endl;
            return 1;
        }
    } else if (argc > 3) {
        std::cerr << "Слишком много аргументов!" << std::endl;
        printHelp();
        return 1;
    }
    

    std::cout << "Сервер: " << server_ip << std::endl;
    std::cout << "Порт: " << port << std::endl;
    std::cout << "Протокол: UDP" << std::endl;

    
    try {
        DaytimeUDPClient client(server_ip, port);
        client.testConnection();
    } catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}