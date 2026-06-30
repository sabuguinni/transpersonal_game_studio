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
    PreDawn     UMETA(DisplayName = "Pre-Dawn"),
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinction = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeOfDayNormalized = 0.35f;

    // How fast time passes (1.0 = real time, 60.0 = 1 min per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale = 60.0f;

    // Whether the cycle is running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Full day duration in real seconds (default: 20 minutes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 1200.0f;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Reference to the height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Palettes for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette PreDawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette GoldenHourPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Palettes")
    FLight_SkyPalette NightPalette;

    // Get the current time of day enum
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    // Get current palette interpolated between key frames
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_SkyPalette GetCurrentPalette() const;

    // Force a specific time of day
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    // Apply current palette to scene lights
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyPaletteToScene(const FLight_SkyPalette& Palette);

    // Get sun pitch angle for current time
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetSunPitchForTime(float NormalizedTime) const;

    // Get hours in 24h format (0-24)
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetHours() const;

    // Get formatted time string "HH:MM"
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FString GetTimeString() const;

private:
    void InitializeDefaultPalettes();
    void AutoFindSceneLights();
    FLight_SkyPalette LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;

    float ElapsedSeconds = 0.0f;
};
