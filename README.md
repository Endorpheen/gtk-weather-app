# GTK Weather App

Простое приложение для отображения погоды, написанное на C с использованием GTK.

## Требования

- GTK 3
- libcurl
- json-glib

## Установка зависимостей

### Ubuntu
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libcurl4-openssl-dev libjson-glib-dev
```

### Windows
Установите MSYS2 и выполните:
```bash
pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-curl mingw-w64-x86_64-json-glib
```

## Компиляция

```bash
gcc -o gtk_example gtk_example.c `pkg-config --cflags --libs gtk+-3.0 json-glib-1.0` -lcurl
```

## Использование

1. Получите API ключ от [OpenWeatherMap](https://openweathermap.org/).
2. Замените "YOUR_API_KEY" в исходном коде на ваш ключ.
3. Скомпилируйте программу.
4. Запустите программу:
   ```
   ./gtk_example
   ```

## Лицензия

[MIT](https://choosealicense.com/licenses/mit/)
