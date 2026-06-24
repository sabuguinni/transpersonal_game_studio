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
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.4f, 0.55f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 1.0f;
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

    // Speed multiplier for day/night cycle (1.0 = real time, 100.0 = 100x faster)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CycleSpeedMultiplier = 60.0f;

    // Whether the cycle is actively running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Current time of day enum
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    // Preset settings for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings NightSettings;

    // References to scene lights (set in editor or auto-found)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ASkyLight> SceneSkyLight;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimePreset(ELight_TimeOfDay Preset);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void FindAndCacheLightActors();

private:
    void UpdateLighting(float DeltaTime);
    void ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings InterpolateSettings(
        const FLight_TimeOfDaySettings& A,
        const FLight_TimeOfDaySettings& B,
        float Alpha) const;
    ELight_TimeOfDay CalculateTimeOfDayEnum(float NormalizedTime) const;

    float LastUpdateTime = 0.0f;
    static constexpr float UpdateInterval = 0.1f; // Update lighting every 100ms
};
