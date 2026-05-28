#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FRotator SunRotation = FRotator(-45.0f, 135.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float AtmosphericScattering = 1.2f;
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn,
    Morning,
    Midday,
    Afternoon,
    Sunset,
    Twilight,
    Night,
    PreDawn
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULight_AtmosphericManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULight_AtmosphericManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Atmospheric lighting control
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetupCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericLighting(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureSkyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureHeightFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigurePostProcessing();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SpawnAtmosphericFillLights();

    // Time of day cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    float CurrentTimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Midday;

protected:
    // Lighting actors references
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY()
    AExponentialHeightFog* HeightFogActor;

    UPROPERTY()
    APostProcessVolume* PostProcessVolume;

    UPROPERTY()
    TArray<APointLight*> FillLights;

    // Time of day settings
    UPROPERTY(EditAnywhere, Category = "Atmospheric Settings")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDaySettings;

    // Internal methods
    void FindLightingActors();
    void InitializeTimeOfDaySettings();
    FLight_TimeOfDaySettings GetCurrentTimeSettings() const;
    void InterpolateToTimeSettings(const FLight_TimeOfDaySettings& TargetSettings, float Alpha);
    ELight_TimeOfDay CalculateTimePhase(float TimeOfDay) const;
};