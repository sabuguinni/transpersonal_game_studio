#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "EnvironmentAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
};

UENUM(BlueprintType)
enum class EEnvArt_BiomeAtmosphere : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Savanna     UMETA(DisplayName = "Savanna")
};

/**
 * Manages atmospheric effects and lighting for different biomes
 * Creates fog, sky atmosphere, and dynamic lighting based on biome type
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentAtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_BiomeAtmosphere CurrentBiome = EEnvArt_BiomeAtmosphere::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings ForestSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings SwampSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings DesertSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings MountainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings SavannaSettings;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLightActor;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EEnvArt_BiomeAtmosphere NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFogSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLightingSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere", CallInEditor)
    void RefreshAtmosphere();

protected:
    UFUNCTION()
    void InitializeAtmosphereComponents();

    UFUNCTION()
    FEnvArt_AtmosphereSettings GetSettingsForBiome(EEnvArt_BiomeAtmosphere Biome) const;

    UFUNCTION()
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);
};