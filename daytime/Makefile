# Компилятор и флаги
CXX = g++
CXXFLAGS = -Wall -std=c++20 -g
LDFLAGS = 

# Имя целевого файла
TARGET = daytime_client

# Исходные файлы
SRC = daytime_client.cpp

# Сборка напрямую из исходников
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Очистка
clean:
	rm -f $(TARGET)

# Запуск
run: $(TARGET)
	./$(TARGET)

help:
	@echo "Доступные цели:"
	@echo "  make        - собрать клиента"
	@echo "  run         - запустить c ip: 172.16.40.1 port: 13"
	@echo "  clean       - удалить скомпилированные файлы"
	@echo "  help        - показать эту справку"
	@echo ""
	@echo "Использование клиента:"
	@echo "  ./daytime_client              - использовать сервер 172.16.40.1:13"
	@echo "  ./daytime_client <IP>         - использовать указанный IP и порт 13"
	@echo "  ./daytime_client <IP> <PORT>  - использовать указанные IP и PORT"
	@echo ""
	@echo "Примеры:"
	@echo "  ./daytime_client"
	@echo "  ./daytime_client 172.16.40.1"
	@echo "  ./daytime_client 172.16.40.1 13"

.PHONY: clean run help