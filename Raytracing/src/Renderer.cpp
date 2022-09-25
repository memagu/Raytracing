#include <iostream>

#include "Walnut/Random.h"

#include "Renderer.h"

namespace Utils
{
    static uint32_t ConvertToRGBA(const glm::vec4& color)
    {
        auto r = static_cast<uint8_t>(color.r * 255.0f);
        auto g = static_cast<uint8_t>(color.g * 255.0f);
        auto b = static_cast<uint8_t>(color.b * 255.0f);
        auto a = static_cast<uint8_t>(color.a * 255.0f);

        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }
}

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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    const glm::vec3& rayOrigin = camera.GetPosition();

    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
        {
            glm::vec2 coord(x, y);
            uint32_t i = y * m_FinalImage->GetWidth() + x;
            ray.Direction = camera.GetRayDirections()[i];

            float u = static_cast<float>(x) / m_FinalImage->GetWidth();
            float v = static_cast<float>(y) / m_FinalImage->GetHeight();
            glm::vec2 coord_uv = {u, v};
            glm::vec2 coord_mapped = {coord_uv * 2.0f - 1.0f};

            float aspectRatio = m_FinalImage->GetWidth() / static_cast<float>(m_FinalImage->GetHeight());
            coord_mapped.x *= aspectRatio;

            glm::vec4 color = TraceRay(scene, ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[i] = Utils::ConvertToRGBA(color);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
    if (scene.Spheres.empty())
        return glm::vec4(0.0f);

    const Sphere* closestSphere = nullptr;
    float hitDistance = std::numeric_limits<float>::max();
    for (const Sphere& sphere : scene.Spheres)
    {
        glm::vec3 origin = ray.Origin - sphere.Position;
        float a = glm::dot(ray.Direction, ray.Direction);
        float b = 2.0f * glm::dot(origin, ray.Direction);
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

        //b^2-4ac
        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
            continue;

        float t[2] = {
            (-b - glm::sqrt(discriminant)) / (2 * a),
            (-b + glm::sqrt(discriminant)) / (2 * a)
        };

        if (t[0] < hitDistance)
        {
            hitDistance = t[0];
            closestSphere = &sphere;
        }
    }
    if (closestSphere == nullptr)
        return glm::vec4(0);

    const Sphere& sphere = *closestSphere;

    glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    // glm::vec3 sphereOrigin = glm::vec3(-0.5f, 0.0f, 0.0f);
    // float sphereRadius = 0.5f;


    // (bx^2+by^2+bz^2)t^2 + (axbx + ayby + azbz)2t + (ax^2 + ay^2 + az^2 - r^2) = 0
    //                   a                        b                            c
    // a = ray origin
    // b = ray direction
    // r = spehere radius
    // t = hit distance

    // (bx^2+by^2+bz^2)t^2 + (axbx+ayby+azbz - (bxox+byoy+bzoz))2t + ax^2+bx^2+az^2 - 2(axox +ayoy+azoz) + ox^2+oy^2+oz^2


    glm::vec3 origin = ray.Origin - sphere.Position;
    glm::vec3 hitPosition = origin + ray.Direction * hitDistance;
    glm::vec3 hitNormal = glm::normalize(hitPosition);
    
    // std::cout << hitNormal[0].x << ", " << hitNormal[0].y << ", " << hitNormal[0].z << ", " << std::endl;
    // std::cout << t[0] << ", " << t[0]*t[0] << std::endl;
    float brightness = glm::max(glm::dot(lightDirection, hitNormal) / (glm::min(hitDistance * hitDistance, 1.0f)), 0.0f);
    // glm::vec3 sphereColor = glm::normalize(hitPosition[0]) * 0.5f + 0.5f; 
    //glm::vec3 sphereColor(1, 1, 1);
    glm::vec3 sphereColor = closestSphere->Albedo;
    return glm::vec4(sphereColor * brightness, 1.0f);
}
