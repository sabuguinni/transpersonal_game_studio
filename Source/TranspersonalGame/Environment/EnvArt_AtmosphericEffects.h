#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/VolumetricCloud.h"
#include "EnvArt_AtmosphericEffects.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableDustMotes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float DustDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAmbientSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with initialization list above
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericEffects : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericEffects();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* DustMotesParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* ForestAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WindAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DistantRoarAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_AtmosphereType AtmosphereType = EEnvArt_AtmosphereType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ADirectionalLight* SunLightRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AExponentialHeightFog* HeightFogRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ASkyLight* SkyLightRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AVolumetricCloud* VolumetricCloudRef;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereType(EEnvArt_AtmosphereType NewType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor = true)
    void RefreshAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeOfDayHours);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherIntensity(float WeatherIntensity);

protected:
    UFUNCTION()
    void FindLightingActors();

    UFUNCTION()
    void ConfigureForestAtmosphere();

    UFUNCTION()
    void ConfigureSwampAtmosphere();

    UFUNCTION()
    void ConfigureSavannaAtmosphere();

    UFUNCTION()
    void ConfigureDesertAtmosphere();

    UFUNCTION()
    void ConfigureMountainAtmosphere();

private:
    float CurrentTimeOfDay = 12.0f; // Noon by default
    float CurrentWeatherIntensity = 0.5f;
    bool bIsInitialized = false;
};