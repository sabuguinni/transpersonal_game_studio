#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/AtmosphericFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "Light_SwampAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_SwampLightingMode : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherCondition : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Misty       UMETA(DisplayName = "Misty"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SwampLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    float SunIntensity = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.96f, 0.82f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.78f, 0.86f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    bool bVolumetricFogEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Lighting")
    float VolumetricScatteringDistribution = 0.2f;

    FLight_SwampLightingData()
    {
        // Default constructor
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FireLightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float Intensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLinearColor Color = FLinearColor(1.0f, 0.71f, 0.39f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float AttenuationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float Temperature = 2700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    bool bFlickerEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerIntensity = 0.2f;

    FLight_FireLightData()
    {
        // Default constructor
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_SwampFireLight : public APointLight
{
    GENERATED_BODY()

public:
    ALight_SwampFireLight();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Light", meta = (AllowPrivateAccess = "true"))
    FLight_FireLightData FireLightData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Light", meta = (AllowPrivateAccess = "true"))
    bool bIsFlickering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Light", meta = (AllowPrivateAccess = "true"))
    float FlickerTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Light", meta = (AllowPrivateAccess = "true"))
    float BaseIntensity = 800.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Fire Light")
    void SetFireLightData(const FLight_FireLightData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Fire Light")
    void StartFlickering();

    UFUNCTION(BlueprintCallable, Category = "Fire Light")
    void StopFlickering();

    UFUNCTION(BlueprintPure, Category = "Fire Light")
    FLight_FireLightData GetFireLightData() const { return FireLightData; }

private:
    void UpdateFlicker(float DeltaTime);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_SwampAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ALight_SwampAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    ELight_SwampLightingMode CurrentLightingMode = ELight_SwampLightingMode::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    ELight_WeatherCondition CurrentWeather = ELight_WeatherCondition::Misty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    FLight_SwampLightingData LightingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    float DayNightCycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    bool bAutomaticDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swamp Atmosphere", meta = (AllowPrivateAccess = "true"))
    float CurrentTimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon

    // Cached references
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    AExponentialHeightFog* VolumetricFog;

    UPROPERTY()
    AAtmosphericFog* AtmosphericFog;

    UPROPERTY()
    TArray<ALight_SwampFireLight*> FireLights;

public:
    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void SetLightingMode(ELight_SwampLightingMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void SetWeatherCondition(ELight_WeatherCondition NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void SetTimeOfDay(float TimeValue); // 0.0 to 1.0

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void EnableDayNightCycle(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void SpawnFireLightAtLocation(const FVector& Location, const FLight_FireLightData& FireData);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere")
    void RemoveAllFireLights();

    UFUNCTION(BlueprintPure, Category = "Swamp Atmosphere")
    ELight_SwampLightingMode GetCurrentLightingMode() const { return CurrentLightingMode; }

    UFUNCTION(BlueprintPure, Category = "Swamp Atmosphere")
    ELight_WeatherCondition GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Swamp Atmosphere")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

private:
    void InitializeAtmosphereComponents();
    void UpdateLighting();
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyLightingMode();
    void ApplyWeatherEffects();
    FLight_SwampLightingData GetLightingDataForMode(ELight_SwampLightingMode Mode) const;
    FLinearColor GetSunColorForTime(float TimeOfDay) const;
    float GetSunIntensityForTime(float TimeOfDay) const;
};

UCLASS()
class TRANSPERSONALGAME_API ULight_SwampAtmosphereSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere Subsystem")
    void RegisterSwampAtmosphereController(ALight_SwampAtmosphereController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere Subsystem")
    void UnregisterSwampAtmosphereController(ALight_SwampAtmosphereController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere Subsystem")
    ALight_SwampAtmosphereController* GetSwampAtmosphereController() const;

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere Subsystem")
    void SetGlobalSwampLighting(ELight_SwampLightingMode Mode, ELight_WeatherCondition Weather);

    UFUNCTION(BlueprintCallable, Category = "Swamp Atmosphere Subsystem")
    void CreateSwampFireLightsInRadius(const FVector& Center, float Radius, int32 Count);

protected:
    UPROPERTY()
    ALight_SwampAtmosphereController* ActiveController;

    UPROPERTY()
    TArray<ALight_SwampFireLight*> ManagedFireLights;

private:
    void CleanupInvalidReferences();
};

#include "Light_SwampAtmosphereSystem.generated.h"