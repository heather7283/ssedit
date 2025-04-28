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

Circle::Circle(ImVec2 center, ImU32 color, float thickness) {
    this->center = center;
    this->radius = 0;
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

Rectangle::Rectangle(ImVec2 start, ImU32 color, float thickness) {
    this->start = start;
    this->end = start;
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

