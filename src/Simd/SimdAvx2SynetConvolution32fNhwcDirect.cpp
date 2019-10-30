/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2019 Yermalayeu Ihar.
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
#include "Simd/SimdSynetConvolution32f.h"
#include "Simd/SimdSynetConvolution32fCommon.h"

namespace Simd
{
#ifdef SIMD_AVX2_ENABLE    
    namespace Avx2
    {
        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect_2x6(const float * src0, const ConvParam32f & p,
            size_t kernelH, size_t kernelW, size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, d10, d11, d20, d21, d30, d31, d40, d41, d50, d51, s0, w0, w1;
            size_t dS = p.srcC * p.strideX, dW = DF * (p.kernelX - kernelW)*srcC, dY = p.srcW * p.srcC, dX = p.srcC, dD = p.dstC;
            const float * src1 = src0 + 1 * dS;
            const float * src2 = src0 + 2 * dS;
            const float * src3 = src0 + 3 * dS;
            const float * src4 = src0 + 4 * dS;
            const float * src5 = src0 + 5 * dS;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps(); d01 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps(); d11 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps(); d21 = _mm256_setzero_ps();
                d30 = _mm256_setzero_ps(); d31 = _mm256_setzero_ps();
                d40 = _mm256_setzero_ps(); d41 = _mm256_setzero_ps();
                d50 = _mm256_setzero_ps(); d51 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            w1 = _mm256_loadu_ps(weight + F);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            d01 = _mm256_fmadd_ps(s0, w1, d01);
                            s0 = _mm256_set1_ps(src1[offset]);
                            d10 = _mm256_fmadd_ps(s0, w0, d10);
                            d11 = _mm256_fmadd_ps(s0, w1, d11);
                            s0 = _mm256_set1_ps(src2[offset]);
                            d20 = _mm256_fmadd_ps(s0, w0, d20);
                            d21 = _mm256_fmadd_ps(s0, w1, d21);
                            s0 = _mm256_set1_ps(src3[offset]);
                            d30 = _mm256_fmadd_ps(s0, w0, d30);
                            d31 = _mm256_fmadd_ps(s0, w1, d31);
                            s0 = _mm256_set1_ps(src4[offset]);
                            d40 = _mm256_fmadd_ps(s0, w0, d40);
                            d41 = _mm256_fmadd_ps(s0, w1, d41);
                            s0 = _mm256_set1_ps(src5[offset]);
                            d50 = _mm256_fmadd_ps(s0, w0, d50);
                            d51 = _mm256_fmadd_ps(s0, w1, d51);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d31, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d41, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d51, bias, params);
                }
                else
                {
                    dstC -= F;
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d31, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d41, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d51, bias, params, dstC);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps();
                d30 = _mm256_setzero_ps();
                d40 = _mm256_setzero_ps();
                d50 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            s0 = _mm256_set1_ps(src1[offset]);
                            d10 = _mm256_fmadd_ps(s0, w0, d10);
                            s0 = _mm256_set1_ps(src2[offset]);
                            d20 = _mm256_fmadd_ps(s0, w0, d20);
                            s0 = _mm256_set1_ps(src3[offset]);
                            d30 = _mm256_fmadd_ps(s0, w0, d30);
                            s0 = _mm256_set1_ps(src4[offset]);
                            d40 = _mm256_fmadd_ps(s0, w0, d40);
                            s0 = _mm256_set1_ps(src5[offset]);
                            d50 = _mm256_fmadd_ps(s0, w0, d50);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == F)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                }
                else
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params, dstC);
                }
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect_2x3(const float * src0, const ConvParam32f & p,
            size_t kernelH, size_t kernelW, size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, d10, d11, d20, d21, s0, w0, w1;
            size_t dS = p.srcC * p.strideX, dW = DF * (p.kernelX - kernelW)*srcC, dY = p.srcW * p.srcC, dX = p.srcC, dD = p.dstC;
            const float * src1 = src0 + 1 * dS;
            const float * src2 = src0 + 2 * dS;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps(); d01 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps(); d11 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps(); d21 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            w1 = _mm256_loadu_ps(weight + F);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            d01 = _mm256_fmadd_ps(s0, w1, d01);
                            s0 = _mm256_set1_ps(src1[offset]);
                            d10 = _mm256_fmadd_ps(s0, w0, d10);
                            d11 = _mm256_fmadd_ps(s0, w1, d11);
                            s0 = _mm256_set1_ps(src2[offset]);
                            d20 = _mm256_fmadd_ps(s0, w0, d20);
                            d21 = _mm256_fmadd_ps(s0, w1, d21);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params);
                }
                else
                {
                    dstC -= F;
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params, dstC);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            s0 = _mm256_set1_ps(src1[offset]);
                            d10 = _mm256_fmadd_ps(s0, w0, d10);
                            s0 = _mm256_set1_ps(src2[offset]);
                            d20 = _mm256_fmadd_ps(s0, w0, d20);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == F)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                }
                else
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params, dstC);
                }
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect_2x1(const float * src0, const ConvParam32f & p,
            size_t kernelH, size_t kernelW, size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, s0, w0, w1;
            size_t dW = DF * (p.kernelX - kernelW)*srcC, dY = p.srcW * p.srcC, dX = p.srcC;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps();
                d01 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            w1 = _mm256_loadu_ps(weight + F);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            d01 = _mm256_fmadd_ps(s0, w1, d01);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                }
                else
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC - F);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                for (size_t ky = 0; ky < kernelH; ++ky)
                {
                    for (size_t kx = 0; kx < kernelW; ++kx)
                    {
                        for (size_t offset = ky * dY + kx * dX, end = offset + srcC; offset < end; ++offset)
                        {
                            w0 = _mm256_loadu_ps(weight + 0);
                            s0 = _mm256_set1_ps(src0[offset]);
                            d00 = _mm256_fmadd_ps(s0, w0, d00);
                            weight += DF;
                        }
                    }
                    weight += dW;
                }
                if (dstC == F)
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                else
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect_2(const float * src, const ConvParam32f & p,
            size_t dstC, size_t yBeg, size_t yEnd, size_t srcC, const float * weight, const float * bias, const float * params, float * dst)
        {
            size_t noseH = p.padY, noseW = p.padX;
            size_t bodyH = p.srcH - p.kernelY + 1 + noseH, bodyW = p.srcW - p.kernelX + 1 + noseW;
            size_t bodyW3 = AlignLoAny(bodyW - noseW, 3 * p.strideX) + noseW;
            size_t bodyW6 = AlignLoAny(bodyW - noseW, 6 * p.strideX) + noseW;
            size_t tailH = bodyH + p.padH, tailW = bodyW + p.padW;
            size_t kY = p.kernelY - noseH, kX = p.kernelX - noseW, kH = bodyH + p.kernelY - 1, kW = bodyW + p.kernelX - 1;

            __m256 _params[2], _bias[2];
            _params[0] = _mm256_set1_ps(params[0]);
            if (type == ::SimdConvolutionActivationRestrictRange)
                _params[1] = _mm256_set1_ps(params[1]);

            for (size_t dc = 0; dc < dstC; dc += DF)
            {
                size_t dC = Simd::Min(DF, dstC - dc);
                _bias[0] = _mm256_loadu_ps(bias + dc + 0);
                _bias[1] = _mm256_loadu_ps(bias + dc + F);
                if (type == ::SimdConvolutionActivationPrelu)
                {
                    _params[0] = _mm256_loadu_ps(params + dc + 0);
                    _params[1] = _mm256_loadu_ps(params + dc + F);
                }
                float * d = dst + dc + yBeg*p.dstW*p.dstC;
                size_t dy = yBeg, sy = dy * p.strideY;
                for (; sy < noseH && dy < yEnd; sy += p.strideY, dy++)
                {
                    size_t sx = 0;
                    const float * s = src;
                    const float * w = weight + (noseH - sy) * p.kernelX * srcC * DF;
                    for (; sx < noseW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s, p, kY + sy, kX + sx, srcC, dC, w + (noseW - sx)*srcC*DF, _bias, _params, d);
                    for (; sx < bodyW6; sx += 6 * p.strideX, d += 6 * p.dstC)
                        ConvolutionNhwcDirect_2x6<term, type>(s + (sx - noseW) * p.srcC, p, kY + sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW3; sx += 3 * p.strideX, d += 3 * p.dstC)
                        ConvolutionNhwcDirect_2x3<term, type>(s + (sx - noseW) * p.srcC, p, kY + sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, kY + sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < tailW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, kY + sy, kW - sx, srcC, dC, w, _bias, _params, d);
                }
                for (; sy < bodyH && dy < yEnd; sy += p.strideY, dy++)
                {
                    size_t sx = 0;
                    const float * s = src + (sy - noseH)*p.srcW*p.srcC;
                    const float * w = weight;
                    for (; sx < noseW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s, p, p.kernelY, kX + sx, srcC, dC, w + (noseW - sx)*srcC*DF, _bias, _params, d);
                    for (; sx < bodyW6; sx += 6 * p.strideX, d += 6 * p.dstC)
                        ConvolutionNhwcDirect_2x6<term, type>(s + (sx - noseW) * p.srcC, p, p.kernelY, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW3; sx += 3 * p.strideX, d += 3 * p.dstC)
                        ConvolutionNhwcDirect_2x3<term, type>(s + (sx - noseW) * p.srcC, p, p.kernelY, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, p.kernelY, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < tailW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, p.kernelY, kW - sx, srcC, dC, w, _bias, _params, d);
                }
                for (; sy < tailH && dy < yEnd; sy += p.strideY, dy++)
                {
                    size_t sx = 0;
                    const float * s = src + (sy - noseH)*p.srcW*p.srcC;
                    const float * w = weight;
                    for (; sx < noseW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s, p, kH - sy, kX + sx, srcC, dC, w + (noseW - sx)*srcC*DF, _bias, _params, d);
                    for (; sx < bodyW6; sx += 6 * p.strideX, d += 6 * p.dstC)
                        ConvolutionNhwcDirect_2x6<term, type>(s + (sx - noseW) * p.srcC, p, kH - sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW3; sx += 3 * p.strideX, d += 3 * p.dstC)
                        ConvolutionNhwcDirect_2x3<term, type>(s + (sx - noseW) * p.srcC, p, kH - sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < bodyW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, kH - sy, p.kernelX, srcC, dC, w, _bias, _params, d);
                    for (; sx < tailW; sx += p.strideX, d += p.dstC)
                        ConvolutionNhwcDirect_2x1<term, type>(s + (sx - noseW) * p.srcC, p, kH - sy, kW - sx, srcC, dC, w, _bias, _params, d);
                }
                weight += p.kernelY * p.kernelX*srcC*DF;
            }
        }

        template<SimdConvolutionActivationType type> void ConvolutionNhwcDirect_2(const float * src, const ConvParam32f & p,
            const SynetConvolution32fNhwcDirect::AlgParam & a, const float * weight, const float * bias, const float * params, float * dst)
        {
            for (size_t dc = 0; dc < p.dstC; dc += a.macroD)
            {
                size_t macroD = Simd::Min(p.dstC, dc + a.macroD) - dc;
                for (size_t sc = 0; sc < p.srcC; sc += a.macroC)
                {
                    size_t macroC = Simd::Min(p.srcC, sc + a.macroC) - sc;
                    size_t macroK = p.kernelY * p.kernelX * macroC;
                    for (size_t yBeg = 0; yBeg < p.dstH;)
                    {
                        size_t yEnd = Simd::Min(yBeg + a.macroH, p.dstH);
                        if (a.macroC == p.srcC)
                            ConvolutionNhwcDirect_2<TermSingle, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else if(sc == 0)
                            ConvolutionNhwcDirect_2<TermFirst, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else if (sc + macroC == p.srcC)
                            ConvolutionNhwcDirect_2<TermLast, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else 
                            ConvolutionNhwcDirect_2<TermIterim, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        yBeg = yEnd;
                    }
                    weight += AlignHiAny(macroD, a.microD)*macroK;
                }
                if (type == ::SimdConvolutionActivationPrelu)
                    params += macroD;
            }
        }

        //---------------------------------------------------------------------

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect1x1_2x6(const float * src0, const ConvParam32f & p,
            size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, d10, d11, d20, d21, d30, d31, d40, d41, d50, d51, s0, w0, w1;
            size_t dS = p.srcC, dD = p.dstC;
            const float * src1 = src0 + 1 * dS;
            const float * src2 = src0 + 2 * dS;
            const float * src3 = src0 + 3 * dS;
            const float * src4 = src0 + 4 * dS;
            const float * src5 = src0 + 5 * dS;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps(); d01 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps(); d11 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps(); d21 = _mm256_setzero_ps();
                d30 = _mm256_setzero_ps(); d31 = _mm256_setzero_ps();
                d40 = _mm256_setzero_ps(); d41 = _mm256_setzero_ps();
                d50 = _mm256_setzero_ps(); d51 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    w1 = _mm256_loadu_ps(weight + F);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    d01 = _mm256_fmadd_ps(s0, w1, d01);
                    s0 = _mm256_set1_ps(src1[offset]);
                    d10 = _mm256_fmadd_ps(s0, w0, d10);
                    d11 = _mm256_fmadd_ps(s0, w1, d11);
                    s0 = _mm256_set1_ps(src2[offset]);
                    d20 = _mm256_fmadd_ps(s0, w0, d20);
                    d21 = _mm256_fmadd_ps(s0, w1, d21);
                    s0 = _mm256_set1_ps(src3[offset]);
                    d30 = _mm256_fmadd_ps(s0, w0, d30);
                    d31 = _mm256_fmadd_ps(s0, w1, d31);
                    s0 = _mm256_set1_ps(src4[offset]);
                    d40 = _mm256_fmadd_ps(s0, w0, d40);
                    d41 = _mm256_fmadd_ps(s0, w1, d41);
                    s0 = _mm256_set1_ps(src5[offset]);
                    d50 = _mm256_fmadd_ps(s0, w0, d50);
                    d51 = _mm256_fmadd_ps(s0, w1, d51);
                    weight += DF;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d31, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d41, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d51, bias, params);
                }
                else
                {
                    dstC -= F;
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d31, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d41, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d51, bias, params, dstC);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps();
                d30 = _mm256_setzero_ps();
                d40 = _mm256_setzero_ps();
                d50 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    s0 = _mm256_set1_ps(src1[offset]);
                    d10 = _mm256_fmadd_ps(s0, w0, d10);
                    s0 = _mm256_set1_ps(src2[offset]);
                    d20 = _mm256_fmadd_ps(s0, w0, d20);
                    s0 = _mm256_set1_ps(src3[offset]);
                    d30 = _mm256_fmadd_ps(s0, w0, d30);
                    s0 = _mm256_set1_ps(src4[offset]);
                    d40 = _mm256_fmadd_ps(s0, w0, d40);
                    s0 = _mm256_set1_ps(src5[offset]);
                    d50 = _mm256_fmadd_ps(s0, w0, d50);
                    weight += DF;
                }
                if (dstC == F)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params);
                }
                else
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d30, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d40, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d50, bias, params, dstC);
                }
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect1x1_2x3(const float * src0, const ConvParam32f & p,
            size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, d10, d11, d20, d21, s0, w0, w1;
            size_t dS = p.srcC, dD = p.dstC;
            const float * src1 = src0 + 1 * dS;
            const float * src2 = src0 + 2 * dS;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps(); d01 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps(); d11 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps(); d21 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    w1 = _mm256_loadu_ps(weight + F);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    d01 = _mm256_fmadd_ps(s0, w1, d01);
                    s0 = _mm256_set1_ps(src1[offset]);
                    d10 = _mm256_fmadd_ps(s0, w0, d10);
                    d11 = _mm256_fmadd_ps(s0, w1, d11);
                    s0 = _mm256_set1_ps(src2[offset]);
                    d20 = _mm256_fmadd_ps(s0, w0, d20);
                    d21 = _mm256_fmadd_ps(s0, w1, d21);
                    weight += DF;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params);
                }
                else
                {
                    dstC -= F;
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d11, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d21, bias, params, dstC);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                d10 = _mm256_setzero_ps();
                d20 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    s0 = _mm256_set1_ps(src1[offset]);
                    d10 = _mm256_fmadd_ps(s0, w0, d10);
                    s0 = _mm256_set1_ps(src2[offset]);
                    d20 = _mm256_fmadd_ps(s0, w0, d20);
                    weight += DF;
                }
                if (dstC == F)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params);
                }
                else
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d10, bias, params, dstC);
                    dst += dD;
                    Term<term>::template Save<type, 0>(dst + 0, d20, bias, params, dstC);
                }
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect1x1_2x1(const float * src0, const ConvParam32f & p,
            size_t srcC, size_t dstC, const float * weight, const __m256 * bias, const __m256 * params, float * dst)
        {
            __m256 d00, d01, s0, w0, w1;
            if (dstC > F)
            {
                d00 = _mm256_setzero_ps(); d01 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    w1 = _mm256_loadu_ps(weight + F);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    d01 = _mm256_fmadd_ps(s0, w1, d01);
                    weight += DF;
                }
                if (dstC == DF)
                {
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params);
                }
                else
                {
                    dstC -= F;
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                    Term<term>::template Save<type, 1>(dst + F, d01, bias, params, dstC);
                }
            }
            else
            {
                d00 = _mm256_setzero_ps();
                for (size_t offset = 0; offset < srcC; ++offset)
                {
                    w0 = _mm256_loadu_ps(weight + 0);
                    s0 = _mm256_set1_ps(src0[offset]);
                    d00 = _mm256_fmadd_ps(s0, w0, d00);
                    weight += DF;
                }
                if (dstC == F)
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params);
                else
                    Term<term>::template Save<type, 0>(dst + 0, d00, bias, params, dstC);
            }
        }

        template<TermType term, SimdConvolutionActivationType type> void ConvolutionNhwcDirect1x1_2(const float * src, const ConvParam32f & p,
            size_t dstC, size_t yBeg, size_t yEnd, size_t srcC, const float * weight, const float * bias, const float * params, float * dst)
        {
            size_t n1 = (yEnd - yBeg)*p.dstW;
            size_t n3 = AlignLoAny(n1, 3);
            size_t n6 = AlignLoAny(n1, 6);

            __m256 _params[2], _bias[2];
            _params[0] = _mm256_set1_ps(params[0]);
            if (type == ::SimdConvolutionActivationRestrictRange)
                _params[1] = _mm256_set1_ps(params[1]);

            for (size_t dc = 0; dc < dstC; dc += DF)
            {
                size_t dC = Simd::Min(DF, dstC - dc);
                _bias[0] = _mm256_loadu_ps(bias + dc + 0);
                _bias[1] = _mm256_loadu_ps(bias + dc + F);
                if (type == ::SimdConvolutionActivationPrelu)
                {
                    _params[0] = _mm256_loadu_ps(params + dc + 0);
                    _params[1] = _mm256_loadu_ps(params + dc + F);
                }
                const float * ps = src + yBeg * p.srcW * p.srcC;
                float * pd = dst + dc + yBeg * p.dstW * p.dstC;
                size_t i = 0;
                for (; i < n6; i += 6, ps += 6 * p.srcC, pd += 6 * p.dstC)
                    ConvolutionNhwcDirect1x1_2x6<term, type>(ps, p, srcC, dC, weight, _bias, _params, pd);
                for (; i < n3; i += 3, ps += 3 * p.srcC, pd += 3 * p.dstC)
                    ConvolutionNhwcDirect1x1_2x3<term, type>(ps, p, srcC, dC, weight, _bias, _params, pd);
                for (; i < n1; i += 1, ps += 1 * p.srcC, pd += 1 * p.dstC)
                    ConvolutionNhwcDirect1x1_2x1<term, type>(ps, p, srcC, dC, weight, _bias, _params, pd);
                weight += srcC*DF;
            }
        }

        template<SimdConvolutionActivationType type> void ConvolutionNhwcDirect1x1_2(const float * src, const ConvParam32f & p,
            const SynetConvolution32fNhwcDirect::AlgParam & a, const float * weight, const float * bias, const float * params, float * dst)
        {
            for (size_t dc = 0; dc < p.dstC; dc += a.macroD)
            {
                size_t macroD = Simd::Min(p.dstC, dc + a.macroD) - dc;
                for (size_t sc = 0; sc < p.srcC; sc += a.macroC)
                {
                    size_t macroC = Simd::Min(p.srcC, sc + a.macroC) - sc;
                    for (size_t yBeg = 0; yBeg < p.dstH;)
                    {
                        size_t yEnd = Simd::Min(yBeg + a.macroH, p.dstH);
                        if (a.macroC == p.srcC)
                            ConvolutionNhwcDirect1x1_2<TermSingle, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else if (sc == 0)
                            ConvolutionNhwcDirect1x1_2<TermFirst, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else if (sc + macroC == p.srcC)
                            ConvolutionNhwcDirect1x1_2<TermLast, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        else
                            ConvolutionNhwcDirect1x1_2<TermIterim, type>(src + sc, p, macroD, yBeg, yEnd, macroC, weight, bias + dc, params, dst + dc);
                        yBeg = yEnd;
                    }
                    weight += AlignHiAny(macroD, a.microD)*macroC;
                }
                if (type == ::SimdConvolutionActivationPrelu)
                    params += macroD;
            }
        }

        //---------------------------------------------------------------------

        template <SimdConvolutionActivationType type> void Set(const ConvParam32f & p, size_t microD, SynetConvolution32fNhwcDirect::ConvolutionPtr & convolution)
        {
            if (p.Is1x1())
            {
                switch (microD)
                {
                case 2 * F: convolution = ConvolutionNhwcDirect1x1_2<type>; break;
                default:
                    assert(0);
                }
            }
            else
            {
                switch (microD)
                {
                case 2 * F: convolution = ConvolutionNhwcDirect_2<type>; break;
                default:
                    assert(0);
                }
            }
        }

        SynetConvolution32fNhwcDirect::SynetConvolution32fNhwcDirect(const ConvParam32f & p)
            : Avx::SynetConvolution32fNhwcDirect(p)
        {
            size_t microD = 2 * F;
            switch (p.activation)
            {
            case SimdConvolutionActivationIdentity: Set<SimdConvolutionActivationIdentity>(p, microD, _convolution); break;
            case SimdConvolutionActivationRelu: Set<SimdConvolutionActivationRelu>(p, microD, _convolution); break;
            case SimdConvolutionActivationLeakyRelu: Set<SimdConvolutionActivationLeakyRelu>(p, microD, _convolution); break;
            case SimdConvolutionActivationRestrictRange: Set<SimdConvolutionActivationRestrictRange>(p, microD, _convolution); break;
            case SimdConvolutionActivationPrelu: Set<SimdConvolutionActivationPrelu>(p, microD, _convolution); break;
            case SimdConvolutionActivationElu: Set<SimdConvolutionActivationElu>(p, microD, _convolution); break;
            default: assert(0);
            }
            SetAlgParam(microD, 32 * 1024, 256 * 1024, 2 * 1024 * 1024);
        }
    }
#endif//SIMD_AVX2_ENABLE
}
