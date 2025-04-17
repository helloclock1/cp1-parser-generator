# Генератор LR(1) парсеров на C++

> [!WARNING]
> Несчастная душа, для будущих поколений, которым зачем-то понадобится мой курсач, проект архивируется и переезжает [сюда](https://github.com/helloclock1/pargen).

![CI](https://github.com/helloclock1/cp1-parser-generator/actions/workflows/ci.yml/badge.svg)

Данная программа --- часть курсового проекта по написанию генератора парсеров с нуля. Программа принимает на вход описание формальной грамматики в формате [BNF](https://en.wikipedia.org/wiki/Backus-Naur_form) (а также несколько флагов) и генерирует заголовочный файл с парсером на языке программирования C++.

## Зависимости

- Компилятор `C++` с поддержкой стандарта `C++20`;
- `cmake` версии `3.31` или выше;
- Библиотека `Boost`;
- (опционально) [nlohmann/json](https://github.com/nlohmann/json) для генерации дерева парсинга в формате JSON в сгенерированном парсере.

## Сборка

```bash
$ git clone https://github.com/helloclock1/cp1-parser-generator.git
$ cd cp1-parser-generator
$ cmake -B build [FLAGS]
$ cmake --build build
```

### Сборка и запуск генератора парсеров

```bash
$ ./gen <input> [options]
```

### Сборка и запуск тестов

```bash
$ ./tests
$ ctest    # эквивалентно
```

### Запуск проверки покрытия кода

Добавьте флаг `-DENABLE_COVERAGE=ON` при сборке. После:

```bash
$ cd build/
$ ctest
```

#### При компиляции с использованием компилятора `g++`

```bash
$ gcovr --root .. --html --html-details -o coverage.html
```

#### При компиляции с использованием компилятора `clang++`

```bash
$ llvm-profdata merge -sparse FILENAME.profraw -o coverage.profdata  # в зависимости от переменных среды, имя .profraw-файла может отличаться
```

Для генерации отчёта в формате HTML:

```bash
$ llvm-cov show ./tests -instr-profile=coverage.profdata -format=html > coverage.html
```

Для генерации отчёта прямо в терминал:

```bash
$ llvm-cov report ./tests -instr-profile=coverage.profdata
```

> [!NOTE]
> Ввиду того, что `g++` и `clang++` и их соответствующие утилиты используют разные алгоритмы подсчёта покрытия, их отчёты, скорее всего, будут отличаться.

## Использование

### Задание формальной грамматики

Пользователем задаётся грамматика в формате BNF и размещается в файле. Нетерминалы оборачиваются символами `<` и `>`. Терминалы могут записываться в двух форматах:

1. `'quote terminal'` или `"quote terminal"`;
2. В отдельной строке: `terminal_name = [0-9]+`, где с правой стороны от знака `=` располагается регулярное выражение, описывающее терминал. Последующее обращение к терминалу в правилах вывода осуществляется без кавычек.

Каждая строка в файле описывает либо терминал, заданный регулярным выражением, либо набор игнорируемых символов (например, `IGNORE = \s+`), либо ненулевое количество выводов. Для набора выводов используется следующий синтаксис:

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
$ ./gen rules.bnf --generate-to parser/ [OPTIONS]
```

Подробнее про возможные флаги можно прочитать в

```bash
$ ./gen --help
```

### Использование парсера

При генерации парсера создаётся 3 файла:

- `Parser.hpp`, содержащий собственно парсер и связанные с ним классы.
- `Lexer.cpp`, содержащий лексер. Этот лексер является лишь примером возможного лексера и сгенерирован с помощью утилиты `flex`, и в связи с этим должен быть отдельно указан при компоновке, например:
```bash
$ clang++ main.cpp ... Lexer.cpp -o main
```
При желании можно использовать любой другой лексер, результатом работы которого является объект `std::vector<Terminal>`.
- `LexerFwd.hpp`, содержащий объявления, нужные для написания собственного лексера или его генерации.

Пример использования сгенерированного парсера:

```cpp
#include <vector>
#include "Parser.hpp"

...

using namespace p;  // пространство имён сгенерированного парсера

// обязательно при использовании сгенерированного лексера
extern std::vector<Terminal> Lex(const char *filename);

std::vector<Terminal> stream = Lex("filename");  // прочитать файл `filename` и сохранить поток токенов в `stream`

Parser parser;
int status = parser.Parse(stream);  // выполнить парсинг потока токенов `stream`
if (status == 0) {
    // выполнение действий при успешном парсинге
} else if (status > 0) {
    // выполнение действий при успешном парсинге с некоторым числом ошибок
} else {
    // выполнение действий при неуспешном парсинге
}
ParseTree tree = parser.GetParseTree();

// При выбранном флаге --json при генерации парсера можно сгенерировать дерево парсинга в JSON файл
JsonTreeGenerator jtg("tree.json");
jtg.Generate(tree);
```

Помимо этого, предоставлен интерфейс для создания собственных классов для обхода дерева с паттерном Visitor. Пример использования:
```cpp
#include <iostream>
#include "Parser.hpp"

using namespace p;

class PrintVisitor : public ParseTreePreorderVisitor {
public:
    void VisitTerminal(const Terminal &t) override {
        std::cout << "Visited terminal named `" << t.name << "` with value `" << t.repr << "`" << std::endl;
    }

    void VisitNonTerminal(const NonTerminal &nt) override {
        std::cout << "Visited nonterminal named `" << nt.name << "`" << std::endl;
    }
};

...

PrintVisitor visitor;
// парсинг выражения

...

ParseTree tree = parser.GetParseTree();

tree.Accept(visitor);
```

Производные класса `ParseTreePreorderVisitor` и `ParseTreePostorderVisitor` выполняют [preorder](https://ru.wikipedia.org/wiki/%D0%9E%D0%B1%D1%85%D0%BE%D0%B4_%D0%B4%D0%B5%D1%80%D0%B5%D0%B2%D0%B0#%D0%9F%D1%80%D1%8F%D0%BC%D0%BE%D0%B9_%D0%BE%D0%B1%D1%85%D0%BE%D0%B4_(NLR)) и [postorder](https://ru.wikipedia.org/wiki/%D0%9E%D0%B1%D1%85%D0%BE%D0%B4_%D0%B4%D0%B5%D1%80%D0%B5%D0%B2%D0%B0#%D0%9E%D0%B1%D1%80%D0%B0%D1%82%D0%BD%D1%8B%D0%B9_%D0%BE%D0%B1%D1%85%D0%BE%D0%B4_(LRN)) обход дерева соответственно. Интерфейс обоих способов обхода идентичен.

