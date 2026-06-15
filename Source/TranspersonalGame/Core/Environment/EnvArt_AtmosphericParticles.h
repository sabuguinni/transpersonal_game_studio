#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericParticles.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_ParticleType : uint8
{
    Pollen UMETA(DisplayName = "Pollen"),
    Dust UMETA(DisplayName = "Dust Motes"),
    Spores UMETA(DisplayName = "Spores"),
    Ash UMETA(DisplayName = "Volcanic Ash"),
    Mist UMETA(DisplayName = "Water Mist"),
    Insects UMETA(DisplayName = "Flying Insects")
};

UENUM(BlueprintType)
enum class EEnvArt_ParticleIntensity : uint8
{
    Light UMETA(DisplayName = "Light"),
    Moderate UMETA(DisplayName = "Moderate"),
    Heavy UMETA(DisplayName = "Heavy"),
    Dense UMETA(DisplayName = "Dense")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings")
    EEnvArt_ParticleType ParticleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings")
    EEnvArt_ParticleIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ParticleSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings")
    FLinearColor ParticleColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float LifeSpan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Settings", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float WindStrength;

    FEnvArt_ParticleSettings()
    {
        ParticleType = EEnvArt_ParticleType::Pollen;
        Intensity = EEnvArt_ParticleIntensity::Moderate;
        SpawnRate = 1.0f;
        ParticleSize = 1.0f;
        ParticleColor = FLinearColor::Yellow;
        LifeSpan = 10.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindStrength = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericParticles : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericParticles();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* ParticleSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* EffectVolume;

    // Particle Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    FEnvArt_ParticleSettings ParticleSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    bool bAutoActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    bool bAffectedByWind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    bool bAffectedByLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    bool bPlayerTriggered;

    // Environmental Conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    float TemperatureThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    float HumidityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    bool bActiveInRain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Conditions")
    bool bActiveAtNight;

    // Time of Day Modulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Modulation")
    class UCurveFloat* IntensityByTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Modulation")
    class UCurveFloat* ColorByTimeOfDay;

    // Particle Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    TArray<class UParticleSystem*> ParticleSystemVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    class UParticleSystem* CurrentParticleSystem;

public:
    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void ActivateParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void DeactivateParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void SetParticleType(EEnvArt_ParticleType NewType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void SetParticleIntensity(EEnvArt_ParticleIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void UpdateParticleSettings(const FEnvArt_ParticleSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void SetWindParameters(FVector Direction, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void ModulateByTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    void ModulateByWeather(float Temperature, float Humidity, bool bIsRaining);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Particles")
    bool IsActiveForCurrentConditions() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnParticlesActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnParticlesDeactivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnPlayerEnteredParticleZone();

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmospheric Events")
    void OnPlayerExitedParticleZone();

protected:
    UFUNCTION()
    void OnEffectVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEffectVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void InitializeParticleSystem();
    void UpdateParticleSystemProperties();
    void ApplyEnvironmentalModulation();
    void HandlePlayerProximity();

    // Internal state
    bool bIsActive;
    bool bPlayerInRange;
    float CurrentTimeOfDay;
    float CurrentTemperature;
    float CurrentHumidity;
    bool bCurrentlyRaining;
};