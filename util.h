#pragma once

#include <cmath>
#include <limits>
#include <ostream>

namespace guard
{

    class Approx
    {
    public:
        explicit Approx(double value)
            : m_value(value), m_epsilon(1e-12), m_scale(1.0)
        {
        }

        // Задать относительную погрешность
        Approx &epsilon(double eps)
        {
            m_epsilon = eps;
            return *this;
        }

        // Задать масштаб (как в doctest / Catch2)
        Approx &scale(double s)
        {
            m_scale = s;
            return *this;
        }

        Approx operator()(double value) const
        {
            Approx copy = *this;
            copy.m_value = value;
            return copy;
        }

        double value() const
        {
            return m_value;
        }
        double epsilon() const
        {
            return m_epsilon;
        }
        double scale() const
        {
            return m_scale;
        }

        // Для отладочных сообщений
        friend std::ostream &operator<<(std::ostream &os, const Approx &a)
        {
            os << "Approx(" << a.m_value << ", eps=" << a.m_epsilon
               << ", scale=" << a.m_scale << ")";
            return os;
        }

        // Сравнение "число == Approx(...)"
        friend bool operator==(double lhs, const Approx &rhs)
        {
            return rhs.compare(lhs);
        }

        friend bool operator==(const Approx &lhs, double rhs)
        {
            return lhs.compare(rhs);
        }

        friend bool operator!=(double lhs, const Approx &rhs)
        {
            return !rhs.compare(lhs);
        }

        friend bool operator!=(const Approx &lhs, double rhs)
        {
            return !lhs.compare(rhs);
        }

    private:
        bool compare(double other) const
        {
            const double diff = std::fabs(other - m_value);
            const double tolerance = m_epsilon * (m_scale + std::fabs(m_value));
            return diff <= tolerance;
        }

        double m_value;
        double m_epsilon;
        double m_scale;
    };

}
