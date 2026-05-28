#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Core_TerrainPhysicsManager.generated.h"

/**
 * Core_TerrainPhysicsManager
 * 
 * Manages terrain-specific physics properties across the 5 biomes.
 * Integrates with BiomeManager to apply different physics behaviors
 * based on terrain type (mud, sand, rock, grass, snow).
 * 
 * Key Features:
 * - Biome-specific mass modifiers
 * - Surface friction coefficients  
 * - Terrain deformation simulation
 * - Dynamic physics material assignment
 * - Performance-optimized collision detection
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_TerrainPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === TERRAIN PHYSICS PROPERTIES ===
    
    /** Base mass multipliers for each biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeMassMultipliers;
    
    /** Surface friction coefficients per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeFrictionCoefficients;
    
    /** Bounce/restitution values per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeRestitutionValues;
    
    /** Enable terrain deformation simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainDeformation = true;
    
    /** Maximum deformation depth in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxDeformationDepth = 25.0f;

    // === PHYSICS MATERIAL MANAGEMENT ===
    
    /** Physics materials for each biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TMap<ECore_BiomeType, class UPhysicalMaterial*> BiomePhysicsMaterials;
    
    /** Default physics material fallback */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    class UPhysicalMaterial* DefaultPhysicsMaterial;

    // === CORE PHYSICS FUNCTIONS ===
    
    /**
     * Apply biome-specific physics properties to a static mesh component
     * @param MeshComponent - Component to modify
     * @param BiomeType - Target biome type
     * @param BaseMass - Original mass value
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyBiomePhysicsProperties(UStaticMeshComponent* MeshComponent, ECore_BiomeType BiomeType, float BaseMass = 100.0f);
    
    /**
     * Get the appropriate physics material for a biome
     * @param BiomeType - Target biome
     * @return Physics material to use
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Terrain Physics")
    class UPhysicalMaterial* GetBiomePhysicsMaterial(ECore_BiomeType BiomeType) const;
    
    /**
     * Calculate terrain-modified mass for an object
     * @param OriginalMass - Base mass value
     * @param BiomeType - Current biome
     * @return Modified mass value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Terrain Physics")
    float CalculateTerrainModifiedMass(float OriginalMass, ECore_BiomeType BiomeType) const;
    
    /**
     * Check if terrain deformation should occur at location
     * @param Location - World location to check
     * @param ImpactForce - Force of impact
     * @return True if deformation should occur
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ShouldDeformTerrain(const FVector& Location, float ImpactForce) const;

    // === PERFORMANCE OPTIMIZATION ===
    
    /** Maximum number of physics objects to process per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjectsPerFrame = 50;
    
    /** Update frequency for physics property checks (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateFrequency = 0.1f;

    // === DESTRUCTION PHYSICS ===
    
    /**
     * Handle object destruction with biome-specific effects
     * @param DestroyedActor - Actor being destroyed
     * @param ImpactLocation - Location of destruction
     * @param ImpactForce - Force causing destruction
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void HandleObjectDestruction(AActor* DestroyedActor, const FVector& ImpactLocation, float ImpactForce);
    
    /**
     * Create debris fragments with proper physics
     * @param OriginalActor - Source actor
     * @param FragmentCount - Number of fragments to create
     * @param ExplosionForce - Force to apply to fragments
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void CreateDestructionDebris(AActor* OriginalActor, int32 FragmentCount, float ExplosionForce);

    // === RAGDOLL PHYSICS ===
    
    /**
     * Apply ragdoll physics with biome-specific modifications
     * @param SkeletalMeshComp - Skeletal mesh to ragdoll
     * @param ImpactVelocity - Initial impact velocity
     * @param BiomeType - Current biome for physics modification
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyBiomeRagdollPhysics(class USkeletalMeshComponent* SkeletalMeshComp, const FVector& ImpactVelocity, ECore_BiomeType BiomeType);

protected:
    // === INTERNAL SYSTEMS ===
    
    /** Initialize biome physics properties */
    void InitializeBiomePhysicsProperties();
    
    /** Update physics properties for tracked objects */
    void UpdateTrackedPhysicsObjects();
    
    /** Timer for physics updates */
    FTimerHandle PhysicsUpdateTimer;
    
    /** List of actors currently being tracked for physics updates */
    UPROPERTY()
    TArray<AActor*> TrackedPhysicsActors;
    
    /** Performance monitoring */
    float LastUpdateTime = 0.0f;
    int32 PhysicsObjectsProcessedThisFrame = 0;

public:
    // === EDITOR FUNCTIONS ===
    
    /** Debug function to visualize physics properties */
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugVisualizeBiomePhysics();
    
    /** Validate all physics materials are assigned */
    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidatePhysicsMaterials();
};