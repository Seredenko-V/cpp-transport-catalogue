#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
        out << "rgb("sv << (unsigned)rgb.red << ","sv << (unsigned)rgb.green << ","sv << (unsigned)rgb.blue << ")"sv;
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const Rgba& rgba) {
        out << "rgba("sv << (unsigned)rgba.red << ","sv << (unsigned)rgba.green << ","sv << (unsigned)rgba.blue
        << ","sv << rgba.opacity << ")"sv;
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const Color& color) {
        visit(OstreamColor{out}, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& stroke_line_cap) {
        switch (stroke_line_cap) {
            case svg::StrokeLineCap::BUTT:
                out << "butt"sv;
                break;
            case svg::StrokeLineCap::ROUND:
                out << "round"sv;
                break;
            case svg::StrokeLineCap::SQUARE:
                out << "square"sv;
                break;
            default:
                break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& stroke_line_cap) {
        switch (stroke_line_cap) {
            case svg::StrokeLineJoin::ARCS:
                out << "arcs"sv;
                break;
            case svg::StrokeLineJoin::BEVEL:
                out << "bevel"sv;
                break;
            case svg::StrokeLineJoin::MITER:
                out << "miter"sv;
                break;
            case svg::StrokeLineJoin::MITER_CLIP:
                out << "miter-clip"sv;
                break;
            case svg::StrokeLineJoin::ROUND:
                out << "round"sv;
                break;
            default:
                break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_first = true;
        for (const Point& point : points_) {
            if (is_first) {
                out << point.x << ","sv << point.y;
                is_first = false;
            } else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\" ";
        RenderAttrs(context.out);
        out <<"/>"sv;
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_from_position_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv;
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_from_position_.x << "\" dy=\""sv << offset_from_position_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\" "sv;
        if (!font_weight_.empty()) {
            out << "font-weight=\""sv << font_weight_ << "\" "sv;
        }
        if (!font_family_.empty()) {
            out << "font-family=\""sv << font_family_ << "\""sv;
        }
        //RenderAttrs(context.out);
        out << ">"sv;
        for (const char& symbol : data_) {
            switch (symbol) {
                case '\"':
                    out << "&quot;"sv;
                    break;
                case '<':
                    out << "&lt;"sv;
                    break;
                case '>':
                    out << "&gt;"sv;
                    break;
                case '\'':
                    out << "&apos;"sv;
                    break;
                case '&':
                    out << "&amp;"sv;
                    break;
                default:
                    out << symbol;
                    break;
            }
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        RenderContext ctx(out, 2, 2);
        for (const std::unique_ptr<Object>& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }
}  // namespace svg