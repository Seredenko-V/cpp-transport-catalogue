#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    //using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;


    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        // Делаем доступными все конструкторы родительского класса variant
        using variant::variant;
        using Value = variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsDict() const;

        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        Array AsArray() const;
        Dict AsDict() const;

        Value& GetValue();
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

    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    bool operator==(Document lhs, Document rhs);
    bool operator!=(Document lhs, Document rhs);

}  // namespace json