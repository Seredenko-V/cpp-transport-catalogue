#include "json.h"

using namespace std;

namespace json {

    namespace load {

        void CheckFillStream(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                throw ParsingError("String parsing error");
            }
        }

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            CheckFillStream(input);
            string word;
            for (char c; input >> c && c != ',';) {
                word.push_back(c);
            }
            if (word != "null"s) {
                throw ParsingError("The uploaded text is not null"s);
            }
            return Node(nullptr);
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadBool(istream& input) {
            CheckFillStream(input);
            string word;
            char c;
            while (input >> c && isalpha(c)) {
                word.push_back(c);
            }
            input.putback(c);
            if (word == "true"s) {
                return Node(true);
            }
            if (word == "false"s) {
                return Node(false);
            }
            throw ParsingError("The uploaded text is not boolean"s);
        }

        Node LoadArray(istream& input) {
            CheckFillStream(input);
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(std::istream& input) {
            Dict dict;

            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    } else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                } else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }
            return Node(std::move(dict));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else if (isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else {
                throw ParsingError("Invalid first character: "s + c);
            }
        }

    }  // namespace load

bool Node::IsInt() const {
    return holds_alternative<int>(*this) ? true : false;
}
bool Node::IsDouble() const {
    return holds_alternative<double>(*this) || holds_alternative<int>(*this) ? true : false;
}
bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this) ? true : false;
}
bool Node::IsBool() const {
    return holds_alternative<bool>(*this) ? true : false;
}
bool Node::IsString() const {
    return holds_alternative<string>(*this) ? true : false;
}
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(*this) ? true : false;
}
bool Node::IsArray() const {
    return holds_alternative<Array>(*this) ? true : false;
}
bool Node::IsMap() const {
    return holds_alternative<Dict>(*this) ? true : false;
}


int Node::AsInt() const {
    if (!this->IsInt()) {
        throw std::logic_error("Data types do not match"s);
    }
    return get<int>(*this);
}
double Node::AsDouble() const {
    if (!this->IsDouble()) {
        throw std::logic_error("Data types do not match"s);
    }
    if (holds_alternative<double>(*this)) {
        return get<double>(*this);
    } else {
        return static_cast<double>(get<int>(*this));
    }   
}
const string& Node::AsString() const {
    if (!this->IsString()) {
        throw std::logic_error("Data types do not match"s);
    }
    return get<string>(*this);
}
bool Node::AsBool() const {
    if (!this->IsBool()) {
        throw std::logic_error("Data types do not match"s);
    }
    return get<bool>(*this);
}
Array Node::AsArray() const {
    if (!this->IsArray()) {
        throw std::logic_error("Data types do not match"s);
    }
    return get<Array>(*this);
}
Dict Node::AsMap() const {
    if (!this->IsMap()) {
        throw std::logic_error("Data types do not match"s);
    }
    return get<Dict>(*this);
}


const Value& Node::GetValue() const {
    return *this;
}


Document::Document(Node root)
    : root_(move(root)) {
}
const Node& Document::GetRoot() const {
    return root_;
}
Document Load(istream& input) {
    return Document{ load::LoadNode(input) };
}


void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}
// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(bool value, std::ostream& out) {
    out << boolalpha << value;
}
// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string& value, std::ostream& out) {
    // Тут сделать замену символов: '\\' => '\\\\', 
    // '\r' => '\\r',
    // '\' => '\\\'
    string result = "\"";
    result.reserve(value.size());
    for (const char& liter : value) {
        switch (liter) {
            case '\n':
                result += "\\n"s;
                break;
            case '\r':
                result += "\\r"s;
                break;
            case '\"':
                result += "\\\""s;
                break;
            case '\\':
                result += "\\\\"s;
                break;
            default:
                result.push_back(liter);
        }
    }
    result += '\"';
    out << result;

}
// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const Array& value, std::ostream& out) {
    out << '[' << '\n';
    bool is_first = true;
    for (const Node& node : value) {
        if (is_first) {
            PrintNode(node, out);
            is_first = false;
        } else {
            out << ',' << '\n';
            PrintNode(node, out);
        }
    }
    out << '\n' << ']';
}
// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const Dict& value, std::ostream& out) {
    out << '{' << '\n';
    bool is_first = true;
    for (const auto& [str, node] : value) {
        if (is_first) {
            PrintValue(str, out);
            out << ':' << ' ';
            PrintNode(node, out);
            is_first = false;
        } else {
            out << ',' << '\n';
            PrintValue(str, out);
            out << ':' << ' ';
            PrintNode(node, out);
        }
    }
    out << '\n' << '}';
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}
bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

bool operator==(Document lhs, Document rhs) {
    return lhs.GetRoot().GetValue() == rhs.GetRoot().GetValue();
}
bool operator!=(Document lhs, Document rhs) {
    return !(lhs == rhs);
}

}  // namespace json