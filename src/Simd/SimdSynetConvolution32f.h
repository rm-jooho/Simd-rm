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
#ifndef __SimdSynetConvolution32f_h__
#define __SimdSynetConvolution32f_h__

#include "Simd/SimdArray.h"
#include "Simd/SimdPerformance.h"
#include "Simd/SimdRuntime.h"
#include "Simd/SimdGemm.h"

#ifdef _N
#undef _N
#endif

namespace Simd
{
    const bool NHWC_GEMM_COMPATIBLE = false;
    const bool NHWC_GEMM_RUNTIME = true;

    struct ConvParam32f : public SimdConvolutionParameters
    {
        SimdBool trans;
        size_t batch;
        SimdGemm32fNNPtr gemm;

        ConvParam32f(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm)
        {
            *((SimdConvolutionParameters*)this) = *conv;
            this->trans = (srcF == SimdTensorFormatNhwc ? SimdTrue : SimdFalse);
            this->batch = batch;
            this->gemm = gemm;
        }

        bool Valid()
        {
            return 
                dstH == (srcH + padY + padH - (dilationY * (kernelY - 1) + 1)) / strideY + 1 && dstH > 0 &&
                dstW == (srcW + padX + padW - (dilationX * (kernelX - 1) + 1)) / strideX + 1 && dstW > 0 &&
                srcT == SimdTensorData32f && dstT == SimdTensorData32f && srcF == dstF && (srcF == SimdTensorFormatNchw || srcF == SimdTensorFormatNhwc);
        }

        SIMD_INLINE bool IsKernel(size_t value) const
        {
            return kernelY == value && kernelX == value;
        }

        SIMD_INLINE bool IsDilation(size_t value) const
        {
            return dilationY == value && dilationX == value;
        }

        SIMD_INLINE bool IsStride(size_t value) const
        {
            return strideY == value && strideX == value;
        }

        SIMD_INLINE bool IsPad(size_t value) const
        {
            return padY == value && padX == value && padH == value && padW == value;
        }

        SIMD_INLINE bool IsDepthwise() const
        {
            return srcC == group && dstC == group;
        }
        SIMD_INLINE bool Is1x1() const
        {
            return IsKernel(1) && IsDilation(1) && IsStride(1) && IsPad(0);
        }

#ifdef SIMD_PERFORMANCE_STATISTIC
        String Info() const
        {
            std::stringstream ss;
            ss << batch << "x" << srcC << "x" << srcH << "x" << srcW;
            ss << "-" << dstC << "x" << kernelY << "x" << kernelX;
            ss << "-" << strideX << "-" << Simd::Max(padX, padW) << "-" << group << "-" << trans;
            return ss.str();
        }

        long long Flop() const
        {
            return batch * kernelY * kernelX * srcC * dstH * dstW * dstC / group * 2;
        }
#endif
    };

    class SynetConvolution32f : public Deletable
    {
    public:
        SynetConvolution32f(const ConvParam32f & p) 
            : _param(p)
            , _0(0.0f)
            , _1(1.0f)
            , _nhwcRun(0)
            , _nhwcReorderB(0)
            , _biasAndActivation(0)
#if defined(SIMD_PERFORMANCE_STATISTIC)
            , _perf(NULL)
#endif
        {
        }

        const ConvParam32f & Param() const 
        {
            return _param;
        }

        virtual String Ext() const = 0;
        virtual String Desc() const = 0;

        virtual size_t ExternalBufferSize() const
        {
            return 1;
        }

        virtual size_t InternalBufferSize() const
        {
            return _buffer.size + _nhwcWeight.size;
        }

        virtual void SetParams(const float * weight, SimdBool * internal, const float * bias, const float * params)
        {
            _weight = weight;
            if (internal)
                *internal = SimdFalse;
            _bias = bias;
            _params = params;
        }

        virtual void Forward(const float * src, float * buf, float * dst) = 0;

        float * Buffer(float * buffer)
        {
            if (buffer)
                return buffer;
            else
            {
                _buffer.Resize(ExternalBufferSize());
                return _buffer.data;
            }
        }

#if defined(SIMD_PERFORMANCE_STATISTIC)
        Base::PerformanceMeasurer * Perf(const String & func)
        {
            if (_perf == NULL)
                _perf = Simd::Base::PerformanceMeasurerStorage::s_storage.Get(func, Param().Info() + " " + Desc(), Param().Flop());
            return _perf;
        }
#endif

