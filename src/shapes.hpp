#pragma once

#include <imgui/imgui.h>

class Shape {
public:
    virtual void Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const = 0;
    virtual void Update(ImVec2 pos) = 0;
    virtual ~Shape() = default;
};

class Line: public Shape {
public:
    Line(ImVec2 start, ImVec2 end, ImU32 color, float thickness);
    void Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const override;
    void Update(ImVec2 pos) override;
private:
    ImVec2 start;
    ImVec2 end;
    ImU32 color;
    float thickness;
};

class Circle: public Shape {
public:
    Circle(ImVec2 center, float radius, ImU32 color, float thickness);
    void Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const override;
    void Update(ImVec2 pos) override;
private:
    ImVec2 center;
    float radius;
    ImU32 color;
    float thickness;
};

class Rectangle: public Shape {
public:
    Rectangle(ImVec2 top_left, ImVec2 bottom_right, ImU32 color, float thickness);
    void Draw(ImDrawList *draw_list, ImVec2 offset, float scale) const override;
    void Update(ImVec2 pos) override;
private:
    ImVec2 start;
    ImVec2 end;
    ImU32 color;
    float thickness;
};

