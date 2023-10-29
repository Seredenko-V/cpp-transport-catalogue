# Транспортный справочник
Учебный проект в рамках [курса](https://practicum.yandex.ru/cpp/?from=catalog) Яндекс Практикума.  
Используются:
* JSON
* SVG
* Бинарная сериализация и десериализация с использованием [Protobuf](https://github.com/protocolbuffers/protobuf)

## Формат взаимодействия
Взаимодействие с проектом осуществляется при помощи запросов и ответов в формате JSON.

### Запросы
Поддерживаются два основных вида запросов, которые указываются в качестве параметра командной 
строки (аргумента `int main`):  
1. **`make_base`** - формирование базы данных. Запрос может содержать следующую информацию:
    * `base_requests` - массив с описанием автобусных маршрутов и остановок. Перечисление осуществляется в произвольном порядке.
        - При описании **остановки** указываются: 
            + `type` - тип запроса, указывается `Stop`;
            + `name` - название остановки;
            + `latitude` - широта;
            + `longitude` - долгота;
            + `road_distances` - словарь расстояний до соседних остановок, в котором ключ - название соседнией остановки, 
            а значение - расстояние в метрах между текущей и указанной в ключе.
        - При описании **автобусного маршрута** указываются:
            + `type` - тип запроса, указывается `Bus`;
            + `name` - название маршрута;
            + `stops` - массив с названиями остановок, через которые проходит маршрут . 
            У кольцевого маршрута название последней остановки дублирует название первой. Например: `["stop1", "stop2", "stop3", "stop1"]`;
            + `is_roundtrip` - являвляется ли кольцевым.
    * `render_settings` - словарь с настройками визуализации карты в формате svg
    * `routing_settings` - словарь с параметрами маршрутизации. Имеет два ключа:
        - `bus_wait_time` - время ожидания автобуса на остановке, в минутах;
        - `bus_velocity` - скорость автобуса, в км/ч.
    * `serialization_settings` - настройки сериализации. Словарь из одной пары ключ-значение:
        - `file` - название файла, в который будет сохранена сериализованная база данных.
2. **`stat_requests`** — массив с запросами к существующей базе данных (транспортному справочнику):
    * `id` - уникальный числовой идентификатор запроса;
    * `type` - тип запроса:
        - `Bus` - получение информации о маршруте;
        - `Stop` - получение информации об остановке;
        - `Map` - получение графического изображения карты в формате svg
        ![Map](https://raw.githubusercontent.com/Seredenko-V/cpp-transport-catalogue/refactoring/images/stat_requests_Map.png?token=GHSAT0AAAAAAB7SDXXW6EBH2PZTOSJ3CSMOZJ6IOZQ "Map")        
        - `Route` - получение кратчайшего маршрута из остановки А к остановке Б.
### Ответы на запросы
1. **`Bus`**:
    * `request_id` - уникальный числовой идентификатор запроса;
    * `curvature` -  извилистость маршрута. Определяется как отношение длины дорожного расстояния маршрута к длине географического 
    расстояния;
    * `route_length` - длина дорожного расстояния маршрута в метрах;
    * `stop_count` - количество остановок на маршруте;
    * `unique_stop_count` - количество уникальных остановок на маршруте.
2. **`Stop`**:
    * `request_id` - уникальный числовой идентификатор запроса;
    * `buses` - массив названий маршрутов, которые проходят через эту остановку. Названия отсортированы в лексикографическом порядке.
3. **`Map`** - код svg-изображения
4. **`Route`**:
    * `request_id` - уникальный числовой идентификатор запроса;
    * `total_time` - суммарное время в минутах, необходимое для поездки;
    * `items` - массив элементов маршрута, каждый из которых описывает непрерывную активность пассажира, требующую временных затрат:
        - `Wait` - ожидание на остановке в минутах
        - `Bus` -  проехать `span_count` остановок
Если не существует маршрута/остановки/пути последует ответ вида
```
{
    "request_id": 12345,
    "error_message": "not found"
}
```

## Системные требования
* CMake 3.10
* Protobuf [3.21.9](https://github.com/protocolbuffers/protobuf/releases/tag/v21.9)
* GCC 10.0
* C++17

## Запуск
Для успешной сборки проекта следует указать в `СMakeLists.txt` директорию, в которой располоагается собранный protobuf
```
set(CMAKE_PREFIX_PATH <путь до protobuf>)
```
Или непостредствено при сборке, указав при помощи флага
```
-DCMAKE_PREFIX_PATH <путь до protobuf>
```
После сборки проекта будут запускаться тесты. Чтобы запустить проект с необходимыми данными необходимо изменить значение `RUN_TYPE` 
в первой строке `main.cpp`. 
Также важно помнить о том, что выбор основного типа запроса "построение базы данных" или "запросы к уже имеющейся" осуществляется
при помощи установления `make_base` или `stat_requests` в качестве параметра командной строки (аргумента `int main`).

## UML-диаграмма
![UML](ссылка "UML-diagram TransportCatalogue")
