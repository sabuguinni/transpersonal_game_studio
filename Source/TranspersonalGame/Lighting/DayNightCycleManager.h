#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity;

    FLight_SkyPalette()
        : SunColor(FLinearColor(1.0f, 0.95f, 0.85f, 1.0f))
        , SunIntensity(10.0f)
        , SunPitch(-45.0f)
        , SunYaw(0.0f)
        , FogColor(FLinearColor(0.6f, 0.7f, 0.9f, 1.0f))
        , FogDensity(0.02f)
        , SkyLightColor(FLinearColor(0.8f, 0.85f, 1.0f, 1.0f))
        , SkyLightIntensity(1.5f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day in hours (0.0 - 24.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay;

    // Speed multiplier for day/night cycle (1.0 = real time, 60.0 = 1 min per game hour)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale;

    // Whether the cycle is running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Reference to the height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Current time of day enum
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentPhase;

    // Palettes for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Palettes")
    FLight_SkyPalette NightPalette;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewHour);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyPalette(const FLight_SkyPalette& Palette);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_SkyPalette GetPaletteForHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    static FLight_SkyPalette LerpPalette(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha);

private:
    void UpdateSunPosition(float Hour);
    void UpdateLighting(float Hour);
    ELight_TimeOfDay HourToPhase(float Hour) const;
    void InitDefaultPalettes();
};
