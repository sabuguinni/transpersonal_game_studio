#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "GoldenHour"),
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
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity;

    FLight_SkyPalette()
        : SunColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f))
        , SunIntensity(10.0f)
        , SunPitch(-45.0f)
        , FogColor(FLinearColor(0.6f, 0.7f, 0.9f, 1.0f))
        , FogDensity(0.02f)
        , SkyLightColor(FLinearColor(0.8f, 0.85f, 1.0f, 1.0f))
        , SkyLightIntensity(2.0f)
    {}
};

/**
 * ADayNightCycleManager
 * Manages the full 24-hour day/night cycle for the prehistoric survival world.
 * Controls sun position, sky palette, fog, and atmospheric conditions.
 * Drives emotional tone through lighting — Roger Deakins philosophy applied to game.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // === REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor;

    // === TIME SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float CurrentTimeOfDayHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayDurationSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bPauseDayNightCycle;

    // === CURRENT STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State")
    ELight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State")
    FLight_SkyPalette CurrentPalette;

    // === PALETTE PRESETS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette GoldenHourPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette NightPalette;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPalette(const FLight_SkyPalette& Palette);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_SkyPalette InterpolatePalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActorsInWorld();

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetNormalizedTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsNightTime() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsDaytime() const;

private:
    void InitializeDefaultPalettes();
    void TickDayNightCycle(float DeltaTime);
    FLight_SkyPalette GetPaletteForTime(float Hours) const;
};
