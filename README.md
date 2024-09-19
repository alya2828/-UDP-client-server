# Задание: UDP клиент-сервер

- два приложения: клиент и сервер, взаимодействующие через UDP;
- сервер при старте читает файл, хранящий список VLAN-ID + MAC-ADDR в несортированном виде (каждая пара уникальна);
- сервер сортирует этот список и сохраняет его у себя в базе данных в оперативной памяти;
- сервер по запросу от клиента выдаёт N записей из начала базы данных, или следующие N записей от предыдущего запроса этого клиента;
- сервер должен уметь обрабатывать несколько клиентов одновременно, не блокируясь на обработке отдельного клиента, при этом должен быть однопоточным;
- клиент и сервер взаимодействуют по UDP;
- клиент запрашивает у сервера N записей из базы данных, выводит их на экран, ожидает нажатия <q> для завершения или <space> для запроса следующих N записей.

## Как использовать
1. **Компиляция программы:**
- make
2. **Запускаем сервер**
- ./server 
3. **Запускаем N-ое количество клиентов**
- ./сlient
4. **Удалить скомпилированные бинарные файлы**
- make clean
 ## Для справки можно найти запущенный процесс по команде и убить 
  - sudo lsof -i -P -n
  - sudo kill -9 "PID"
