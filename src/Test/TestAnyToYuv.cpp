/*
* Tests for Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2019 Yermalayeu Ihar,
*               2014-2015 Antonenka Mikhail.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/TestData.h"

namespace Test
{
    namespace
    {
        struct FuncYuv
        {
            typedef void(*FuncPtr)(const uint8_t * bgr, size_t width, size_t height, size_t bgrStride, uint8_t * y, size_t yStride,
                uint8_t * u, size_t uStride, uint8_t * v, size_t vStride);

            FuncPtr func;
            String description;

            FuncYuv(const FuncPtr & f, const String & d) : func(f), description(d) {}

            void Call(const View & bgr, View & y, View & u, View & v) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(bgr.data, bgr.width, bgr.height, bgr.stride, y.data, y.stride, u.data, u.stride, v.data, v.stride);
            }
        };
    }

#define FUNC_YUV(function) FuncYuv(function, #function)

    bool AnyToYuvAutoTest(int width, int height, View::Format srcType, int dx, int dy, const FuncYuv & f1, const FuncYuv & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        const int uvWidth = width / dx;
        const int uvHeight = height / dy;

        View src(width, height, srcType, NULL, TEST_ALIGN(width));
        FillRandom(src);

        View y1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));

        View y2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, y1, u1, v1));
        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, y2, u2, v2));

        result = result && Compare(y1, y2, 0, true, 64, 0, "y");
        result = result && Compare(u1, u2, 0, true, 64, 0, "u");
        result = result && Compare(v1, v2, 0, true, 64, 0, "v");

        return result;
    }

    bool AnyToYuvAutoTest(View::Format srcType, int dx, int dy, const FuncYuv & f1, const FuncYuv & f2)
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(W, H, srcType, dx, dy, f1, f2);
        result = result && AnyToYuvAutoTest(W + O*dx, H - O*dy, srcType, dx, dy, f1, f2);

        return result;
    }

    bool BgraToYuv420pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Base::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));

#ifdef SIMD_SSE2_ENABLE
        if (Simd::Sse2::Enable && W >= Simd::Sse2::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Sse2::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif 

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Ssse3::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Avx2::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Avx512bw::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Vmx::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 2, FUNC_YUV(Simd::Neon::BgraToYuv420p), FUNC_YUV(SimdBgraToYuv420p));
#endif

        return result;
    }

    bool BgraToYuv422pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Base::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));

#ifdef SIMD_SSE2_ENABLE
        if (Simd::Sse2::Enable && W >= Simd::Sse2::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Sse2::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif 

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Ssse3::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Avx2::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Avx512bw::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Vmx::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::DA)
            result = result && AnyToYuvAutoTest(View::Bgra32, 2, 1, FUNC_YUV(Simd::Neon::BgraToYuv422p), FUNC_YUV(SimdBgraToYuv422p));
#endif

        return result;
    }

    bool BgraToYuv444pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Base::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));

#ifdef SIMD_SSE2_ENABLE
        if (Simd::Sse2::Enable && W >= Simd::Sse2::A)
            result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Sse2::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::A)
            result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Avx2::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Avx512bw::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::A)
            result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Vmx::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::A)
            result = result && AnyToYuvAutoTest(View::Bgra32, 1, 1, FUNC_YUV(Simd::Neon::BgraToYuv444p), FUNC_YUV(SimdBgraToYuv444p));
#endif

        return result;
    }

    bool BgrToYuv420pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Base::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Ssse3::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Avx2::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Avx512bw::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Vmx::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 2, FUNC_YUV(Simd::Neon::BgrToYuv420p), FUNC_YUV(SimdBgrToYuv420p));
#endif

        return result;
    }

    bool BgrToYuv422pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Base::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Ssse3::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Avx2::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Avx512bw::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Vmx::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::DA)
            result = result && AnyToYuvAutoTest(View::Bgr24, 2, 1, FUNC_YUV(Simd::Neon::BgrToYuv422p), FUNC_YUV(SimdBgrToYuv422p));
#endif

        return result;
    }

    bool BgrToYuv444pAutoTest()
    {
        bool result = true;

        result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Base::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::A)
            result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Ssse3::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::A)
            result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Avx2::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Avx512bw::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));
#endif

#ifdef SIMD_VMX_ENABLE
        if (Simd::Vmx::Enable && W >= Simd::Vmx::A)
            result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Vmx::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::A)
            result = result && AnyToYuvAutoTest(View::Bgr24, 1, 1, FUNC_YUV(Simd::Neon::BgrToYuv444p), FUNC_YUV(SimdBgrToYuv444p));
#endif

        return result;
    }

    namespace
    {
        struct FuncYuva
        {
            typedef void(*FuncPtr)(const uint8_t * bgra, size_t bgraStride, size_t width, size_t height, uint8_t * y, size_t yStride,
                uint8_t * u, size_t uStride, uint8_t * v, size_t vStride, uint8_t * a, size_t aStride);

            FuncPtr func;
            String description;

            FuncYuva(const FuncPtr & f, const String & d) : func(f), description(d) {}

            void Call(const View & bgra, View & y, View & u, View & v, View & a) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(bgra.data, bgra.stride, bgra.width, bgra.height, y.data, y.stride, u.data, u.stride, v.data, v.stride, a.data, a.stride);
            }
        };
    }

#define FUNC_YUVA(function) FuncYuva(function, #function)

    bool BgraToYuvaAutoTest(int width, int height, const FuncYuva & f1, const FuncYuva & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        const int uvWidth = width / 2;
        const int uvHeight = height / 2;

        View bgra(width, height, View::Bgra32, NULL, TEST_ALIGN(width));
        FillRandom(bgra);

        View y1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View a1(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        View y2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View a2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(bgra, y1, u1, v1, a1));
        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(bgra, y2, u2, v2, a2));

        result = result && Compare(y1, y2, 0, true, 64, 0, "y");
        result = result && Compare(u1, u2, 0, true, 64, 0, "u");
        result = result && Compare(v1, v2, 0, true, 64, 0, "v");
        result = result && Compare(a1, a2, 0, true, 64, 0, "a");

        return result;
    }

    bool BgraToYuvaAutoTest(const FuncYuva & f1, const FuncYuva & f2)
    {
        bool result = true;

        result = result && BgraToYuvaAutoTest(W, H, f1, f2);
        result = result && BgraToYuvaAutoTest(W + E, H - E, f1, f2);

        return result;
    }

    bool BgraToYuva420pAutoTest()
    {
        bool result = true;

        result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Base::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));

#ifdef SIMD_SSE2_ENABLE
        if (Simd::Sse2::Enable && W >= Simd::Sse2::DA)
            result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Sse2::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));
#endif 

#ifdef SIMD_SSSE3_ENABLE
        if (Simd::Ssse3::Enable && W >= Simd::Ssse3::DA)
            result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Ssse3::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if (Simd::Avx2::Enable && W >= Simd::Avx2::DA)
            result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Avx2::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));
#endif

#ifdef SIMD_AVX512BW_ENABLE
        if (Simd::Avx512bw::Enable)
            result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Avx512bw::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));
#endif

#ifdef SIMD_NEON_ENABLE
        if (Simd::Neon::Enable && W >= Simd::Neon::DA)
            result = result && BgraToYuvaAutoTest(FUNC_YUVA(Simd::Neon::BgraToYuva420p), FUNC_YUVA(SimdBgraToYuva420p));
#endif

        return result;
    }

    //-----------------------------------------------------------------------

    bool AnyToYuvDataTest(bool create, int width, int height, View::Format srcType, int dx, int dy, const FuncYuv & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        const int uvWidth = width / dx;
        const int uvHeight = height / dy;

        View src(width, height, srcType, NULL, TEST_ALIGN(width));

        View y1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v1(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));

        View y2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View u2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));
        View v2(uvWidth, uvHeight, View::Gray8, NULL, TEST_ALIGN(uvWidth));

        if (create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, y1, u1, v1);

            TEST_SAVE(y1);
            TEST_SAVE(u1);
            TEST_SAVE(v1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(y1);
            TEST_LOAD(u1);
            TEST_LOAD(v1);

            f.Call(src, y2, u2, v2);

            TEST_SAVE(y2);
            TEST_SAVE(u2);
            TEST_SAVE(v2);

            result = result && Compare(y1, y2, 0, true, 64, 0, "y");
            result = result && Compare(u1, u2, 0, true, 64, 0, "u");
            result = result && Compare(v1, v2, 0, true, 64, 0, "v");
        }

        return result;
    }

    bool BgraToYuv420pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgra32, 2, 2, FUNC_YUV(SimdBgraToYuv420p));

        return result;
    }

    bool BgraToYuv422pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgra32, 2, 1, FUNC_YUV(SimdBgraToYuv422p));

        return result;
    }

    bool BgraToYuv444pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgra32, 1, 1, FUNC_YUV(SimdBgraToYuv444p));

        return result;
    }

    bool BgrToYuv420pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgr24, 2, 2, FUNC_YUV(SimdBgrToYuv420p));

        return result;
    }

    bool BgrToYuv422pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgr24, 2, 1, FUNC_YUV(SimdBgrToYuv422p));

        return result;
    }

    bool BgrToYuv444pDataTest(bool create)
    {
        bool result = true;

        result = result && AnyToYuvDataTest(create, DW, DH, View::Bgr24, 1, 1, FUNC_YUV(SimdBgrToYuv444p));

        return result;
    }
}
