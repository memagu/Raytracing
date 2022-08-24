﻿#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Walnut/Image.h"

class Renderer
{
public:
    Renderer() = default;
    
    void Render();
    void OnResize(uint32_t width, uint32_t height);

    [[nodiscard]] std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
    
private:
    glm::vec4 PerPixel(glm::vec2 uv_coord);
    
private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
};