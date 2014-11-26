#pragma once

namespace gep{
    template <typename T>
    struct vec4_t {
        union {
            struct {
                T x,y,z,w;
            };
            T data[4];
        };
		static const size_t dimension =4;

		inline vec4_t()
		{
		    this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
			this->w = 0.0f;
		}

		inline vec4_t(DoNotInitialize) {}

		inline vec4_t(T x, T y, T z, T w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
			this->w = w;
        }
    };
    typedef vec4_t<float> vec4;
}
