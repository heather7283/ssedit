#include <cmath>
#include <algorithm>

#include "shapes.hpp"

Line::Line(ImVec2 start, ImU32 color, float thickness) {
    this->start = start;
    this->end = start;
    this->color = color;
    this->thickness = thickness;
}

void Line::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    draw_list->AddLine(offset + this->start * scale, offset + this->end * scale,
                       this->color, this->thickness * scale);
}

void Line::Update(ImVec2 pos) {
    this->end = pos;
}

Circle::Circle(ImVec2 center, ImU32 color, float thickness, bool fill) {
    this->center = center;
    this->radius = 0;
    this->color = color;
    this->thickness = thickness;
    this->fill = fill;
}

void Circle::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    if (this->fill) {
        draw_list->AddCircleFilled(offset + this->center * scale, this->radius * scale,
                                   this->color, 0);
    } else {
        draw_list->AddCircle(offset + this->center * scale, this->radius * scale,
                             this->color, 0, this->thickness * scale);
    }
}

void Circle::Update(ImVec2 pos) {
    ImVec2 d = pos - this->center;
    this->radius = sqrt((d.x * d.x) + (d.y * d.y));
}

Rectangle::Rectangle(ImVec2 start, ImU32 color, float thickness, bool fill) {
    this->start = start;
    this->end = start;
    this->color = color;
    this->thickness = thickness;
    this->fill = fill;
}

void Rectangle::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    if (this->fill) {
        draw_list->AddRectFilled(offset + this->start * scale, offset + this->end * scale,
                                 this->color, 0.0f, 0);
    } else {
        draw_list->AddRect(offset + this->start * scale, offset + this->end * scale,
                           this->color, 0.0f, 0, this->thickness * scale);
    }
}

void Rectangle::Update(ImVec2 pos) {
    this->end = pos;
}

Freeform::Freeform(ImVec2 start, ImU32 color, float thickness) {
    this->points.push_back(start);
    this->color = color;
    this->thickness = thickness;
}

void Freeform::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    auto _ = std::adjacent_find(this->points.begin(), this->points.end(), [&](ImVec2 a, ImVec2 b) {
        draw_list->AddLine(offset + a * scale, offset + b * scale,
                           this->color, this->thickness * scale);
        draw_list->AddCircleFilled(offset + b * scale, this->thickness * scale / 2, this->color);
        return false;
    });
}

void Freeform::Update(ImVec2 pos) {
    this->points.push_back(pos);
}

Arrow::Arrow(ImVec2 start, ImU32 color, float thickness) {
    this->start = start;
    this->end = start;
    this->color = color;
    this->thickness = thickness;
}

// Vibe coded, probably bad
void Arrow::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    ImVec2 p0 = offset + this->start * scale;
    ImVec2 p1 = offset + this->end * scale;

    ImVec2 dir = p0 - p1;
    float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (length == 0.0f) return;
    dir.x /= length;
    dir.y /= length;

    const float thickness_scaled = this->thickness * scale;
    const float head_length = 4.0f * thickness_scaled;  // Head length proportional to thickness
    const float head_width = 3.0f * thickness_scaled;   // Head width proportional to thickness

    // Shift the end of the line back so it doesn't overlap the arrowhead
    ImVec2 line_end = ImVec2(
        p1.x + dir.x * head_length * 0.9,
        p1.y + dir.y * head_length * 0.9
    );

    draw_list->AddLine(p0, line_end, this->color, thickness_scaled);

    ImVec2 left = ImVec2(
        p1.x + dir.x * head_length - dir.y * head_width * 0.5f,
        p1.y + dir.y * head_length + dir.x * head_width * 0.5f
    );

    ImVec2 right = ImVec2(
        p1.x + dir.x * head_length + dir.y * head_width * 0.5f,
        p1.y + dir.y * head_length - dir.x * head_width * 0.5f
    );

    draw_list->AddTriangleFilled(p1, left, right, this->color);
}

void Arrow::Update(ImVec2 pos) {
    this->end = pos;
}