    protected:
        typedef void(*NhwcReorderB)(size_t M, size_t N, size_t K, const float * B, float * pB, GemmKernelType type, bool compatibility);
        typedef void(*NhwcRun)(size_t M, size_t N, size_t K, const float * A, const float * B, float * C, GemmKernelType type, bool compatibility);
        typedef void(*BiasAndActivation)(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType activation, const float * params, SimdBool trans, float * dst);

        ConvParam32f _param;
        Array32f _buffer;
        float _0, _1;
        const float * _weight, * _bias, * _params;
        RuntimeGemm _gemm;
        RuntimeGemmCb _gemmCb;
        Array32f _nhwcWeight;
        NhwcRun _nhwcRun;
        NhwcReorderB _nhwcReorderB;
        BiasAndActivation _biasAndActivation;
#if defined(SIMD_PERFORMANCE_STATISTIC)
        Base::PerformanceMeasurer * _perf;
#endif
    };

    namespace Base
    {
        void ConvolutionBiasAndActivation(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType activation, const float * params, SimdBool trans, float * dst);

        class SynetConvolution32fGemmNN : public SynetConvolution32f
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::GemmNN" + (_merge > 1 ? "-" + ToStr(_merge) : ""); }
            virtual size_t ExternalBufferSize() const;
            virtual void SetParams(const float * weight, SimdBool * internal, const float * bias, const float * params);
            virtual void Forward(const float * src, float * buf, float * dst);

        protected:
            virtual void ImgToCol(const float * src, float * dst);
            virtual void ImgToRow(const float * src, float * dst);

            bool _is1x1;
            size_t _M, _N, _K, _ldW, _ldS, _ldD, _grW, _grS, _grD, _batch, _sizeS, _sizeB, _sizeD, _merge;
        };

        class SynetConvolution32fGemmNT : public SynetConvolution32f
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::GemmNT"; }
            virtual size_t ExternalBufferSize() const;
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

        protected:
            virtual void GemmAndBias(const float * src, float * dst);

            static void ImgToRow(const float * src, const ConvParam32f & p, float * dst);

            bool _is1x1;
            size_t _weightStep, _srcStep, _dstStep, _M, _N, _K, _batch, _sizeS, _sizeB, _sizeD;
        };

        class SynetConvolution32fWinograd : public SynetConvolution32f
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::Winograd" + ToStr(_block) + "x3" + (_merge > 1 ? "-" + ToStr(_merge) : ""); }
            virtual size_t ExternalBufferSize() const;
            virtual size_t InternalBufferSize() const;
            virtual void SetParams(const float * weight, SimdBool * internal, const float * bias, const float * params);
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

        protected:
            typedef void(*SetFilter)(const float * src, size_t size, float * dst, SimdBool trans);
            typedef void(*SetInput)(const float * src, size_t srcChannels, size_t srcHeight, size_t srcWidth, float * dst, size_t dstStride, SimdBool pad, SimdBool trans);
            typedef void(*SetOutput)(const float * src, size_t srcStride, float * dst, size_t dstChannels, size_t dstHeight, size_t dstWidth, SimdBool trans);

            void SetBlock(size_t block);
            void ForwardMerged(const float * src, float * bufS, float * bufD, float * dst, size_t merge);

            size_t _count, _block, _tileH, _tileW, _strideW, _strideS, _strideD, _M, _N, _K, _batch, _sizeS, _sizeD, _nhwcStrideW, _merge;
            SimdBool _pad;
            Array32f _winogradWeight;
            SetFilter _setFilter;
            SetInput _setInput;
            SetOutput _setOutput;
#if 0
            struct WinArgs
            {
                const float * src; float * bufS; float * bufD; float * dst;
                SIMD_INLINE WinArgs(const float * src_, float * bufS_, float * bufD_, float * dst_)
                    :src(src_), bufS(bufS_), bufD(bufD_), dst(dst_)
                {}
            };  
            
            struct WinFunc
            {
                SIMD_INLINE WinFunc(const SynetConvolution32fWinograd * win, const String & name)
                    : _win(win)
                    , _name(name)
                {
                }

                SIMD_INLINE String Name() const { return _name; }

                SIMD_INLINE void Run(const WinArgs & args)
                {
                }

            private:
                const SynetConvolution32fWinograd * _win;
                String _name;
            };
