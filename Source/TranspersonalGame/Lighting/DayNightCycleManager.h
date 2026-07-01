#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
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
struct FLight_TimeOfDayPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitch = -75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.018f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.85f, 0.90f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float BloomScale = 0.35f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day as normalized 0-1 (0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    float CurrentTimeNormalized = 0.5f;

    // Speed multiplier for day/night cycle (1.0 = real time, 60.0 = 1 min per real second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    float CycleSpeedMultiplier = 60.0f;

    // Whether the cycle advances automatically
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Time")
    bool bCycleActive = true;

    // Current time of day enum (derived from CurrentTimeNormalized)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DayNight|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // Predefined palettes for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Palettes")
    TArray<FLight_TimeOfDayPalette> TimeOfDayPalettes;

    // Reference to the scene directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ADirectionalLight* SunLight = nullptr;

    // Reference to the scene exponential height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    AExponentialHeightFog* SceneFog = nullptr;

    // Reference to the scene sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|References")
    ASkyLight* SceneSkyLight = nullptr;

    // Apply a specific time of day palette immediately
    UFUNCTION(BlueprintCallable, Category = "DayNight|Control")
    void ApplyTimeOfDayPalette(ELight_TimeOfDay TimeOfDay);

    // Set time directly (0-1 normalized)
    UFUNCTION(BlueprintCallable, Category = "DayNight|Control")
    void SetTimeNormalized(float NewTime);

    // Get current time as hours (0-24)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DayNight|Query")
    float GetCurrentHour() const;

    // Get current time of day enum
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DayNight|Query")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    // Force auto-find scene lighting actors
    UFUNCTION(BlueprintCallable, Category = "DayNight|Setup")
    void AutoFindLightingActors();

    // Initialize default palettes for all times of day
    UFUNCTION(BlueprintCallable, Category = "DayNight|Setup")
    void InitializeDefaultPalettes();

private:
    void AdvanceCycle(float DeltaTime);
    void UpdateLightingFromTime(float TimeNormalized);
    FLight_TimeOfDayPalette InterpolatePalettes(const FLight_TimeOfDayPalette& A, const FLight_TimeOfDayPalette& B, float Alpha) const;
    ELight_TimeOfDay TimeNormalizedToEnum(float TimeNormalized) const;
    void ApplyPaletteToScene(const FLight_TimeOfDayPalette& Palette);

    float AccumulatedTime = 0.0f;
    static const float SecondsPerDay;
};
