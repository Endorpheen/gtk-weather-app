# Weather App
Простое приложение для отображения погоды, реализованное на трех платформах: GTK (C), Qt (C++), и Kotlin.

## GTK Version (C)

### Требования
- GTK 3
- libcurl
- json-glib

### Установка зависимостей
#### Ubuntu
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libcurl4-openssl-dev libjson-glib-dev
```
#### Windows
Установите MSYS2 и выполните:
```bash
pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-curl mingw-w64-x86_64-json-glib
```

### Компиляция
```bash
gcc -o gtk_example gtk_example.c `pkg-config --cflags --libs gtk+-3.0 json-glib-1.0` -lcurl
```

## Qt Version (C++)

### Требования
- Qt 5 или выше
- CMake 3.5 или выше
- Компилятор с поддержкой C++11 (например, GCC, Clang, MSVC)

### Установка зависимостей
#### Ubuntu
```bash
sudo apt-get update
sudo apt-get install qt5-default qtcreator cmake
```
#### Windows
1. Скачайте и установите Qt с официального сайта: https://www.qt.io/download
2. Установите CMake: https://cmake.org/download/
3. Убедитесь, что у вас установлен совместимый компилятор (например, MinGW или MSVC)

### Компиляция
1. Перейдите в директорию qt-version
2. Создайте директорию для сборки и перейдите в неё:
   ```
   mkdir build && cd build
   ```
3. Сконфигурируйте проект с помощью CMake:
   ```
   cmake ..
   ```
4. Скомпилируйте проект:
   ```
   cmake --build .
   ```

### Запуск
После успешной компиляции, запустите исполняемый файл `qtweatherapp` (имя может отличаться в зависимости от настроек CMake).

## Kotlin Version

### Требования
- JDK 8 или выше
- Kotlin compiler

### Установка зависимостей
Установите JDK и Kotlin compiler согласно официальной документации Kotlin.

### Компиляция и запуск
```bash
kotlinc KotlinWeatherApp.kt -include-runtime -d KotlinWeatherApp.jar
java -jar KotlinWeatherApp.jar
```

## Использование
1. Получите API ключ от [OpenWeatherMap](https://openweathermap.org/).
2. Замените "YOUR_API_KEY" в исходном коде на ваш ключ.
3. Скомпилируйте и запустите нужную версию программы.

## Лицензия
[MIT](https://choosealicense.com/licenses/mit/)

