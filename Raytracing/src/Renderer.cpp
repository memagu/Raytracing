#include <cmath>
#include <iostream>

#include "Walnut/Random.h"

#include "Renderer.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
    if (m_FinalImage)
    {
        if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
            return;

        m_FinalImage->Resize(width, height);
    }
    else
    {
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
    for (uint32_t i = 0; i < m_FinalImage->GetWidth() * m_FinalImage->GetHeight(); i++)
    {
        uint32_t y = std::floor(i / m_FinalImage->GetWidth());
        uint32_t x = i % m_FinalImage->GetWidth();
        glm::vec2 coord = {x, y};

        float u = static_cast<float>(x) / m_FinalImage->GetWidth();
        float v = static_cast<float>(y) / m_FinalImage->GetHeight();
        glm::vec2 coord_uv = {u, v};
        glm::vec2 coord_mapped = {coord_uv * 2.0f - 1.0f};

        m_ImageData[i] = PerPixel(coord_mapped);
    }

    m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
    // uint8_t R, G, B, A;
    //
    // R = static_cast<uint8_t>(coord.x * 255.0f);
    // G = static_cast<uint8_t>(coord.y * 255.0f);
    // B = 0x00;
    // A = 0xff;

    glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
    rayDirection = glm::normalize(rayDirection);
    float radius = 0.5f;

    // (bx^2+by^2+bz^2)t^2 + (axbx + ayby + azbz)2t + (ax^2 + ay^2 + az^2 - r^2) = 0
    //                   a                        b                            c
    // a = ray origin
    // b = ray direction
    // r = spehere radius
    // t = hit distance

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(rayOrigin, rayDirection);
    float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

    //b^2-4ac

    float discriminant = b * b - 4.0f * a * c;

    float dist = (-b + glm::sqrt(discriminant)) / 2 * a;

    if (discriminant >= 0)
        return 0xffffffffu / dist;

    // return (0x00000000u | (A << 24) | (B << 16) | (G << 8) | R);
    return 0x00000000u;
}
