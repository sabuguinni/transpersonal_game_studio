#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "../SharedTypes.h"
#include "../Engine/Eng_BiomeManager.h"
#include "Core_BiomePhysicsManager.generated.h"

/**
 * BIOME PHYSICS MANAGER - CORE SYSTEMS
 * Core Systems Programmer Agent #03
 * 
 * Manages physics behavior based on biome types.
 * Integrates with BiomeManager to provide biome-specific collision layers,
 * physics materials, and destruction parameters.
 * 
 * FEATURES:
 * - Biome-specific collision channels
 * - Dynamic physics material assignment
 * - Terrain-based movement modifiers
 * - Destruction thresholds per biome type
 * - Performance-optimized collision detection
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_BiomePhysicsParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    // Collision properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionRadius = 100.0f;

    // Physics material properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Density = 1.0f;

    // Movement modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bAllowJumping = true;

    // Destruction parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float RockHardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction = true;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TemperatureDamageRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bCausesSlowdown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float NoiseLevel = 1.0f;

    FCore_BiomePhysicsParameters()
    {
        BiomeType = EEng_BiomeType::Grassland;
        CollisionChannel = ECC_WorldStatic;
        CollisionRadius = 100.0f;
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        MovementSpeedMultiplier = 1.0f;
        StaminaDrainMultiplier = 1.0f;
        bAllowJumping = true;
        DestructionThreshold = 1000.0f;
        RockHardness = 1.0f;
        bEnableDestruction = true;
        TemperatureDamageRate = 0.0f;
        bCausesSlowdown = false;
        NoiseLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionLayerMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> GroundChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> VegetationChannel = ECC_WorldDynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> RockChannel = ECC_Destructible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> WaterChannel = ECC_WorldStatic;

    FCore_CollisionLayerMapping()
    {
        BiomeType = EEng_BiomeType::Grassland;
        GroundChannel = ECC_WorldStatic;
        VegetationChannel = ECC_WorldDynamic;
        RockChannel = ECC_Destructible;
        WaterChannel = ECC_WorldStatic;
    }
};

/**
 * Biome Physics Manager Component
 * Handles physics behavior based on current biome
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_BiomePhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_BiomePhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core physics functions
    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    FCore_BiomePhysicsParameters GetPhysicsParametersAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void ApplyBiomePhysicsToActor(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Physics")
    void UpdateActorPhysicsForCurrentBiome(AActor* Actor);

    // Collision layer management
    UFUNCTION(BlueprintCallable, Category = "Collision")
    TEnumAsByte<ECollisionChannel> GetCollisionChannelForBiome(EEng_BiomeType BiomeType, const FString& ObjectType) const;

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetupCollisionLayersForActor(AActor* Actor, EEng_BiomeType BiomeType);

    // Movement and physics effects
    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetMovementSpeedMultiplier(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetStaminaDrainMultiplier(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool CanJumpAtLocation(const FVector& Location) const;

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanDestroyAtLocation(const FVector& Location, float ImpactForce) const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ProcessDestruction(AActor* Actor, const FVector& ImpactLocation, float ImpactForce);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureDamageRate(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CausesSlowdown(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetNoiseLevel(const FVector& Location) const;

    // Configuration and setup
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBiomePhysicsParameters(EEng_BiomeType BiomeType, const FCore_BiomePhysicsParameters& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void InitializeDefaultBiomePhysics();

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidatePhysicsConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawPhysicsInfo(const FVector& Location);

protected:
    // Physics configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EEng_BiomeType, FCore_BiomePhysicsParameters> BiomePhysicsMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FCore_CollisionLayerMapping> CollisionLayerMappings;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsOptimization = true;

    // Component references
    UPROPERTY()
    class UEng_BiomeManager* BiomeManager;

private:
    // Internal helper functions
    void CacheNearbyActors();
    void UpdateCachedActorPhysics();
    FCore_BiomePhysicsParameters BlendPhysicsParameters(const FCore_BiomePhysicsParameters& ParamsA, const FCore_BiomePhysicsParameters& ParamsB, float BlendFactor) const;
    
    // Performance cache
    TArray<TWeakObjectPtr<AActor>> CachedNearbyActors;
    float LastCacheUpdate = 0.0f;
    float LastPhysicsUpdate = 0.0f;
};