#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DawnAtmosphereConfig.generated.h"

/**
 * Dawn/Early Morning atmosphere configuration for the Cretaceous world.
 * Defines lighting parameters for the golden-hour sunrise palette.
 * Agent #08 — Lighting & Atmosphere Agent — Cycle AUTO_20260701_011
 */

USTRUCT(BlueprintType)
struct FLight_DawnPalette
{
    GENERATED_BODY()

    // Sun direction — low angle just above horizon (east)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunPitchDegrees = -8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunYawDegrees = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunIntensityLux = 4.5f;

    // Peachy-pink dawn sun color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.784f, 0.627f, 1.0f);

    // SkyLight — cool-blue ambient for pre-dawn sky
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sky")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sky")
    FLinearColor SkyLightColor = FLinearColor(0.706f, 0.784f, 0.902f, 1.0f);

    // Fog — soft pink-lavender dawn mist
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float FogHeightFalloff = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.72f, 0.55f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float VolumetricFogExtinctionScale = 1.2f;
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (05:00-07:00)"),
    Morning     UMETA(DisplayName = "Morning (07:00-10:00)"),
    Midday      UMETA(DisplayName = "Midday (10:00-14:00)"),
    Afternoon   UMETA(DisplayName = "Afternoon (14:00-17:00)"),
    Dusk        UMETA(DisplayName = "Dusk (17:00-19:30)"),
    Night       UMETA(DisplayName = "Night (19:30-05:00)")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDawnAtmosphereConfig : public UObject
{
    GENERATED_BODY()

public:
    UDawnAtmosphereConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_DawnPalette DawnPalette;

    // Returns true if current time is within dawn window
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    bool IsDawnActive() const;

    // Returns normalized time-of-day blend factor (0=night, 1=peak)
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    float GetDawnBlendFactor(float GameHour) const;

    // Applies dawn palette values to the world lighting actors
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmosphere")
    void ApplyDawnPaletteToWorld();
};
