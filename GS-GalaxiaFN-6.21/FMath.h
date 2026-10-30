#pragma once

#undef  PI
#define PI 					(3.1415926535897932f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)

// Copied from float.h
#define MAX_FLT 3.402823466e+38F

// Aux constants.
#define INV_PI			(0.31830988618f)
#define HALF_PI			(1.57079632679f)

// Magic numbers for numerical precision.
#define DELTA			(0.00001f)


class FMath
{
private:
    static int GSRandSeed;

public:
    static __forceinline float Sin(float Value) { return sinf(Value); }
    static __forceinline float Asin(float Value) { return asinf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
    static __forceinline float Sinh(float Value) { return sinhf(Value); }
    static __forceinline float Cos(float Value) { return cosf(Value); }
    static __forceinline float Acos(float Value) { return acosf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
    static __forceinline float Tan(float Value) { return tanf(Value); }
    static __forceinline float Atan(float Value) { return atanf(Value); }
    static __forceinline float Sqrt(float Value) { return sqrtf(Value); }
    static __forceinline float Pow(float A, float B) { return powf(A, B); }

    template< class T = int32 >
    static FORCEINLINE T Min(const T A, const T B) { return (A <= B) ? A : B; }

    template< class T = int32 >
    static FORCEINLINE T Max(const T A, const T B) { return (A >= B) ? A : B; }

    template< class T >
    static __forceinline T Clamp(const T X, const T Min, const T Max)
    {
        return X < Min ? Min : X < Max ? X : Max;
    }

    template< class T, class U >
    static __forceinline T Lerp(const T& A, const T& B, const U& Alpha)
    {
        return (T)(A + Alpha * (B - A));
    }

    template< class T >
    static __forceinline T Square(const T A)
    {
        return A * A;
    }

    static __forceinline int TruncToInt(float F)
    {
        return (int)F;
    }

    static __forceinline float TruncToFloat(float F)
    {
        return (float)TruncToInt(F);
    }

    static __forceinline float Fractional(float Value)
    {
        return Value - TruncToFloat(Value);
    }

    static __forceinline float DistSquared(const FVector& V1, const FVector& V2)
    {
        return Square(V2.X - V1.X) + Square(V2.Y - V1.Y) + Square(V2.Z - V1.Z);
    }

    template<class T>
    static __forceinline auto DegreesToRadians(T const& DegVal) -> decltype(DegVal* (PI / 180.f))
    {
        return DegVal * (PI / 180.f);
    }

    static int32 RandRange(int32 Min, int32 Max)
    {
        // Utilise SRand pour obtenir un float entre 0 et 1
        float NormalizedRandom = FMath::SRand();

        // Convertit le float en un entier dans la plage souhaitée
        int32 RandomValue = Min + static_cast<int32>(NormalizedRandom * (Max - Min + 1));

        return FMath::Clamp(RandomValue, Min, Max);
    }
    
    inline static float SRand()
    {
        // [OFFSET-TODO-6.21] FMath::SRand (8.51 = 0x1be57f0) : remplace par UKismetMathLibrary::RandomFloat (sans adresse)
        // float (*SRand)() = decltype(SRand)(0x1be57f0 + uintptr_t(GetModuleHandle(0)));
        // return SRand();
        return UKismetMathLibrary::RandomFloat();
    }

    static FORCEINLINE float InvSqrt(float F)
    {
        const __m128 fOneHalf = _mm_set_ss(0.5f);
        __m128 Y0, X0, X1, X2, FOver2;
        float temp;

        Y0 = _mm_set_ss(F);
        X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
        FOver2 = _mm_mul_ss(Y0, fOneHalf);

        // 1st Newton-Raphson iteration
        X1 = _mm_mul_ss(X0, X0);
        X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
        X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

        // 2nd Newton-Raphson iteration
        X2 = _mm_mul_ss(X1, X1);
        X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
        X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

        _mm_store_ss(&temp, X2);
        return temp;
    }
};