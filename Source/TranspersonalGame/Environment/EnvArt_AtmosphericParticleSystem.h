#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericParticleSystem.generated.h"

/**
 * Atmospheric Particle System for Cretaceous environments
 * Manages dust, pollen, volcanic ash, and other atmospheric effects
 * Responds to weather conditions and biome types
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericParticleSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericParticleSystem();

protected:
    virtual void BeginPlay() override;

    /** Root component for the particle system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    /** Main particle system component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* ParticleSystemComponent;

    /** Trigger volume for particle activation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* ActivationTrigger;

    /** Type of atmospheric particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    EEnvArt_AtmosphericParticleType ParticleType;

    /** Particle intensity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ParticleIntensity;

    /** Wind direction affecting particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    FVector WindDirection;

    /** Wind strength affecting particle movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WindStrength;

    /** Whether particles respond to weather changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    bool bWeatherResponsive;

    /** Current weather condition affecting particles */
    UPROPERTY(BlueprintReadOnly, Category = "Atmospheric Effects")
    EEnvArt_WeatherType CurrentWeather;

    /** Biome-specific particle settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EEnvArt_BiomeType BiomeType;

    /** Particle spawn rate multiplier for different biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    float BiomeSpawnRateMultiplier;

public:
    virtual void Tick(float DeltaTime) override;

    /** Update particle system based on weather conditions */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void UpdateWeatherEffects(EEnvArt_WeatherType NewWeather);

    /** Set particle intensity */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SetParticleIntensity(float NewIntensity);

    /** Set wind parameters */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SetWindParameters(FVector NewDirection, float NewStrength);

    /** Activate particle system */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void ActivateParticleSystem();

    /** Deactivate particle system */
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void DeactivateParticleSystem();

    /** Check if particle system is active */
    UFUNCTION(BlueprintPure, Category = "Atmospheric Effects")
    bool IsParticleSystemActive() const;

protected:
    /** Configure particle system for specific biome */
    UFUNCTION(BlueprintCallable, Category = "Biome Settings")
    void ConfigureForBiome(EEnvArt_BiomeType NewBiomeType);

    /** Update particle parameters based on current settings */
    void UpdateParticleParameters();

    /** Handle trigger overlap events */
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    /** Timer for weather update checks */
    FTimerHandle WeatherUpdateTimer;

    /** Current activation state */
    bool bIsActive;

    /** Player is within trigger range */
    bool bPlayerInRange;
};