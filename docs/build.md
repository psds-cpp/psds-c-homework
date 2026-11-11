# Сборка проекта

## Содержание
1. [Командная строка Linux](#командная-строка-linux)
2. [Visual Studio](#visual-studio)
3. [QtCreator](#qtcreator)

## По поводу теста

Юнит-тест test.cpp имеет зависимость – библиотеку/фреймворк GoogleTest.
С управлением зависимостями в C++ все довольно [печально](https://youtu.be/IwuBZpLUq8Q?si=RcYTNlPB5wl5RTtc)...
Обычно трудно обеспечить корректный и универсальный (для всех окружений сборки) способ установить зависимости.
Поэтому компиляция теста и его прогон не является обязательной частью домашнего задания.
По-умолчанию тест в проекте не собирается.
Однако, я буду очень рад, если вы попытаетесь собрать его и проверить свое решение.
Для этого я постарался сделать относительно отказоустойчивый способ собрать тест.
Естественно, всех ситуаций я предусмотреть не могу, и проблемы со сборкой теста не исключены,
но я попробовал собрать проект с тестом по инструкциям ниже на нескольких дистрибутивах Linux и в разных
IDE, и по-моему, всё получилось.
Если вы собираетесь компилировать с тестом, то вот общая идея, по которой система сборки работает с GoogleTest:
1. система сборки ищет библиотеки GoogleTest в системе;
2. если в системе GoogleTest не установлен, предпринимается попытка самостоятельно его скачать.

Поэтому в теории, проект должен собраться с тестом, даже если GoogleTest не установлена в вашей системе.
Но это в теории... 🙏

## Командная строка Linux

### Сборка библиотеки

1. Перейти в директорию с проектом:
    ```sh
    cd prosoft-c-stack
    ```
2. Выполнить конфигурацию `cmake` в директории `build/Release`, которая создастся автоматически:
    ```sh
    cmake -B build/Release
    ```
3. Собрать проект без распараллеливания или с распараллеливанием:
    ```sh
    cmake --build build/Release
    cmake --build build/Release -j $(nproc)
    ```

### Сборка и запуск теста (необязательно)

1. Установить фреймворк [googletest](https://github.com/google/googletest),
    библиотеку адрес-санитайзера и библиотеку санитайзера UB любым известным способом
    (собрать из исходников, скачать с помощью пакетного менеджера системы).

    Для Debian репозиториев:
    ```sh
    sudo apt install build-essential libgtest-dev libgmock-dev
    ```
    Для RedHat репозиториев:
    ```sh
    sudo dnf install gtest-devel gmock-devel libasan libubsan
    ```
    Для ArchLinux:
    ```sh
    pacman -S base-devel gtest gmock
    ```
    Я предпочитаю пользоваться пакетным менеджером [conan](https://conan.io/), для которого в репозитории я
    подготовил файл [conanfile.txt](../conanfile.txt).
    Если у вас есть этот инструмент и вы с ним знакомы, установить googletest можно с помощью команды:
    ```sh
    conan install . --build=missing
    ```
3. Сконфигурировать `cmake` с опцией `WITH_TEST`:
    ```sh
    cmake -B build/Release -DWITH_TEST=ON
    cmake --preset conan-release -DWITH_TEST=ON # если использовали conan
    ```
4. Собрать проект без распараллеливания или с распараллеливанием:
    ```sh
    cmake --build build/Release
    cmake --build build/Release -j $(nproc)
    ```
5. Запустить выполнение теста:
   ```sh
   ctest --test-dir build/Release
   ```

### Автоформатирование

Обязательно после внесения изменений применяйте
автоформатирование по правилам, определенным в [``.clang-format``](./.clang-format).
Соответствие стилю автоматически проверяется в GitHub Actions
([ci.yml](../.github/workflows/ci.yml)) командой:

```sh
clang-format --dry-run -Werror cstack.c
```

Если файл соответствует стилю, команда ничего не выведет и завершится кодом 0,
иначе будут перечислены нарушения, а команда завершится ненулевым кодом.

Для форматирования потребуется установить утилиту `clang-format`.
Конфигурация совместима с версией 19 и старше (в GitHub Actions используется 19.1.7):

```sh
$ clang-format --version
clang-format version 19.1.7
```

Перед коммитом выполните:

```sh
clang-format -i cstack.*
clang-format --dry-run -Werror cstack.c # убедитесь, что проверка проходит
```

## Visual Studio

### Сборка библиотеки

1. В Visual Studio выберите ``Open a local folder`` и укажите путь до директории с проектом.
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/ec60aaae-63a6-45e8-8208-5f4728f959ef)
2. Соберите библиотеку, нажав \<F7\> или ``Build``->``Build All``.
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/9926a3c2-dfa1-4c74-a3ac-f059fd50518f)

### Сборка теста (необязательно)

1. В ``Project``->``CMake settings for cstack`` поставбте галочку в чекбоксе ``WITH_TEST`` и нажмите \<Ctrl-S\>.
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/3c649b22-c32a-4a21-9301-57f5b721b65e)
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/0c3e0cbd-4b06-4e48-8045-226beb6c7d3c)
2. Нажмите ``Tests``->``Run CTests for cstack``.
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/5844208c-b947-4e9d-b021-d7a0768a083c)
3. Для более подробного изучения результатов тестов, откройте ``Test Explorer``.
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/510ee7ba-928c-4d51-a8ea-e38fc71e2087)

## QtCreator

### Сборка библиотеки

1. В QtCreator выберите ``Open Project`` и укажите путь до репозитория:
    ![Screenshot from 2023-11-19 23-36-51](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/376e998c-39fa-4670-8a4f-d477c9b81d5d)
2. Выберите ``CMakeLists.txt`` и подтвердите действие:
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/a6b2e921-025f-4a62-aa01-6b216c780289)
3. Сконфигурируйте проект:
    ![Screenshot from 2023-11-19 23-40-06](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/045ad414-92c8-4441-ba9d-ddb2949ad5be)
4. Для сборки нажмите на символ молотка в нижнем левом углу или нажмите \<Ctrl-B\>.
    ![Screenshot from 2023-11-19 23-41-08](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/d448a90e-b720-46bf-bd84-7e0d7217a1b8)

### Сборка теста (необязательно)

1. Зайдите в ``Projects``:
    ![Screenshot from 2023-11-19 23-43-19](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/97f544cc-cde0-4e7d-9e93-cfe32e07aa58)
2. Поставьте галочку в чекбоксе ``WITH_TEST`` и нажмите ``Run CMake``
    ![Screenshot from 2023-11-19 23-44-15](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/b65ebe66-a8be-4ecc-8f29-3df825c85027)
3. Запустите сборку.
4. Запустите тесты ``Tools``->``Tests``->``Run All Tests``:
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/71d2004d-4da1-4360-98b3-2eafb8038737)
    ![image](https://github.com/czertyaka/prosoft-c-stack/assets/69390349/c18b07c9-0b9b-4e00-a0e6-951588776134)
