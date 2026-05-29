#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FRotator SunAngle = FRotator(-30.0f, 45.0f, 0.0f);
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savana,
    Pantano,
    Floresta,
    Deserto,
    Montanha
};

/**
 * Manages atmospheric effects and lighting for different Cretaceous biomes
 * Creates immersive prehistoric environments with proper lighting and fog
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TMap<EEnvArt_BiomeType, FEnvArt_BiomeAtmosphere> BiomeAtmospheres;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_BiomeType CurrentBiome = EEnvArt_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereTransitionTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableAtmosphericParticles = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateVolumetricFog(const FVector& Location, const FEnvArt_BiomeAtmosphere& Atmosphere);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericParticles(const FVector& Location, EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateDirectionalLighting(const FEnvArt_BiomeAtmosphere& Atmosphere);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphere GetBiomeAtmosphere(EEnvArt_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeBiomeAtmospheres();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmosphere")
    void ApplyCurrentAtmosphere();

private:
    void SetupDefaultAtmospheres();
    
    UPROPERTY()
    TArray<class AExponentialHeightFog*> SpawnedFogActors;

    UPROPERTY()
    TArray<class AEmitter*> SpawnedParticleActors;
};