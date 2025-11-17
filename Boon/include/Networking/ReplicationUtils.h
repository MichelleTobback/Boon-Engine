#pragma once

namespace Boon
{
    namespace ReplicationUtils
    {
        static constexpr float POS_PRECISION = 0.01f;   // 1 cm
        static constexpr float INV_POS_PRECISION = 1.0f / POS_PRECISION;
        static constexpr float POSITION_MIN = -327.68f;
        static constexpr float POSITION_MAX = 327.67f;
        static constexpr float PI = 3.14159265358979323846f;

        inline int16_t QuantizePos(float v)
        {
            v = std::clamp(v, POSITION_MIN, POSITION_MAX);
            return (int16_t)std::round(v * INV_POS_PRECISION);
        }

        inline float DequantizePos(int16_t q)
        {
            return (float)q * POS_PRECISION;
        }

        inline uint16_t QuantizeAngle(float radians)
        {
            // map radians to 0..1 range
            float norm = std::fmod(radians, 2.0f * PI);
            if (norm < 0) norm += 2.0f * PI;

            return (uint16_t)std::round((norm / (2.0f * PI)) * 65535.0f);
        }

        inline float DequantizeAngle(uint16_t q)
        {
            return (float(q) / 65535.0f) * (2.0f * PI);
        }

        inline uint16_t QuantizeAngleDeg(float degrees)
        {
            // normalize degrees to [0,360)
            float d = std::fmod(degrees, 360.0f);
            if (d < 0.0f)
                d += 360.0f;

            // scale to 0..65535
            float t = (d / 360.0f) * 65535.0f;
            return (uint16_t)std::round(t);
        }

        inline float DequantizeAngleDeg(uint16_t q)
        {
            return (float(q) / 65535.0f) * 360.0f;
        }

        inline float LerpAngleDegrees(float a, float b, float t)
        {
            float delta = fmodf(b - a + 540.0f, 360.0f) - 180.0f;
            return a + delta * t;
        }
    }
}