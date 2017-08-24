#pragma once

#include <Eigen/Core>
#include "opengl.hpp"

// Define command key for windows/mac/linux
#ifdef __APPLE__
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

namespace psychicui {
    using Eigen::Vector2f;
    using Eigen::Vector3f;
    using Eigen::Vector4f;
    using Eigen::Vector2i;
    using Eigen::Vector3i;
    using Eigen::Vector4i;
    using Eigen::Matrix3f;
    using Eigen::Matrix4f;
    using Eigen::VectorXf;
    using Eigen::MatrixXf;

    enum class Cursor {
        Arrow = 0,
        IBeam,
        Crosshair,
        Hand,
        HResize,
        VResize,
        /// Not a cursor --- should always be last: enables a loop over the cursor types.
        CursorCount
    };

    class Color : public Eigen::Vector4f {
        typedef Eigen::Vector4f Base;
    public:
        Color() : Color(0, 0, 0, 0) {}

        Color(const Eigen::Vector4f &color) : Eigen::Vector4f(color) {}

        Color(const Eigen::Vector3f &color, float alpha) : Color(color(0), color(1), color(2), alpha) {}

        Color(const Eigen::Vector3i &color, int alpha) : Color(color.cast<float>() / 255.f, alpha / 255.f) {}

        Color(const Eigen::Vector3f &color) : Color(color, 1.0f) {}

        Color(const Eigen::Vector3i &color) : Color((Vector3f) (color.cast<float>() / 255.f)) {}

        Color(const Eigen::Vector4i &color) : Color((Vector4f) (color.cast<float>() / 255.f)) {}

        Color(float intensity, float alpha) : Color(Vector3f::Constant(intensity), alpha) {}

        Color(int intensity, int alpha) : Color(Vector3i::Constant(intensity), alpha) {}

        Color(float r, float g, float b, float a) : Color(Vector4f(r, g, b, a)) {}

        Color(int r, int g, int b, int a) : Color(Vector4i(r, g, b, a)) {}

        /// Construct a color vector from MatrixBase (needed to play nice with Eigen)
        template<typename Derived> Color(const Eigen::MatrixBase<Derived> &p) : Base(p) {}

        /// Assign a color vector from MatrixBase (needed to play nice with Eigen)
        template<typename Derived> Color &operator=(const Eigen::MatrixBase<Derived> &p) {
            this->Base::operator=(p);
            return *this;
        }

        float &r() { return x(); }

        const float &r() const { return x(); }

        float &g() { return y(); }

        const float &g() const { return y(); }

        float &b() { return z(); }

        const float &b() const { return z(); }

        float &a() { return w(); }

        const float &a() const { return w(); }

        Color contrastingColor() const {
            float luminance = cwiseProduct(Color(0.299f, 0.587f, 0.144f, 0.f)).sum();
            return Color(luminance < 0.5f ? 1.f : 0.f, 1.f);
        }

        inline operator const NVGcolor &() const {
            return reinterpret_cast<const NVGcolor &>(*this->data());
        }

    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    void init();
    void mainloop();
    void shutdown();

}