#endif
        };

        class SynetConvolution32fDirectNchw : public SynetConvolution32f
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::DirectNchw"; }
            virtual size_t ExternalBufferSize() const;
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

            typedef void(*ConvolutionBiasActivationPtr)(const float * src, size_t srcC, size_t srcH, size_t srcW, const float * weight, const float * bias, const float * params, float * dst, size_t dstC, size_t dstH, size_t dstW);
        protected:
            void Pad(const float * src, float * dst) const;
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();

            size_t _grW, _grS, _grD, _srcC, _srcH, _srcW, _dstC;
            int _pad;
            ConvolutionBiasActivationPtr _convolutionBiasActivation;
        };

        class SynetConvolution32fDirectNhwc : public SynetConvolution32f
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::DirectNhwc"; }
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

            typedef void(*ConvolutionBiasActivationPtr)(const float * src, const ConvParam32f & p, const float * weight, const float * bias, const float * params, float * dst);
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation(); 

            size_t _batch, _sizeS, _sizeD;
            ConvolutionBiasActivationPtr _convolutionBiasActivation;
        };

        class SynetConvolution32fDepthwiseDotProduct : public SynetConvolution32f
        {
        public:
            SynetConvolution32fDepthwiseDotProduct(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::DepthwiseDotProduct"; }
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

        protected:
            size_t _count, _size, _batch, _sizeS, _sizeD;
        }; 

        class SynetConvolution32fNhwcDirect : public SynetConvolution32f
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Base"; }
            virtual String Desc() const { return Ext() + "::NhwcDirect"; }
            virtual size_t InternalBufferSize() const;
            virtual void SetParams(const float * weight, SimdBool * internal, const float * bias, const float * params);
            virtual void Forward(const float * src, float * buf, float * dst);

            static bool Preferable(const ConvParam32f & p);

            struct AlgParam
            {
                size_t microD, macroH, macroC, macroD;
            };
            typedef void(*ConvolutionPtr)(const float * src, const ConvParam32f & p, const AlgParam & a, const float * weight, const float * bias, const float * params, float * dst);

        protected:
            void SetAlgParam(size_t microD, size_t L1, size_t L2, size_t L3);
            void ReorderWeight(const float * src, float * dst);

            size_t _sizeS, _sizeD;
            AlgParam _alg;
            Array32f _rWeight, _rBias, _rParams;
            ConvolutionPtr _convolution;
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }

#ifdef SIMD_SSE2_ENABLE    
    namespace Sse2
    {
        void ConvolutionBiasAndActivation(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType activation, const float * params, ::SimdBool trans, float * dst);

        class SynetConvolution32fGemmNN : public Base::SynetConvolution32fGemmNN
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }
        };

        class SynetConvolution32fWinograd : public Base::SynetConvolution32fWinograd
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }
        };

        class SynetConvolution32fDirectNchw : public Base::SynetConvolution32fDirectNchw
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }

            static bool Preferable(const ConvParam32f & p);

        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDirectNhwc : public Base::SynetConvolution32fDirectNhwc
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }

            static bool Preferable(const ConvParam32f & p);
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDepthwiseDotProduct : public Base::SynetConvolution32fDepthwiseDotProduct
        {
        public:
            SynetConvolution32fDepthwiseDotProduct(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }
            virtual void Forward(const float * src, float * buf, float * dst);
        };

        class SynetConvolution32fNhwcDirect : public Base::SynetConvolution32fNhwcDirect
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Sse2"; }

            static bool Preferable(const ConvParam32f & p);
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_SSE2_ENABLE

#ifdef SIMD_SSE3_ENABLE    
    namespace Sse3
    {
        class SynetConvolution32fGemmNT : public Base::SynetConvolution32fGemmNT
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Sse3"; }

            static bool Preferable(const ConvParam32f & p);
        protected:
            virtual void GemmAndBias(const float * src, float * dst);
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_SSE3_ENABLE

#ifdef SIMD_AVX_ENABLE    
    namespace Avx
    {
        void ConvolutionBiasAndActivation(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType type, const float * params, ::SimdBool trans, float * dst);

        class SynetConvolution32fGemmNN : public Sse2::SynetConvolution32fGemmNN
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        protected:
            virtual void ImgToRow(const float * src, float * dst);
        };

        class SynetConvolution32fGemmNT : public Sse3::SynetConvolution32fGemmNT
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        protected:
            virtual void GemmAndBias(const float * src, float * dst);
        };

        class SynetConvolution32fWinograd : public Sse2::SynetConvolution32fWinograd
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        };

        class SynetConvolution32fDirectNchw : public Sse2::SynetConvolution32fDirectNchw
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDirectNhwc : public Sse2::SynetConvolution32fDirectNhwc
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        
            static bool Preferable(const ConvParam32f & p);
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDepthwiseDotProduct : public Sse2::SynetConvolution32fDepthwiseDotProduct
        {
        public:
            SynetConvolution32fDepthwiseDotProduct(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
            virtual void Forward(const float * src, float * buf, float * dst);
        };

        class SynetConvolution32fNhwcDirect : public Sse2::SynetConvolution32fNhwcDirect
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Avx"; }
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_AVX_ENABLE

#ifdef SIMD_AVX2_ENABLE    
    namespace Avx2
    {
        void NhwcRun(size_t M, size_t N, size_t K, const float * A, const float * B, float * C);
        void NhwcReorderB(size_t M, size_t N, size_t K, const float * B, float * pB);
        size_t NhwcBufferSize(size_t M, size_t N, size_t K);

        class SynetConvolution32fGemmNN : public Avx::SynetConvolution32fGemmNN
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        protected:
            virtual void ImgToCol(const float * src, float * dst);
        private:
            Array32i _index, _nose, _tail, _start;
        };

        class SynetConvolution32fGemmNT : public Avx::SynetConvolution32fGemmNT
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        protected:
            virtual void GemmAndBias(const float * src, float * dst);
        };

        class SynetConvolution32fWinograd : public Avx::SynetConvolution32fWinograd
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        };

        class SynetConvolution32fDirectNchw : public Avx::SynetConvolution32fDirectNchw
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDirectNhwc : public Avx::SynetConvolution32fDirectNhwc
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fNhwcDirect : public Avx::SynetConvolution32fNhwcDirect
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Avx2"; }
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_AVX2_ENABLE

