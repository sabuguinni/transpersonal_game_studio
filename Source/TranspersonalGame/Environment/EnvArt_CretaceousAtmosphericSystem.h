#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "EnvArt_CretaceousAtmosphericSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericParticleData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    class UParticleSystem* DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    class UParticleSystem* PollenParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    class UParticleSystem* SporeParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    float ParticleIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    FVector WindDirection;

    FEnvArt_AtmosphericParticleData()
    {
        DustParticleSystem = nullptr;
        PollenParticleSystem = nullptr;
        SporeParticleSystem = nullptr;
        ParticleIntensity = 1.0f;
        WindStrength = 0.5f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float AtmosphericFogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float VolumetricFogExtinction;

    FEnvArt_CretaceousLightingData()
    {
        SunAngle = 45.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 8.0f;
        AtmosphericFogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        VolumetricFogExtinction = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AmbientSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    class USoundCue* WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    class USoundCue* BirdCallsSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    class USoundCue* DistantRoarsSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    class USoundCue* InsectBuzzSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Sounds")
    float SoundRange;

    FEnvArt_AmbientSoundData()
    {
        WindSound = nullptr;
        BirdCallsSound = nullptr;
        DistantRoarsSound = nullptr;
        InsectBuzzSound = nullptr;
        AmbientVolume = 0.5f;
        SoundRange = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousAtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousAtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmospheric components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* PollenParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* SporeParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BirdCallsAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DistantRoarsAudioComponent;

    // Atmospheric data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FEnvArt_AtmosphericParticleData ParticleData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FEnvArt_CretaceousLightingData LightingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FEnvArt_AmbientSoundData AmbientSoundData;

    // Time of day control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeAdvanceSpeed;

    // Weather control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsStormy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float HumidityLevel;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetMidDayLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetDuskLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAtmosphericParticles(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAmbientSounds(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetWeatherCondition(bool bStormy, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    FLinearColor GetCurrentSkyColor() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    float GetCurrentLightIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SpawnAtmosphericEffectAtLocation(const FVector& Location, float Radius);

private:
    // Internal update functions
    void UpdateLightingBasedOnTime();
    void UpdateParticleSystemsBasedOnWeather();
    void UpdateAmbientAudioBasedOnConditions();
    
    // Helper functions
    FLinearColor CalculateSkyColorForTime(float Time) const;
    float CalculateLightIntensityForTime(float Time) const;
    FVector CalculateWindDirectionForWeather() const;
    
    // Internal state
    float LastTimeUpdate;
    bool bSystemInitialized;
};