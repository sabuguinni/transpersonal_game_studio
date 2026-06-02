#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Light_DayNightCycleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    bool bRealTimeProgression = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float TimeMultiplier = 1.0f; // Speed up/slow down time

    FLight_TimeOfDaySettings()
    {
        TimeOfDay = 12.0f;
        DayDurationMinutes = 20.0f;
        bRealTimeProgression = true;
        TimeMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SunLightSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor NoonColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor DuskColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    FLinearColor NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float DawnIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float NoonIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float DuskIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Light")
    float NightIntensity = 0.1f;

    FLight_SunLightSettings()
    {
        DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
        NoonColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        DuskColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
        NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
        DawnIntensity = 2.0f;
        NoonIntensity = 5.0f;
        DuskIntensity = 1.5f;
        NightIntensity = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorptionScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    FLight_AtmosphereSettings()
    {
        RayleighScatteringScale = 0.4f;
        MieScatteringScale = 0.05f;
        MieAbsorptionScale = 0.01f;
        FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
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

    // Core lighting components references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components", meta = (AllowPrivateAccess = "true"))
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components", meta = (AllowPrivateAccess = "true"))
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components", meta = (AllowPrivateAccess = "true"))
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components", meta = (AllowPrivateAccess = "true"))
    class AExponentialHeightFog* HeightFog;

    // Configuration settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    FLight_TimeOfDaySettings TimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    FLight_SunLightSettings SunSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    FLight_AtmosphereSettings AtmosphereSettings;

    // Runtime state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    float CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    bool bIsNightTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    float SunAngle;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNightTime() const { return bIsNightTime; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseTimeProgression(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentSunIntensity() const;

protected:
    // Internal methods
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateAtmosphere();
    void UpdateFog();
    void FindLightingActors();
    
    // Helper functions
    float CalculateSunAngle(float TimeOfDay) const;
    FLinearColor InterpolateSunColor(float TimeOfDay) const;
    float InterpolateSunIntensity(float TimeOfDay) const;
    FRotator CalculateSunRotation(float SunAngle) const;

private:
    bool bTimeProgressionPaused;
    float AccumulatedTime;
};