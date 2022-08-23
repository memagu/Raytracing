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

        float u = static_cast<float>(x) / m_FinalImage->GetWidth();
        float v = static_cast<float>(y) / m_FinalImage->GetHeight();
        
        m_ImageData[i] = PerPixel(glm::vec2(u, v));
    }

    m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 uv_coord)
{
    uint8_t r, g, b, a;
    
    r = static_cast<uint8_t>(uv_coord.x * 255.0f);
    g = static_cast<uint8_t>(uv_coord.y * 255.0f);
    b = 0x00;
    a = 0xff;
    
    return (0x00000000u | (a << 24) | (b << 16) | (g << 8) | r);
} 