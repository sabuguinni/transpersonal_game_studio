#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LightingAtmosphereConfig.generated.h"

/**
 * Cretaceous lighting preset configuration.
 * All values validated against MinPlayableMap golden-hour stack.
 * Agent #08 — Lighting & Atmosphere
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Midday      UMETA(DisplayName = "Midday"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float PitchDegrees = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float YawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 12.0f;

    /** Warm golden-hour tint: R=255 G=220 B=160 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FColor LightColor = FColor(255, 220, 160, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float ShadowAmount = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float DynamicShadowDistance = 50000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float HeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor InscatteringColor = FLinearColor(0.45f, 0.62f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float MaxOpacity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float StartDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_PostProcessConfig
{
    GENERATED_BODY()

    /** Manual exposure bias — prevents auto-exposure darkening */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float ExposureBias = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float BloomIntensity = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float BloomThreshold = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float AmbientOcclusionIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float AmbientOcclusionRadius = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float VignetteIntensity = 0.35f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingAtmosphereConfig : public UObject
{
    GENERATED_BODY()

public:
    ULightingAtmosphereConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_SunConfig SunConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_FogConfig FogConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_PostProcessConfig PostProcessConfig;

    /** SkyLight real-time capture intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    /** Rayleigh scattering — denser Cretaceous atmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float RayleighScatteringScale = 0.0331f;

    /** Mie scattering — haze and humidity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float MieScatteringScale = 0.003f;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_SunConfig GetSunConfigForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    static FString GetTimeOfDayName(ELight_TimeOfDay TimeOfDay);
};
