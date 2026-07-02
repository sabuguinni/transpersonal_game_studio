#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// Day/Night Cycle Lighting Palettes — Agent #08
// Prehistoric survival game — realistic Cretaceous atmosphere
// ============================================================

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
struct TRANSPERSONALGAME_API FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitch = -75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYaw = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.97f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.012f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.85f, 0.78f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float VolumetricFogExtinction = 0.8f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    float TimeOfDayNormalized = 0.5f;

    // --- Cycle speed: 1.0 = real-time, 60.0 = 1 minute per game day ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    float CycleSpeedMultiplier = 60.0f;

    // --- Whether the cycle is running ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    bool bCycleActive = true;

    // --- Lighting palette presets ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    TArray<FLight_SkyPalette> Palettes;

    // --- References to scene lights ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ASkyLight* SkyLightActor = nullptr;

    // --- Current resolved time of day enum ---
    UPROPERTY(BlueprintReadOnly, Category = "DayNight|State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void ApplyPalette(const FLight_SkyPalette& Palette);

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    FLight_SkyPalette InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;

    UFUNCTION(BlueprintCallable, Category = "DayNight")
    void AutoFindSceneLights();

    UFUNCTION(CallInEditor, Category = "DayNight")
    void SnapToCurrentTime();

private:
    void InitDefaultPalettes();
    void UpdateLighting(float DeltaTime);
    float GetSunPitchForTime(float NormalizedTime) const;
    float GetSunYawForTime(float NormalizedTime) const;
    FLinearColor GetSkyColorForTime(float NormalizedTime) const;
};
