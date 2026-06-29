#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "NightCycleManager.generated.h"

/**
 * ELight_TimeOfDay — Enumeration of day/night cycle phases
 * Prefix: Light_ to avoid conflicts with other agents
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

/**
 * FLight_SkyPalette — Color palette for a specific time of day
 */
USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.0f;
};

/**
 * ANightCycleManager — Manages the full day/night lighting cycle
 * for the Cretaceous prehistoric survival game.
 * 
 * Handles: sun position, sky color, fog density, moonlight,
 * bioluminescent accents, and Lumen GI transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ANightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Time of Day Control ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 600.0f;  // 10 minutes per full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bEnableRealTimeCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 12.0f;  // 0-24 hour representation

    // --- Light References ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunMoonLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // --- Palettes ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette GoldenHourPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette NightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MidnightPalette;

    // --- Night-specific settings ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Night")
    float MoonlightIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Night")
    FLinearColor MoonlightColor = FLinearColor(0.55f, 0.65f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Night")
    float BioluminescentIntensity = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Night")
    FLinearColor BioluminescentColor = FLinearColor(0.1f, 0.9f, 0.3f, 1.0f);

    // --- Functions ---

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetHour(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Night")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Night")
    float GetNightDangerMultiplier() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void ApplyCurrentPalette();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void ApplyNightPaletteImmediate();

private:
    float ElapsedDayTime = 0.0f;

    void InitializeDefaultPalettes();
    void ApplyPalette(const FLight_SkyPalette& Palette);
    void UpdateCycleFromHour(float Hour);
    ELight_TimeOfDay HourToTimeOfDay(float Hour) const;
    float TimeOfDayToSunPitch(float Hour) const;
};
