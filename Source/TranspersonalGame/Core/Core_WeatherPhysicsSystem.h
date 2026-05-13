#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "Core_WeatherPhysicsSystem.generated.h"

/**
 * Core Weather Physics System
 * Handles realistic weather effects on physics simulation:
 * - Rain impact on surfaces and character movement
 * - Wind forces affecting objects and projectiles
 * - Storm dynamics with lightning physics
 * - Temperature effects on material properties
 * - Atmospheric pressure simulation
 */

UENUM(BlueprintType)
enum class ECore_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    Wind            UMETA(DisplayName = "High Wind"),
    Hail            UMETA(DisplayName = "Hailstorm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WeatherPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics")
    ECore_WeatherType WeatherType = ECore_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WindSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics", meta = (ClampMin = "-50.0", ClampMax = "50.0"))
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics", meta = (ClampMin = "900.0", ClampMax = "1100.0"))
    float AtmosphericPressure = 1013.25f;

    FCore_WeatherPhysicsData()
    {
        WeatherType = ECore_WeatherType::Clear;
        Intensity = 0.5f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindSpeed = 10.0f;
        Temperature = 20.0f;
        Humidity = 0.6f;
        AtmosphericPressure = 1013.25f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PrecipitationPhysics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float DropletMass = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float TerminalVelocity = 9.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float ImpactForce = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float SurfaceWetness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float AccumulationRate = 0.01f;

    FCore_PrecipitationPhysics()
    {
        DropletMass = 0.001f;
        TerminalVelocity = 9.0f;
        ImpactForce = 0.1f;
        SurfaceWetness = 0.0f;
        AccumulationRate = 0.01f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_WeatherPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_WeatherPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather System Control
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetWeatherType(ECore_WeatherType NewWeatherType, float NewIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetWindParameters(FVector Direction, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetAtmosphericConditions(float Temperature, float Humidity, float Pressure);

    // Physics Effects
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void ApplyWindForceToActor(AActor* TargetActor, float ForceMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void ApplyPrecipitationEffects(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SimulateStormLightning(FVector StrikeLocation);

    // Environmental Impact
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    float CalculateMovementModifier(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    float CalculateVisibilityModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    float CalculateFrictionModifier(const FVector& SurfaceNormal) const;

    // Weather Transition
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void TransitionToWeather(ECore_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    bool IsWeatherTransitioning() const { return bIsTransitioning; }

    // Data Access
    UFUNCTION(BlueprintPure, Category = "Weather Physics")
    FCore_WeatherPhysicsData GetCurrentWeatherData() const { return CurrentWeatherData; }

    UFUNCTION(BlueprintPure, Category = "Weather Physics")
    FCore_PrecipitationPhysics GetPrecipitationData() const { return PrecipitationData; }

    // Debug Functions
    UFUNCTION(CallInEditor, Category = "Weather Physics")
    void DebugWeatherEffects();

    UFUNCTION(CallInEditor, Category = "Weather Physics")
    void TestStormSimulation();

protected:
    // Core Weather Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    FCore_WeatherPhysicsData CurrentWeatherData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    FCore_PrecipitationPhysics PrecipitationData;

    // Wind Physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    float WindForceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    float WindTurbulenceScale = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    float MaxWindForce = 1000.0f;

    // Precipitation Physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation Physics")
    float RainDropDensity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation Physics")
    float HailImpactForce = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation Physics")
    float WetnessDecayRate = 0.1f;

    // Lightning Physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Physics")
    float LightningStrikeForce = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Physics")
    float LightningRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Physics")
    float ElectromagneticPulseStrength = 100.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAffectedActors = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdvancedPhysics = true;

private:
    // Internal State
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;
    FCore_WeatherPhysicsData TargetWeatherData;

    // Cached References
    UPROPERTY()
    UWorld* CachedWorld = nullptr;

    // Performance Tracking
    float LastUpdateTime = 0.0f;
    int32 ProcessedActorsThisFrame = 0;

    // Internal Methods
    void UpdateWeatherTransition(float DeltaTime);
    void ProcessWeatherEffects(float DeltaTime);
    void ApplyWindTurbulence(AActor* Actor, float DeltaTime);
    void UpdateSurfaceWetness(float DeltaTime);
    FVector CalculateWindForce(const FVector& ActorLocation, float ActorMass) const;
    float CalculateAtmosphericDensity() const;
    void SpawnLightningEffects(const FVector& Location);
};