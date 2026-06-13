#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor LightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightIntensity;

    FEnvArt_AtmosphericZone()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
        FogDensity = 0.1f;
        LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        LightIntensity = 8.0f;
    }
};

USTRUCT(BlueprintType)
struct FEnvArt_ParticleEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float SpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bIsActive;

    FEnvArt_ParticleEffect()
    {
        EffectName = TEXT("DefaultParticles");
        SpawnLocation = FVector::ZeroVector;
        SpawnRate = 10.0f;
        ParticleLifetime = 5.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmospheric zones management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Zones")
    TArray<FEnvArt_AtmosphericZone> AtmosphericZones;

    // Particle effects management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects")
    TArray<FEnvArt_ParticleEffect> ParticleEffects;

    // Time of day settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationInMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableDayNightCycle;

    // Weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsFoggy;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Atmospheric functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphericZone(const FString& ZoneName, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLightingForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnParticleEffect(const FString& EffectName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherConditions(bool bRain, bool bFog, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FEnvArt_AtmosphericZone GetCurrentAtmosphericZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeDefaultAtmosphericZones();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericEffects(float DeltaTime);

private:
    // Internal state
    FString CurrentZoneName;
    float TimeAccumulator;
    
    // Helper functions
    void UpdateDirectionalLight();
    void UpdateVolumetricFog();
    void UpdateParticleEffects();
    void CalculateAtmosphericBlending(const FVector& PlayerLocation);
};