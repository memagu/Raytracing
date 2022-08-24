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
        uint32_t y = glm::floor(i / m_FinalImage->GetWidth());
        uint32_t x = i % m_FinalImage->GetWidth();
        glm::vec2 coord = {x, y};

        float aspectRatio = m_FinalImage->GetWidth() / static_cast<float>(m_FinalImage->GetHeight());
        
        float u = static_cast<float>(x) / m_FinalImage->GetWidth();
        float v = static_cast<float>(y) / m_FinalImage->GetHeight();
        glm::vec2 coord_uv = {u, v};
        glm::vec2 coord_mapped = {coord_uv * 2.0f - 1.0f};
        coord_mapped.x *= aspectRatio;

        m_ImageData[i] = PerPixel(coord_mapped);
    }

    m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
    glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(coord.x, coord.y, -1.0f));
    glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, 1.0f, -0.1f));
    glm::vec3 sphereOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
    float sphereRadius = 0.5f;
    

    // (bx^2+by^2+bz^2)t^2 + (axbx + ayby + azbz)2t + (ax^2 + ay^2 + az^2 - r^2) = 0
    //                   a                        b                            c
    // a = ray origin
    // b = ray direction
    // r = spehere radius
    // t = hit distance

    // (bx^2+by^2+bz^2)t^2 + (axbx+ayby+azbz - (bxox+byoy+bzoz))2t + ax^2+bx^2+az^2 - 2(axox +ayoy+azoz) + ox^2+oy^2+oz^2

    glm::vec3 oc = rayOrigin - sphereOrigin;
    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(oc, rayDirection);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    //b^2-4ac
    float discriminant = b*b - 4.0f*a*c;
    
    if (discriminant >= 0.0f)
    {
        float t[2] = {(-b + glm::sqrt(discriminant)) / (2 * a),
                      (-b - glm::sqrt(discriminant)) / (2 * a)};

        glm::vec3 hitPosition[2];
        glm::vec3 hitNormal[2];
        
        for (int i = 0; i < 2; i++)
        {
            hitPosition[i] = rayOrigin + rayDirection * t[i];
            hitNormal[i] = glm::normalize(hitPosition[i] - sphereOrigin);
        }

        // float hitDepth = glm::distance(hitPosition[0], hitPosition[1]);

        float brightness = (glm::dot(lightDirection, hitNormal[0]) + 1) / 2;
        
        auto R = static_cast<uint8_t>(0xff * brightness);
        auto G = static_cast<uint8_t>(0xff * brightness);
        auto B = static_cast<uint8_t>(0xff * brightness);
        auto A = static_cast<uint8_t>(0xff);
        
         return (0x00000000u | (A << 24) | (B << 16) | (G << 8) | R);
        
    }

    // if (discriminant >= 0)
    //     return 0xffffffffu;
    
    return 0x00000000u;
}
