#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cmath>
// A single closed, smooth stingray body. Upper and lower surfaces meet at the fin edge.
inline Mesh makeRayBody() {
    constexpr int rings = 18, sides = 96;
    Mesh m{};
    m.vertexCount = 2 * rings * sides * 6;
    m.triangleCount = m.vertexCount / 3;
    m.vertices = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
    m.normals = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
    m.texcoords = (float *)MemAlloc(m.vertexCount * 2 * sizeof(float));
    m.colors = (unsigned char *)MemAlloc(m.vertexCount * 4);
    auto point = [](float r, float a, bool top) {
        float s = std::sin(a), c = std::cos(a);
        float x = 2.05f * s * r,
              z = (c * (c > 0 ? .98f : 1.12f) - .36f * std::pow(std::abs(s), 3.f)) * r;
        float center = std::exp(-x * x * 1.4f) * (1 - r * r);
        float edge = .13f + .26f * r * r * r * std::pow(std::abs(s), 4.f);
        return Vector3{x, edge + (top ? .61f : -.16f) * center, z};
    };
    int n = 0;
    auto put = [&](float r, float a, bool top) {
        Vector3 p = point(r, a, top);
        float e = .001f;
        Vector3 tangent = Vector3Subtract(point(std::max(.001f, r), a + e, top),
                                          point(std::max(.001f, r), a - e, top));
        Vector3 radial = Vector3Subtract(point(r + e, a, top), point(r - e, a, top));
        Vector3 normal = Vector3Normalize(Vector3CrossProduct(tangent, radial));
        if ((top && normal.y < 0) || (!top && normal.y > 0))
            normal = Vector3Negate(normal);
        m.vertices[n * 3] = p.x;
        m.vertices[n * 3 + 1] = p.y;
        m.vertices[n * 3 + 2] = p.z;
        m.normals[n * 3] = normal.x;
        m.normals[n * 3 + 1] = normal.y;
        m.normals[n * 3 + 2] = normal.z;
        m.texcoords[n * 2] = p.x / 4.2f + .5f;
        m.texcoords[n * 2 + 1] = p.z / 2.4f + .5f;
        Color color = top ? Color{35, (unsigned char)(153 + 30 * r), (unsigned char)(179 + 25 * r), 255}
                          : Color{229, 234, 202, 255};
        for (int j = 0; j < 4; ++j)
            m.colors[n * 4 + j] = ((unsigned char *)&color)[j];
        ++n;
    };
    for (bool top : {true, false})
        for (int r = 0; r < rings; ++r)
            for (int a = 0; a < sides; ++a) {
                float r0 = float(r) / rings, r1 = float(r + 1) / rings, a0 = a * 2 * PI / sides,
                      a1 = (a + 1) * 2 * PI / sides;
                if (top) {
                    put(r0, a0, top);
                    put(r1, a1, top);
                    put(r1, a0, top);
                    put(r0, a0, top);
                    put(r0, a1, top);
                    put(r1, a1, top);
                } else {
                    put(r0, a0, top);
                    put(r1, a0, top);
                    put(r1, a1, top);
                    put(r0, a0, top);
                    put(r1, a1, top);
                    put(r0, a1, top);
                }
            }
    UploadMesh(&m, false);
    return m;
}
