#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private Value {
public:
    // Делаем доступными все конструкторы родительского класса variant
    using variant::variant;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;
    Array AsArray() const;
    Dict AsMap() const;

    const Value& GetValue() const;

};

class Document {
public:
    Document() = default;
    explicit Document(Node root);
    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

// Шаблон, подходящий для вывода double и int
template <typename Value>
inline void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}
// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(bool value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string& value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array& value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict& value, std::ostream& out);

void PrintNode(const Node& node, std::ostream& out);

void Print(const Document& doc, std::ostream& output);

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

bool operator==(Document lhs, Document rhs);
bool operator!=(Document lhs, Document rhs);

}  // namespace json