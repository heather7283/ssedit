#include <cmath>

#include "shapes.hpp"

Line::Line(ImVec2 start, ImVec2 end, ImU32 color, float thickness) {
    this->start = start;
    this->end = end;
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

Circle::Circle(ImVec2 center, float radius, ImU32 color, float thickness) {
    this->center = center;
    this->radius = radius;
    this->color = color;
    this->thickness = thickness;
}

void Circle::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    draw_list->AddCircle(offset + this->center * scale, this->radius * scale,
                         this->color, 0, this->thickness * scale);
}

void Circle::Update(ImVec2 pos) {
    ImVec2 d = pos - this->center;
    this->radius = sqrt((d.x * d.x) + (d.y * d.y));
}

Rectangle::Rectangle(ImVec2 top_left, ImVec2 bottom_right, ImU32 color, float thickness) {
    this->start = top_left;
    this->end = bottom_right;
    this->color = color;
    this->thickness = thickness;
}

void Rectangle::Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const {
    draw_list->AddRect(offset + this->start * scale, offset + this->end * scale,
                       this->color, 0.0f, 0, this->thickness * scale);
}

void Rectangle::Update(ImVec2 pos) {
    this->end = pos;
}

