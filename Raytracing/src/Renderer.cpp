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
    m_ActiveScene = &scene;
    m_ActiveCamera = &camera;

    const glm::vec3& rayOrigin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
        {
            glm::vec2 coord(x, y);
            uint32_t i = y * m_FinalImage->GetWidth() + x;

            float u = static_cast<float>(x) / m_FinalImage->GetWidth();
            float v = static_cast<float>(y) / m_FinalImage->GetHeight();
            glm::vec2 coord_uv = {u, v};
            glm::vec2 coord_mapped = {coord_uv * 2.0f - 1.0f};

            float aspectRatio = m_FinalImage->GetWidth() / static_cast<float>(m_FinalImage->GetHeight());
            coord_mapped.x *= aspectRatio;

            glm::vec4 color = PerPixel(x, y);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[i] = Utils::ConvertToRGBA(color);
        }
    }

    m_FinalImage->SetData(m_ImageData);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
    int closestSphere = -1;
    float hitDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
    {
        const Sphere& sphere = m_ActiveScene->Spheres[i];
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

        if (t[0] > 0.0f && t[0] < hitDistance)
        {
            hitDistance = t[0];
            closestSphere = static_cast<int>(i);
        }
    }
    if (closestSphere < 0)
        return Miss(ray);

    return ClosestHit(ray, hitDistance, closestSphere);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

    glm::vec3 color = glm::vec3(0.0f);

    float multiplier = 1.0f;

    int bounces = 50;
    for (int i = 0; i < bounces; i++)
    {
        Renderer::HitPayload payload = TraceRay(ray);

        if (payload.HitDistance < 0.0f)
        {
            glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
            // glm::vec3 skyColor = glm::vec3(0.0f);
            color += skyColor * multiplier;
            break;
        }

        glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));


        // float brightness = glm::max(
        //     glm::dot(-lightDirection, payload.WorldNormal) / (
        //         glm::min(payload.HitDistance * payload.HitDistance, 1.0f)),
        //     0.0f);

        float brightness = glm::max(
            glm::dot(-lightDirection, payload.WorldNormal),
            0.0f);

        const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
        const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];
        glm::vec3 sphereColor = material.Albedo;
        color += sphereColor * brightness * multiplier;
        
        multiplier *= 0.5f;

        ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
        ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5));
    }

    return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
    Renderer::HitPayload payload;
    payload.HitDistance = hitDistance;
    payload.ObjectIndex = objectIndex;

    const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

    glm::vec3 origin = ray.Origin - closestSphere.Position;
    payload.WorldPosition = origin + ray.Direction * hitDistance;
    payload.WorldNormal = glm::normalize(payload.WorldPosition);

    payload.WorldPosition += closestSphere.Position;

    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
    Renderer::HitPayload payload;
    payload.HitDistance = -1.0f;
    return payload;
}
