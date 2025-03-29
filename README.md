# Генератор LR(1) парсеров на C++

![CI](https://github.com/helloclock1/cp1-parser-generator/actions/workflows/ci.yml/badge.svg)

Данная программа --- часть курсового проекта по написанию генератора парсеров с нуля. Программа принимает на вход описание формальной грамматики в формате [BNF](https://en.wikipedia.org/wiki/Backus-Naur_form) (а также несколько флагов) и генерирует заголовочный файл с парсером на языке программирования C++.

## Зависимости

- Компилятор `C++` с поддержкой стандарта `C++20`;
- `cmake` версии `3.31` или выше;
- Библиотека `Boost`;
- (опционально) [nlohmann/json](https://github.com/nlohmann/json) для генерации дерева парсинга в формате JSON в сгенерированном парсере.

## Сборка

```bash
git clone https://github.com/helloclock1/cp1-parser-generator.git
cd cp1-parser-generator
cmake -B build [FLAGS]
cmake --build build
```

### Сборка и запуск генератора парсеров

```bash
./gen <input> [options]
```

### Сборка и запуск тестов

```bash
./tests
ctest    # эквивалентно
```

### Запуск проверки покрытия кода

Добавьте флаг `-DENABLE_COVERAGE=ON` при сборке. После:

```bash
cd build/
ctest
```

#### При компиляции с использованием компилятора `g++`

```bash
gcovr --root .. --html --html-details -o coverage.html
```

#### При компиляции с использованием компилятора `clang++`

```bash
llvm-profdata merge -sparse FILENAME.profraw -o coverage.profdata  # в зависимости от переменных среды, имя .profraw-файла может отличаться
```

Для генерации отчёта в формате HTML:

```bash
llvm-cov show ./tests -instr-profile=coverage.profdata -format=html > coverage.html
```

Для генерации отчёта прямо в терминал:

```bash
llvm-cov report ./tests -instr-profile=coverage.profdata
```

> [!NOTE]
> Ввиду того, что `g++` и `clang++` и их соответствующие утилиты используют разные алгоритмы подсчёта покрытия, их отчёты, скорее всего, будут отличаться.

## Использование

### Задание формальной грамматики

Пользователем задаётся грамматика в формате BNF и размещается в файле. Нетерминалы оборачиваются символами `<` и `>`. Терминалы могут записываться в двух форматах:

1. `'quote terminal'` или `"quote terminal"`;
2. В отдельной строке: `terminal_name = [0-9]+`, где с правой стороны от знака `=` располагается регулярное выражение, описывающее терминал. Последующее обращение к терминалу в правилах вывода осуществляется без кавычек.

Каждая строка в файле описывает либо терминал, заданный регулярным выражением, либо набор игнорируемых символов (`IGNORE = \s+`), либо ненулевое количество выводов. Для набора выводов используется следующий синтаксис:

```math
<\mathrm{NonTerminal}>\ = \mathrm{Token}_1^1 \dots \mathrm{Token}_{k_1}^1 | \mathrm{Token}_1^2 \dots \mathrm{Token}_{k_2}^2 | \dots | \mathrm{Token}_1^n \dots \mathrm{Token}_{k_n}^n,
```

где вместо $\mathrm{Token_i^j}$ может быть терминал, нетерминал или пустая строка (записываемая как `EPSILON`). Пустая строка не может содержаться в выводе, который будет непустым при удалении пустой строки из вывода. Записанный выше вывод может быть записан в более развёрнутом виде:

```math
\begin{align*}
&<\mathrm{NonTerminal}>\ = \mathrm{Token}_1^1 \dots \mathrm{Token}_{k_1}^1\\
&<\mathrm{NonTerminal}>\ = \mathrm{Token}_1^2 \dots \mathrm{Token}_{k_2}^2 \\
&\vdots \\
&<\mathrm{NonTerminal}>\ = \mathrm{Token}_1^n \dots \mathrm{Token}_{k_n}^n
\end{align*}
```

Примеры грамматик в нужном формате могут быть найдены в директории `example_grammars/`.

### Генерация парсера

Пусть грамматика задана в файле `rules.bnf` (формат файла неважен). Тогда для генерации парсера необходимо выполнить команду:

```bash
./gen rules.bnf --generate-to parser/ [OPTIONS]
```

Подробнее про возможные флаги можно прочитать в

```bash
./gen --help
```

### Использование парсера

TODO
