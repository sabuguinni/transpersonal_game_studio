#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentIntegrator.generated.h"

/**
 * Cretaceous Environment Integrator
 * Manages the integration and coordination of all Cretaceous period environmental systems
 * Handles atmospheric lighting, biome-specific props, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable, Category = "Environment|Cretaceous")
class TRANSPERSONALGAME_API AEnvArt_CretaceousEnvironmentIntegrator : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousEnvironmentIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC LIGHTING SYSTEM ===
    
    /** Main directional light for sun simulation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* MainSunLight;
    
    /** Exponential height fog for atmospheric depth */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class AExponentialHeightFog* AtmosphericFog;
    
    /** Golden hour lighting configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor GoldenHourColor;
    
    /** Sun intensity for different times of day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SunIntensity;
    
    /** Sun angle for golden hour effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator GoldenHourAngle;

    // === BIOME-SPECIFIC SYSTEMS ===
    
    /** Biome locations for prop placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeLocation> BiomeLocations;
    
    /** Volcanic biome lighting color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    FLinearColor VolcanicGlowColor;
    
    /** Swamp biome atmospheric color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    FLinearColor SwampAtmosphereColor;
    
    /** Forest biome ambient settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    FLinearColor ForestAmbientColor;

    // === PROP MANAGEMENT ===
    
    /** Array of spawned environment props */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<AActor*> SpawnedEnvironmentProps;
    
    /** Array of biome-specific lighting actors */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<class APointLight*> BiomeLights;
    
    /** Maximum number of props per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxPropsPerBiome;

    // === FOG AND ATMOSPHERE ===
    
    /** Fog density for Cretaceous atmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CretaceousFogDensity;
    
    /** Fog height falloff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FogHeightFalloff;
    
    /** Fog inscattering color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor;

    // === PERFORMANCE OPTIMIZATION ===
    
    /** Enable LOD system for environment props */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePropLOD;
    
    /** Distance for prop culling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float PropCullingDistance;
    
    /** Update frequency for atmospheric effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AtmosphericUpdateFrequency;

public:
    // === CORE FUNCTIONALITY ===
    
    /** Initialize all Cretaceous environment systems */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void InitializeCretaceousEnvironment();
    
    /** Spawn biome-specific props at designated locations */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SpawnBiomeProps();
    
    /** Configure atmospheric lighting for golden hour */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupGoldenHourLighting();
    
    /** Configure exponential height fog for Cretaceous atmosphere */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureCretaceousFog();
    
    /** Update biome-specific lighting based on time of day */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateBiomeLighting(float TimeOfDay);

    // === PROP MANAGEMENT ===
    
    /** Spawn fallen log prop at specified location */
    UFUNCTION(BlueprintCallable, Category = "Props")
    AActor* SpawnFallenLog(FVector Location, FRotator Rotation, const FString& BiomeName);
    
    /** Spawn ancient rock formation at specified location */
    UFUNCTION(BlueprintCallable, Category = "Props")
    AActor* SpawnAncientRock(FVector Location, FRotator Rotation, const FString& BiomeName);
    
    /** Spawn biome-specific atmospheric lighting */
    UFUNCTION(BlueprintCallable, Category = "Props")
    class APointLight* SpawnBiomeLight(FVector Location, FLinearColor Color, float Intensity, const FString& BiomeName);

    // === PERFORMANCE AND OPTIMIZATION ===
    
    /** Optimize environment props based on player distance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentProps();
    
    /** Get current atmospheric performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetAtmosphericPerformanceMetrics() const;
    
    /** Validate environment integration status */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool ValidateEnvironmentIntegration() const;

private:
    /** Timer for atmospheric updates */
    FTimerHandle AtmosphericUpdateTimer;
    
    /** Timer for prop optimization */
    FTimerHandle PropOptimizationTimer;
    
    /** Internal atmospheric update function */
    void UpdateAtmosphericEffects();
    
    /** Internal prop optimization function */
    void InternalOptimizeProps();
    
    /** Find or create main directional light */
    void FindOrCreateMainSunLight();
    
    /** Find or create exponential height fog */
    void FindOrCreateAtmosphericFog();
};