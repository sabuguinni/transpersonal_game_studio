#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "EnvArt_BiomeAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereProfile : uint8
{
    CretaceousGoldenHour    UMETA(DisplayName = "Cretaceous Golden Hour"),
    VolcanicHaze           UMETA(DisplayName = "Volcanic Atmospheric Haze"),
    SwampMist              UMETA(DisplayName = "Swamp Mist"),
    ForestCanopy           UMETA(DisplayName = "Forest Canopy Filtering"),
    DesertClear            UMETA(DisplayName = "Desert Clear Air"),
    MountainThin           UMETA(DisplayName = "Mountain Thin Air")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 15.0f; // Golden hour angle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm golden

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaze = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float DustParticleDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float PollenParticleDensity = 0.05f;

    FEnvArt_AtmosphereSettings()
    {
        // Default constructor with Cretaceous golden hour settings
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_BiomeAtmosphereManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_BiomeAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core atmosphere management
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetAtmosphereProfile(EEnvArt_AtmosphereProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyBiomeAtmosphere(EBiomeType BiomeType, float TransitionWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void EnableVolcanicHaze(float Intensity = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetupVolumetricFog(const FEnvArt_AtmosphereSettings& Settings);

    // Particle atmosphere effects
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SpawnDustParticles(FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SpawnPollenParticles(FVector Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void CreateAtmosphericGodRays(FVector SunDirection);

    // Biome-specific atmosphere
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetSwampMistAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetForestCanopyAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetDesertClearAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetMountainThinAtmosphere();

    // Real-time atmosphere updates
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateAtmosphereForPlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void BlendAtmosphereBetweenBiomes(EBiomeType FromBiome, EBiomeType ToBiome, float BlendFactor);

protected:
    // Atmosphere settings per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    TMap<EBiomeType, FEnvArt_AtmosphereSettings> BiomeAtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    FEnvArt_AtmosphereSettings CurrentAtmosphereSettings;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    // Particle systems for atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    TObjectPtr<UNiagaraSystem> DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    TObjectPtr<UNiagaraSystem> PollenParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    TObjectPtr<UNiagaraSystem> GodRayParticleSystem;

    // Active particle components
    UPROPERTY(BlueprintReadOnly, Category = "Active Effects")
    TArray<TObjectPtr<UNiagaraComponent>> ActiveParticleComponents;

    // Atmosphere transition
    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    FEnvArt_AtmosphereSettings TransitionStartSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    FEnvArt_AtmosphereSettings TransitionTargetSettings;

private:
    // Internal helper methods
    void InitializeAtmosphereComponents();
    void FindWorldAtmosphereActors();
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);
    void UpdateTransition(float DeltaTime);
    FEnvArt_AtmosphereSettings GetDefaultSettingsForBiome(EBiomeType BiomeType);
    void CleanupInactiveParticles();
};