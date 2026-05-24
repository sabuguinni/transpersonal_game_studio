#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvArt_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Swamp_SW        UMETA(DisplayName = "Swamp Southwest"),
    Forest_NW       UMETA(DisplayName = "Forest Northwest"),
    Savanna_Center  UMETA(DisplayName = "Savanna Center"),
    Desert_E        UMETA(DisplayName = "Desert East"),
    Mountain_NE     UMETA(DisplayName = "Mountain Northeast")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAnglePitch = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngleYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricScatteringIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.8f;

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with golden hour settings
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType BiomeType = EEnvArt_BiomeType::Forest_NW;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableParticleEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnableAmbientSounds = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetBiomeType(EEnvArt_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SpawnBiomeSpecificEffects();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void PreviewAtmosphericEffects();

    UFUNCTION(BlueprintPure, Category = "Atmospheric System")
    FEnvArt_AtmosphericSettings GetCurrentAtmosphericSettings() const { return AtmosphericSettings; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric System")
    EEnvArt_BiomeType GetBiomeType() const { return BiomeType; }

private:
    void ConfigureDirectionalLight();
    void ConfigureSkyAtmosphere();
    void CreateVolumetricFogEffects();
    void SetupAmbientSoundTriggers();
    void SpawnParticleEffects();

    // Cached references to world lighting actors
    UPROPERTY()
    class ADirectionalLight* CachedSunLight;

    UPROPERTY()
    class ASkyAtmosphere* CachedSkyAtmosphere;
};