#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../SharedTypes.h"
#include "Core_TerrainPhysicsSystem.generated.h"

/**
 * Core Terrain Physics System
 * Manages realistic terrain-based physics interactions for prehistoric survival gameplay.
 * Handles terrain material properties, slope physics, and environmental physics effects.
 * 
 * Key Features:
 * - Terrain material physics (mud, rock, sand, grass, water)
 * - Slope-based movement physics and stability
 * - Weather-based terrain modification (rain makes mud slippery)
 * - Realistic footstep physics and terrain deformation
 * - Integration with UE5 Landscape system and Chaos Physics
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainMaterial
{
    GENERATED_BODY()

    /** Base friction coefficient for this terrain type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float BaseFriction = 0.7f;

    /** Bounciness/restitution of the terrain material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.1f;

    /** How much this terrain absorbs impact energy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Damping = 0.8f;

    /** Density of the terrain material (affects sinking/footprints) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    /** How much this terrain can deform under pressure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DeformationResistance = 0.5f;

    /** Sound attenuation factor for footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundAttenuation = 1.0f;

    /** Particle effect intensity for footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleIntensity = 1.0f;

    FCore_TerrainMaterial()
    {
        BaseFriction = 0.7f;
        Restitution = 0.1f;
        Damping = 0.8f;
        Density = 1.0f;
        DeformationResistance = 0.5f;
        SoundAttenuation = 1.0f;
        ParticleIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SlopePhysics
{
    GENERATED_BODY()

    /** Current slope angle in degrees */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float SlopeAngle = 0.0f;

    /** Maximum stable angle before sliding occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxStableAngle = 45.0f;

    /** Friction multiplier based on slope */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    float SlopeFrictionMultiplier = 1.0f;

    /** Whether object is currently sliding down slope */
    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    bool bIsSliding = false;

    /** Sliding acceleration factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SlidingAcceleration = 9.8f;

    FCore_SlopePhysics()
    {
        SlopeAngle = 0.0f;
        MaxStableAngle = 45.0f;
        SlopeFrictionMultiplier = 1.0f;
        bIsSliding = false;
        SlidingAcceleration = 9.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WeatherPhysicsModifier
{
    GENERATED_BODY()

    /** Current weather intensity (0.0 = none, 1.0 = maximum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.0f;

    /** Friction reduction due to rain/water */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WetnessFrictionReduction = 0.3f;

    /** How much rain affects terrain deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainDeformationMultiplier = 1.5f;

    /** Wind force affecting objects on terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindForce = FVector::ZeroVector;

    FCore_WeatherPhysicsModifier()
    {
        WeatherIntensity = 0.0f;
        WetnessFrictionReduction = 0.3f;
        RainDeformationMultiplier = 1.5f;
        WindForce = FVector::ZeroVector;
    }
};

/**
 * Core Terrain Physics Component
 * Attached to actors that need terrain-aware physics simulation
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Current terrain material properties under this actor */
    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics")
    FCore_TerrainMaterial CurrentTerrainMaterial;

    /** Current slope physics state */
    UPROPERTY(BlueprintReadOnly, Category = "Terrain Physics")
    FCore_SlopePhysics CurrentSlopePhysics;

    /** Weather-based physics modifiers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_WeatherPhysicsModifier WeatherModifier;

    /** Whether this component should automatically update terrain physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bAutoUpdateTerrainPhysics = true;

    /** How often to update terrain physics (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float UpdateFrequency = 0.1f;

    /** Trace distance for terrain detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float TerrainTraceDistance = 200.0f;

    /** Update terrain physics based on current location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics();

    /** Get terrain material at specific location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainMaterial GetTerrainMaterialAtLocation(const FVector& Location);

    /** Calculate slope physics for current location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_SlopePhysics CalculateSlopePhysics(const FVector& Location);

    /** Apply weather effects to terrain physics */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyWeatherEffects(float WeatherIntensity, EWeatherType WeatherType);

    /** Get effective friction coefficient including all modifiers */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetEffectiveFriction() const;

    /** Check if actor should slide on current terrain */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ShouldSlideOnTerrain() const;

    /** Apply terrain-based physics forces to actor */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainForces();

private:
    /** Timer for terrain physics updates */
    float UpdateTimer = 0.0f;

    /** Cached landscape reference for performance */
    UPROPERTY()
    ALandscape* CachedLandscape = nullptr;

    /** Find landscape actor in the world */
    void FindLandscape();

    /** Trace to terrain and get surface normal */
    bool TraceToTerrain(const FVector& Location, FVector& OutHitLocation, FVector& OutSurfaceNormal);

    /** Get physical material from landscape at location */
    UPhysicalMaterial* GetLandscapePhysicalMaterial(const FVector& Location);

    /** Convert physical material to terrain material properties */
    FCore_TerrainMaterial ConvertPhysicalMaterialToTerrainMaterial(UPhysicalMaterial* PhysMat);
};

/**
 * Core Terrain Physics Subsystem
 * World-level manager for terrain physics systems
 */
UCLASS()
class TRANSPERSONALGAME_API UCore_TerrainPhysicsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Register an actor for terrain physics updates */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterActorForTerrainPhysics(AActor* Actor);

    /** Unregister an actor from terrain physics updates */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UnregisterActorFromTerrainPhysics(AActor* Actor);

    /** Update global weather effects */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateGlobalWeatherEffects(float WeatherIntensity, EWeatherType WeatherType);

    /** Get terrain physics component from actor */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    UCore_TerrainPhysicsComponent* GetTerrainPhysicsComponent(AActor* Actor);

    /** Batch update all registered terrain physics components */
    void BatchUpdateTerrainPhysics();

protected:
    /** All actors registered for terrain physics */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    /** Current global weather state */
    UPROPERTY()
    FCore_WeatherPhysicsModifier GlobalWeatherState;

    /** Timer for batch updates */
    FTimerHandle BatchUpdateTimer;

    /** Start batch update timer */
    void StartBatchUpdateTimer();

    /** Clean up invalid actor references */
    void CleanupInvalidActors();
};