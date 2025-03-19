#include "CodeGenerator.h"

#include <filesystem>

#include "Automaton.h"
#include "Entities.h"

CodeGenerator::CodeGenerator(
    const std::string &folder, ActionTable &at, GotoTable &gt, const Grammar &g
)
    : folder_(folder), at_(at), gt_(gt), g_(g) {
}

void CodeGenerator::Generate() {
    GenerateLexer();
    GenerateParser();
}

void CodeGenerator::GenerateParser() {
    std::filesystem::create_directories(folder_ + "/tmp");
    std::ofstream out(folder_ + "/Parser.hpp");
    out << "#pragma once\n";
    out << "\n";
    out << "#include <algorithm>\n";
    out << "#include <fstream>\n";
    out << "#include <memory>\n";
    out << "#include <nlohmann/json.hpp>\n";
    out << "#include <optional>\n";
    out << "#include <ranges>\n";
    out << "#include <set>\n";
    out << "#include <stack>\n";
    out << "#include <stdexcept>\n";
    out << "#include <string>\n";
    out << "#include <unordered_map>\n";
    out << "#include <variant>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"LexerFwd.hpp\"\n";
    out << "\n";
    out << "namespace p {\n";
    out << "};  // namespace p\n";
    out << "\n";
    out << "namespace std {\n";
    out << "template <>\n";
    out << "struct hash<p::Terminal> {\n";
    out << "    size_t operator()(const p::Terminal &t) const {\n";
    out << "        return hash<std::string>()(\"t\" + t.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<p::NonTerminal> {\n";
    out << "    size_t operator()(const p::NonTerminal &nt) const {\n";
    out << "        return hash<std::string>()(\"nt\" + nt.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<p::Token> {\n";
    out << "    size_t operator()(const p::Token &t) const {\n";
    out << "        if (std::holds_alternative<p::Terminal>(t)) {\n";
    out << "            return "
           "hash<p::Terminal>()(std::get<p::Terminal>(t));\n";
    out << "        } else {\n";
    out << "            return "
           "hash<p::NonTerminal>()(std::get<p::NonTerminal>(t));\n";
    out << "        }\n";
    out << "    }\n";
    out << "};\n";
    out << "};  // namespace std\n";
    out << "\n";
    out << "namespace p {\n";
    out << "using json = nlohmann::ordered_json;\n";
    out << "\n";
    out << "\n";
    out << "bool operator<(const Token &a, const Token &b);\n";
    out << "\n";
    out << "using Production = std::vector<Token>;\n";
    out << "\n";
    out << "struct Rule {\n";
    out << "    NonTerminal lhs;\n";
    out << "    Production prod;\n";
    out << "};\n";
    out << "\n";
    out << "using Grammar = std::vector<Rule>;\n";
    out << "\n";
    out << "struct Item {\n";
    out << "    Item(size_t rule_number, size_t dot_pos);\n";
    out << "    size_t rule_number;\n";
    out << "    size_t dot_pos;\n";
    out << "\n";
    out << "    friend bool operator<(const Item &a, const Item &b);\n";
    out << "    bool DotAtEnd() const;\n";
    out << "    std::optional<Token> NextToken() const;\n";
    out << "    Rule GetRule() const;\n";
    out << "};\n";
    out << "\n";
    out << "using State = std::set<Item>;\n";
    out << "\n";
    out << "enum class ActionType {\n";
    out << "    SHIFT,\n";
    out << "    REDUCE,\n";
    out << "    ACCEPT,\n";
    out << "    ERROR\n";
    out << "};\n";
    out << "\n";
    out << "struct Action {\n";
    out << "    ActionType type = ActionType::ERROR;\n";
    out << "    size_t value = 0;\n";
    out << "};\n";
    out << "\n";
    out << "using ActionTable = std::vector<std::unordered_map<std::string, "
           "Action>>;\n";
    out << "using GotoTable = std::unordered_map<size_t, "
           "std::unordered_map<NonTerminal, size_t>>;\n";
    out << "\n";
    out << "std::string QualName(Token token) {\n";
    out << "    if (std::holds_alternative<Terminal>(token)) {\n";
    out << "        Terminal t = std::get<Terminal>(token);\n";
    out << "        if (t.repr.empty()) {\n";
    out << "            return \"T_\" + t.name;\n";
    out << "        } else {\n";
    out << "            return \"R_\" + t.name;\n";
    out << "        }\n";
    out << "    } else {\n";
    out << "        return \"NT_\" + std::get<NonTerminal>(token).name;\n";
    out << "    }\n";
    out << "}\n";
    out << "\n";
    out << "class ParserTables {\n";
    out << "public:\n";
    out << "    static const ActionTable &GetActionTable() {\n";
    out << "        static const ActionTable table = {\n";
    for (size_t i = 0; i < at_.size(); ++i) {
        out << "            {\n";
        size_t j = 0;
        for (const auto &[terminal, action] : at_[i]) {
            out << "                ";
            out << "{\"" << terminal << "\", Action{ActionType::";
            switch (action.type_) {
                case ActionType::ACCEPT:
                    out << "ACCEPT";
                    break;
                case ActionType::ERROR:
                    out << "ERROR";
                    break;
                case ActionType::REDUCE:
                    out << "REDUCE";
                    break;
                case ActionType::SHIFT:
                    out << "SHIFT";
                    break;
            }
            out << ", " << action.value_ << "}}";
            if (j != at_[i].size() - 1) {
                out << ",";
            }
            ++j;
            out << "\n";
        }
        out << "            }";
        if (i != at_.size() - 1) {
            out << ",";
        }
        out << "\n";
    }
    out << "        };\n";
    out << "\n";
    out << "        return table;\n";
    out << "    }\n";
    out << "\n";
    out << "    static const GotoTable &GetGotoTable() {\n";
    out << "        static const GotoTable table = {\n";
    for (const auto &[state, table] : gt_) {
        size_t nonzero_count = 0;
        for (const auto &[terminal, goto_value] : table) {
            if (goto_value != 0) {
                ++nonzero_count;
            }
        }
        if (nonzero_count == 0) {
            continue;
        }
        out << "            ";
        out << "{\n";
        out << "                ";
        out << state << ", {\n";
        size_t j = 0;
        for (const auto &[terminal, goto_value] : table) {
            if (goto_value == 0) {
                continue;
            }
            out << "                    ";
            out << "{NonTerminal{\"" << terminal.name_ << "\"}, " << goto_value
                << "}";
            if (j != table.size() - 1) {
                out << ",";
            }
            ++j;
            out << "\n";
        }
        out << "                }\n";
        out << "            },\n";
    }
    out << "        };\n";
    out << "\n";
    out << "        return table;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "class ParseTreeVisitor {\n";
    out << "public:\n";
    out << "    virtual void VisitTerminal(const Terminal &t) = 0;\n";
    out << "    virtual void VisitNonTerminal(const NonTerminal &nt) = 0;\n";
    out << "    virtual ~ParseTreeVisitor() = default;\n";
    out << "};\n";
    out << "\n";
    out << "struct ParseTreeNode {\n";
    out << "    Token value;\n";
    out << "    std::vector<std::shared_ptr<ParseTreeNode>> children;\n";
    out << "\n";
    out << "    void Accept(ParseTreeVisitor &visitor) const "
           "{\n";
    out << "        if (std::holds_alternative<p::Terminal>(value)) {\n";
    out << "            visitor.VisitTerminal(std::get<Terminal>(value));\n";
    out << "        } else {\n";
    out << "            "
           "visitor.VisitNonTerminal(std::get<NonTerminal>(value));\n";
    out << "        }\n";
    out << "        for (const auto &child : children) {\n";
    out << "            child->Accept(visitor);\n";
    out << "        }\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "class ParseTree {\n";
    out << "public:\n";
    out << "    explicit ParseTree(std::shared_ptr<ParseTreeNode> root) : "
           "root_(root) {}\n";
    out << "\n";
    out << "    std::shared_ptr<ParseTreeNode> GetRoot() const {\n";
    out << "        return root_;\n";
    out << "    }\n";
    out << "\n";
    out << "    void Accept(ParseTreeVisitor &visitor) const {\n";
    out << "        root_->Accept(visitor);\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    std::shared_ptr<ParseTreeNode> root_;\n";
    out << "};\n";
    out << "\n";
    out << "class JsonTreeGenerator {\n";
    out << "public:\n";
    out << "    JsonTreeGenerator(const std::string &filename) : "
           "filename_(filename) {}\n";
    out << "\n";
    out << "    void Generate(ParseTree tree) {\n";
    out << "        json j = GenerateForNode(tree.GetRoot());\n";
    out << "        std::ofstream out(filename_);\n";
    out << "        out << j.dump(4);\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    json GenerateForNode(std::shared_ptr<ParseTreeNode> node){\n";
    out << "        json tree;\n";
    out << "        if (std::holds_alternative<Terminal>(node->value)) {\n";
    out << "            Terminal t = std::get<Terminal>(node->value);\n";
    out << "            tree[\"value\"] = t.name;\n";
    out << "            if (!t.repr.empty()) {\n";
    out << "                tree[\"lexeme\"] = t.repr;\n";
    out << "            }\n";
    out << "        } else {\n";
    out << "            tree[\"type\"] = "
           "std::get<NonTerminal>(node->value).name;\n";
    out << "        }\n";
    out << "        if (node->children.empty()) {\n";
    out << "            return tree;\n";
    out << "        }\n";
    out << "        for (const auto child : node->children) {\n";
    out << "            "
           "tree[\"children\"].push_back(GenerateForNode(child));\n";
    out << "        }\n";
    out << "        return tree;\n";
    out << "    }\n";
    out << "\n";
    out << "    std::string filename_;\n";
    out << "};\n";
    out << "\n";
    out << "class Parser {\n";
    out << "public:\n";
    out << "    void Parse(const std::vector<Terminal> &stream) {\n";
    out << "        Clear();\n";
    out << "        for (const Terminal &token : stream | std::views::reverse) "
           "{\n";
    out << "            seq_.push(token);\n";
    out << "        }\n";
    out << "    \n";
    out << "        Terminal a = seq_.top();\n";
    out << "        bool done = false;\n";
    out << "        while (!done) {\n";
    out << "            size_t s = state_stack_.top();\n";
    out << "            Action action = action_[s][QualName(a)];\n";
    out << "            switch (action.type) {\n";
    out << "                case ActionType::SHIFT: {\n";
    out << "                    auto new_node = "
           "std::make_shared<ParseTreeNode>(ParseTreeNode{a, {}});\n";
    out << "                    node_stack_.push(new_node);\n";
    out << "                    state_stack_.push(action.value);\n";
    out << "                    seq_.pop();\n";
    out << "                    a = seq_.top();\n";
    out << "                    break;\n";
    out << "                }\n";
    out << "                case ActionType::REDUCE: {\n";
    out << "                    Rule rule = g_[action.value];\n";
    out << "                    std::vector<std::shared_ptr<ParseTreeNode>> "
           "new_children;\n";
    out << "                    if (QualName(rule.prod[0]) != \"T_\") {\n";
    out << "                        for (size_t i = 0; i < rule.prod.size(); "
           "++i) "
           "{\n";
    out << "                            "
           "new_children.push_back(node_stack_.top());\n";
    out << "                            node_stack_.pop();\n";
    out << "                            state_stack_.pop();\n";
    out << "                        }\n";
    out << "                    }\n";
    out << "                    std::reverse(new_children.begin(), "
           "new_children.end());\n";
    out << "                    auto new_node = "
           "std::make_shared<ParseTreeNode>(ParseTreeNode{rule.lhs, "
           "new_children});\n";
    out << "                    node_stack_.push(new_node);\n";
    out << "                    size_t t = state_stack_.top();\n";
    out << "                    state_stack_.push(goto_[t][rule.lhs]);\n";
    out << "                    break;\n";
    out << "                }\n";
    out << "                case ActionType::ACCEPT:\n";
    out << "                    done = true;\n";
    out << "                    break;\n";
    out << "                case ActionType::ERROR:\n";
    out << "                    throw std::runtime_error(\"PANIC PANIC "
           "PANIC\");\n";
    out << "            }\n";
    out << "        }\n";
    out << "    }\n";
    out << "\n";
    out << "    ParseTree GetParseTree() const {\n";
    out << "        return ParseTree(node_stack_.top());\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    void Clear() {\n";
    out << "        while (!seq_.empty()) {\n";
    out << "            seq_.pop();\n";
    out << "        }\n";
    out << "        seq_.push(Terminal{\"$\", \"$\"});\n";
    out << "        while (!state_stack_.empty()) {\n";
    out << "            state_stack_.pop();\n";
    out << "        }\n";
    out << "        state_stack_.push(0);\n";
    out << "        while (!node_stack_.empty()) {\n";
    out << "            node_stack_.pop();\n";
    out << "        }\n";
    out << "    }\n";
    out << "\n";
    out << "    Grammar g_ = {\n";
    for (const Rule &rule : g_.rules_) {
        out << "        {\n";
        out << "            NonTerminal{\"" << rule.lhs.name_ << "\"},\n";
        out << "            {\n";
        for (const Token &token : rule.prod) {
            out << "                ";
            if (IsTerminal(token)) {
                Terminal t = std::get<Terminal>(token);
                out << "Terminal{\"" << t.name_ << "\"";
                if (!t.repr_.empty()) {
                    out << ", \"" << t.repr_ << "\"";
                }
                out << "},\n";
            } else {
                out << "NonTerminal{\"" << std::get<NonTerminal>(token).name_
                    << "\"},\n";
            }
        }
        out << "            },\n";
        out << "        },\n";
    }
    out << "    };\n";
    out << "\n";
    out << "    std::stack<Terminal> seq_;\n";
    out << "    std::stack<size_t> state_stack_;\n";
    out << "    std::stack<std::shared_ptr<ParseTreeNode>> node_stack_;\n";
    out << "\n";
    out << "    ActionTable action_ = ParserTables::GetActionTable();\n";
    out << "    GotoTable goto_ = ParserTables::GetGotoTable();\n";
    out << "};\n";
    out << "};  // namespace p\n";
    out.close();
}

void CodeGenerator::GenerateLexer() {
    std::filesystem::create_directories(folder_ + "/tmp");
    std::ofstream out(folder_ + "/tmp/Lexer.l");
    out << "%option noyywrap\n";
    out << "%{\n";
    out << "#include <cstdio>\n";
    out << "#include <cstdlib>\n";
    out << "#include <iostream>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"LexerFwd.hpp\"\n";
    out << "\n";
    out << "std::vector<p::Terminal> tokens;\n";
    out << "\n";
    out << "extern FILE *yyin;\n";
    out << "extern \"C\" int yylex();\n";
    out << "\n";
    out << "std::vector<p::Terminal> Lex(const char *filename) {\n";
    out << "    FILE *file = fopen(filename, \"r\");\n";
    out << "    if (!file) {\n";
    out << "        std::cerr << \"Error: cannot open file `\" << filename << "
           "\"`\" << std::endl;\n";
    out << "        exit(1);\n";
    out << "    }\n";
    out << "    yyin = file;\n";
    out << "    yylex();\n";
    out << "    return tokens;\n";
    out << "}\n";
    out << "%}\n";
    out << "\n";
    out << "%%\n";
    out << "\n";
    for (const Token &token : g_.tokens_) {
        if (IsTerminal(token)) {
            Terminal t = std::get<Terminal>(token);
            if (t.name_.empty()) {
                continue;
            }
            if (t.repr_.empty()) {
                out << "\"";
                for (const char &c : t.name_) {
                    // user may pass something like '\n' as a token. this should
                    // be taken exactly as if it was embraced in double quotes
                    // without modifying (so '\n' is considered a newline, not a
                    // sequence of backslash and `n`)
                    if (c == '"') {
                        out << "\\";
                    }
                    out << c;
                }
                out << "\"" << "\t"
                    << "{ tokens.push_back(p::Terminal{yytext}); }\n";
            } else if (t.repr_ != " ") {
                out << t.repr_ << "\t" << "{ tokens.push_back(p::Terminal{\""
                    << t.name_ << "\", yytext}); }\n";
            }
        }
    }
    for (const std::string &regex : g_.ignored_) {
        out << regex << "\t;\n";
    }

    out << "\n";
    out << "%%\n";
    out.close();

    std::string build_lexer_command =
        "flex --outfile=" + folder_ + "/Lexer.cpp " + folder_ + "/tmp/Lexer.l";
    system(build_lexer_command.c_str());
    std::filesystem::remove_all(folder_ + "/tmp");
    out = std::ofstream(folder_ + "/LexerFwd.hpp");
    out << "#pragma once\n";
    out << "\n";
    out << "#include \"variant\"\n";
    out << "\n";
    out << "namespace p {\n";
    out << "struct Terminal {\n";
    out << "    std::string name;\n";
    out << "    std::string repr = \"\";\n";
    out << "    bool operator==(const Terminal &other) const {\n";
    out << "        return name == other.name;\n";
    out << "    }\n";
    out << "    bool operator!=(const Terminal &other) const {\n";
    out << "        return name != other.name;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "struct NonTerminal {\n";
    out << "    std::string name;\n";
    out << "    bool operator==(const NonTerminal &other) const {\n";
    out << "        return name == other.name;\n";
    out << "    }\n";
    out << "    bool operator!=(const NonTerminal &other) const {\n";
    out << "        return name != other.name;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "using Token = std::variant<Terminal, NonTerminal>;\n";
    out << "};  // namespace p\n";
}