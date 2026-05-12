#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_PhysicsTerrainIntegrator.generated.h"

/**
 * CORE SYSTEMS PROGRAMMER - CYCLE 008
 * Physics-Terrain Integration System
 * 
 * Bridges terrain generation with physics simulation for realistic ground interaction.
 * Handles dynamic terrain physics properties, deformation, and collision optimization.
 * 
 * Key Features:
 * - Real-time terrain physics property calculation based on biome and material
 * - Dynamic collision mesh generation for procedural terrain
 * - Terrain deformation physics for footprints, impacts, and erosion
 * - Optimized physics LOD system for large terrains
 * - Integration with Core physics systems and biome architecture
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    /** Surface friction coefficient (0.0 = ice, 1.0 = concrete) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float SurfaceFriction = 0.7f;

    /** Terrain hardness for deformation (0.0 = mud, 1.0 = rock) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainHardness = 0.5f;

    /** Bounce/restitution coefficient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Restitution = 0.1f;

    /** Density for physics calculations (kg/m³) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float Density = 1500.0f;

    /** Sound material type for footsteps and impacts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    ESurfaceType SoundMaterial = SurfaceType1;

    /** Whether this terrain can be deformed by impacts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bCanDeform = true;

    /** Minimum force required to deform terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation", meta = (ClampMin = "0.0"))
    float DeformationThreshold = 500.0f;

    FCore_TerrainPhysicsProperties()
    {
        SurfaceFriction = 0.7f;
        TerrainHardness = 0.5f;
        Restitution = 0.1f;
        Density = 1500.0f;
        SoundMaterial = SurfaceType1;
        bCanDeform = true;
        DeformationThreshold = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainCollisionData
{
    GENERATED_BODY()

    /** Collision mesh LOD level (0 = highest detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    int32 CollisionLOD = 0;

    /** Collision complexity setting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionTraceFlag> CollisionComplexity = CTF_UseDefault;

    /** Whether to use complex collision for physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bUseComplexAsSimple = false;

    /** Physics material override */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    class UPhysicalMaterial* PhysicsMaterial = nullptr;

    /** Collision response channels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionResponse> CollisionResponse = ECR_Block;

    FCore_TerrainCollisionData()
    {
        CollisionLOD = 0;
        CollisionComplexity = CTF_UseDefault;
        bUseComplexAsSimple = false;
        PhysicsMaterial = nullptr;
        CollisionResponse = ECR_Block;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Current terrain physics properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Properties")
    FCore_TerrainPhysicsProperties PhysicsProperties;

    /** Collision data configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FCore_TerrainCollisionData CollisionData;

    /** Whether to enable real-time physics updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableRealTimePhysics = true;

    /** Physics update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "120.0"))
    float PhysicsUpdateFrequency = 30.0f;

    /** Maximum distance for detailed physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0"))
    float MaxPhysicsDistance = 5000.0f;

    // Core Physics Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateTerrainPhysicsProperties(const FCore_TerrainPhysicsProperties& NewProperties);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyBiomePhysicsSettings(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegenerateCollisionMesh();

    UFUNCTION(BlueprintCallable, Category = "Deformation")
    void ApplyTerrainDeformation(const FVector& ImpactLocation, float Force, float Radius);

    UFUNCTION(BlueprintPure, Category = "Physics Query")
    FCore_TerrainPhysicsProperties GetPhysicsPropertiesAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsLODLevel(int32 LODLevel);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainDeformation, FVector, Location, float, Magnitude);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTerrainDeformation OnTerrainDeformed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsPropertiesChanged, FCore_TerrainPhysicsProperties, NewProperties);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsPropertiesChanged OnPhysicsPropertiesChanged;

private:
    /** Cached reference to landscape actor */
    UPROPERTY()
    class ALandscape* CachedLandscape;

    /** Physics update timer */
    float PhysicsUpdateTimer;

    /** Last known player location for LOD calculations */
    FVector LastPlayerLocation;

    // Internal Methods
    void UpdatePhysicsLOD();
    void CacheTerrainReferences();
    void ApplyPhysicsMaterialToTerrain();
    bool IsLocationInPhysicsRange(const FVector& Location) const;
    float CalculatePhysicsLODDistance(const FVector& Location) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsTerrainIntegrator : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsTerrainIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Primary terrain physics component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCore_TerrainPhysicsComponent* TerrainPhysicsComponent;

    /** Global terrain physics settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    FCore_TerrainPhysicsProperties GlobalPhysicsSettings;

    /** Biome-specific physics overrides */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EBiomeType, FCore_TerrainPhysicsProperties> BiomePhysicsOverrides;

    /** Whether to automatically detect and apply biome physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Automation")
    bool bAutoApplyBiomePhysics = true;

    /** Performance monitoring enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnablePerformanceMonitoring = false;

    /** Debug visualization enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugVisualization = false;

    // Core Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Integration")
    void RefreshAllTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void UpdatePhysicsForBiome(EBiomeType BiomeType, const FCore_TerrainPhysicsProperties& Properties);

    UFUNCTION(BlueprintPure, Category = "Query")
    FCore_TerrainPhysicsProperties GetEffectivePhysicsProperties(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsForPerformance();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowPhysicsProperties();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugVisualization();

    // Performance Metrics
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCurrentPhysicsPerformanceMetric() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetActivePhysicsActorCount() const;

private:
    /** Performance monitoring data */
    float LastFramePhysicsTime;
    int32 ActivePhysicsActors;
    
    /** Cached world reference */
    UPROPERTY()
    UWorld* CachedWorld;

    // Internal Methods
    void UpdatePerformanceMetrics();
    void ApplyGlobalPhysicsSettings();
    void RegisterWithPhysicsManager();
    void HandleBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome);
};