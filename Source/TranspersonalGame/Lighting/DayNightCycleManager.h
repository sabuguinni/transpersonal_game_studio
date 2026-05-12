#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    FLight_TimeSettings()
    {
        SunElevation = -45.0f;
        SunAzimuth = 0.0f;
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);
        SkyTint = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_DayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<APostProcessVolume> PostProcessVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimePresets;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

public:
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimePreset(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    float GetCurrentTimeHours() const { return CurrentTime; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void PauseCycle() { bEnableCycle = false; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void ResumeCycle() { bEnableCycle = true; }

protected:
    void InitializeTimePresets();
    void UpdateLighting();
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdatePostProcess();
    ELight_TimeOfDay CalculateTimeOfDay() const;
    FLight_TimeSettings InterpolateTimeSettings(float TimeHours) const;
    void FindLightingActors();

private:
    bool bLightingActorsFound = false;
    float LastUpdateTime = 0.0f;
};

#include "DayNightCycleManager.generated.h"