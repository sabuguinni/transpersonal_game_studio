#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunElevation = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleDensity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float WindStrength = 1.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* VolumetricFogActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AtmosphericParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* ForestAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* WindSound;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetWindStrength(float Strength);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyMidnightLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void CreateVolumetricFogZone(FVector Location, FVector Scale);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SpawnAtmosphericParticles(FVector Location, float Density);

private:
    UPROPERTY()
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY()
    float CurrentWeatherIntensity = 0.5f;

    void UpdateLightingBasedOnTime();
    void UpdateFogBasedOnWeather();
    void UpdateParticleEffects();
    void UpdateAmbientAudio();
};