#ifdef SIMD_AVX512F_ENABLE    
    namespace Avx512f
    {
        void ConvolutionBiasAndActivation(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType activation, const float * params, ::SimdBool trans, float * dst);

        class SynetConvolution32fGemmNN : public Avx2::SynetConvolution32fGemmNN
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }
        protected:
            virtual void ImgToCol(const float * src, float * dst);
        private:
            Array32i _index;
            Array16u _nose, _tail;
        };

        class SynetConvolution32fGemmNT : public Avx2::SynetConvolution32fGemmNT
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }
        protected:
            virtual void GemmAndBias(const float * src, float * dst);
        };

        class SynetConvolution32fWinograd : public Avx2::SynetConvolution32fWinograd
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }
        };

        class SynetConvolution32fDirectNchw : public Avx2::SynetConvolution32fDirectNchw
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }

            static bool Preferable(const ConvParam32f & p);

        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDirectNhwc : public Avx2::SynetConvolution32fDirectNhwc
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fNhwcDirect : public Avx2::SynetConvolution32fNhwcDirect
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Avx512f"; }
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_AVX512F_ENABLE

#ifdef SIMD_NEON_ENABLE    
    namespace Neon
    {
        void ConvolutionBiasAndActivation(const float * bias, size_t count, size_t size, ::SimdConvolutionActivationType activation, const float * params, ::SimdBool trans, float * dst);

        class SynetConvolution32fGemmNN : public Base::SynetConvolution32fGemmNN
        {
        public:
            SynetConvolution32fGemmNN(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }
        };

        class SynetConvolution32fGemmNT : public Base::SynetConvolution32fGemmNT
        {
        public:
            SynetConvolution32fGemmNT(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }

            static bool Preferable(const ConvParam32f & p);
        protected:
            virtual void GemmAndBias(const float * src, float * dst);
        };

        class SynetConvolution32fWinograd : public Base::SynetConvolution32fWinograd
        {
        public:
            SynetConvolution32fWinograd(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }

            static bool Preferable(const ConvParam32f & p);
        };

        class SynetConvolution32fDirectNchw : public Base::SynetConvolution32fDirectNchw
        {
        public:
            SynetConvolution32fDirectNchw(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }

            static bool Preferable(const ConvParam32f & p);

        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDirectNhwc : public Base::SynetConvolution32fDirectNhwc
        {
        public:
            SynetConvolution32fDirectNhwc(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }

            static bool Preferable(const ConvParam32f & p);
        protected:
            virtual ConvolutionBiasActivationPtr SetConvolutionBiasActivation();
        };

        class SynetConvolution32fDepthwiseDotProduct : public Base::SynetConvolution32fDepthwiseDotProduct
        {
        public:
            SynetConvolution32fDepthwiseDotProduct(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }
            virtual void Forward(const float * src, float * buf, float * dst);
        };

        class SynetConvolution32fNhwcDirect : public Base::SynetConvolution32fNhwcDirect
        {
        public:
            SynetConvolution32fNhwcDirect(const ConvParam32f & p);
            virtual String Ext() const { return "Neon"; }

            static bool Preferable(const ConvParam32f & p);
        };

        void * SynetConvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm);
    }
#endif//SIMD_NEON_ENABLE
}

#endif//__SimdSynetConvolution32f_h__
