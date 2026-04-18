#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "EnvironmentalAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Grassland   UMETA(DisplayName = "Grassland"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableParticleEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleIntensity = 1.0f;

    FEnvArt_AtmosphereSettings()
    {
        // Default constructor
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentalAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentalAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Atmosphere configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEnvArt_AtmosphereType AtmosphereType = EEnvArt_AtmosphereType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EEnvArt_TimeOfDay TimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    // Lighting references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    class AVolumetricCloud* VolumetricClouds;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystemComponent* DustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystemComponent* PollenParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystemComponent* MistParticles;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* AmbientAudio;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetAtmosphereType(EEnvArt_AtmosphereType NewType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FEnvArt_AtmosphereSettings GetPresetSettings(EEnvArt_AtmosphereType Type, EEnvArt_TimeOfDay Time);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void RefreshAtmosphere();

private:
    void FindLightingActors();
    void CreateParticleEffects();
    void SetupAmbientAudio();
    FEnvArt_AtmosphereSettings GetForestSettings(EEnvArt_TimeOfDay Time);
    FEnvArt_AtmosphereSettings GetVolcanicSettings(EEnvArt_TimeOfDay Time);
    FEnvArt_AtmosphereSettings GetSwampSettings(EEnvArt_TimeOfDay Time);
    FEnvArt_AtmosphereSettings GetGrasslandSettings(EEnvArt_TimeOfDay Time);
};