#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphericType : uint8
{
    DustParticles    UMETA(DisplayName = "Dust Particles"),
    MistFog         UMETA(DisplayName = "Mist Fog"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    Pollen          UMETA(DisplayName = "Pollen"),
    SandStorm       UMETA(DisplayName = "Sand Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    EEnvArt_AtmosphericType AtmosphericType = EEnvArt_AtmosphericType::DustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    bool bAffectedByWeather = true;

    FEnvArt_AtmosphericSettings()
    {
        AtmosphericType = EEnvArt_AtmosphericType::DustParticles;
        Intensity = 1.0f;
        WindStrength = 0.5f;
        EffectScale = FVector(1.0f, 1.0f, 1.0f);
        bAffectedByWeather = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* ParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float EffectRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    bool bAutoActivate = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetAtmosphericType(EEnvArt_AtmosphericType NewType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetWindStrength(float NewWindStrength);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void ActivateAtmosphericEffect();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void DeactivateAtmosphericEffect();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void UpdateForBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void UpdateForWeather(EWeatherType WeatherType, float WeatherIntensity);

private:
    void InitializeParticleSystem();
    void InitializeAmbientAudio();
    void UpdateParticleParameters();
    void UpdateAudioParameters();

    float CurrentIntensity;
    float TargetIntensity;
    bool bIsActive;
};