# Process Monitor
Консольное кроссплатформенное приложение для мониторинга процессов по имени. Отслеживает потребление CPU и памяти (VmRSS) по каждому PID, выводит информацию при изменениях, а также отслеживает исчезновение процессов. Вся информация берется из ``/proc/{id}/*``. Программа отрисовывает окно с информацией о просцессах при изменениях CPU больше чем на 5% или памяти больше чем на 5 МБ. Так же, программа отрисовывает изменения когда теряется один из процесов.

# Запуск
Пример запуска:
``./build/TaskManager Telegram zoom`` 
# Тесты

Тесты находятся в test/ и реализованы через assertы.

## Комманда для сборки:
```
cmake -B build
cmake --build build --target run_tests
```

## Тестовое задание для сигнатека. Код проверку не прошел. Фидбек выудить не удалось
