#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "SharedTypes.h"
#include "Core_TerrainDeformationSystem.generated.h"

/**
 * Core_TerrainDeformationSystem
 * 
 * Manages real-time terrain deformation for dinosaur footprints, impacts, and environmental changes.
 * Integrates with UE5 Landscape system for height-based deformation and material blending.
 * Provides performance-optimized deformation with LOD system for large-scale worlds.
 * 
 * Key Features:
 * - Dynamic height map modification for footprints and impacts
 * - Material blending for mud, sand, and rock surface changes
 * - Performance LOD system with distance-based detail reduction
 * - Integration with physics simulation for realistic deformation
 * - Temporal decay system for gradual terrain recovery
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainDeformationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainDeformationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN DEFORMATION INTERFACE ===
    
    /**
     * Creates a deformation at the specified world location
     * @param WorldLocation - World position for deformation
     * @param DeformationRadius - Radius of affected area in cm
     * @param DeformationDepth - Depth of deformation in cm (negative for depression)
     * @param DeformationType - Type of deformation (footprint, impact, erosion)
     * @param bPermanent - Whether deformation persists or decays over time
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void CreateTerrainDeformation(
        const FVector& WorldLocation,
        float DeformationRadius,
        float DeformationDepth,
        ECore_TerrainDeformationType DeformationType,
        bool bPermanent = false
    );

    /**
     * Creates dinosaur footprint with species-specific characteristics
     * @param FootprintLocation - World position of footprint
     * @param DinosaurSpecies - Species type for footprint shape and size
     * @param FootSize - Scale multiplier for footprint size
     * @param bDeepMud - Whether terrain is muddy (deeper impressions)
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void CreateDinosaurFootprint(
        const FVector& FootprintLocation,
        ECore_DinosaurSpecies DinosaurSpecies,
        float FootSize = 1.0f,
        bool bDeepMud = false
    );

    /**
     * Creates impact crater from falling objects or explosions
     * @param ImpactLocation - World position of impact
     * @param ImpactForce - Force of impact (affects crater size)
     * @param ImpactType - Type of impact (meteorite, tree fall, etc.)
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Deformation")
    void CreateImpactCrater(
        const FVector& ImpactLocation,
        float ImpactForce,
        ECore_ImpactType ImpactType
    );

    // === TERRAIN MATERIAL MODIFICATION ===
    
    /**
     * Modifies terrain materials at specified location
     * @param Location - World position for material change
     * @param Radius - Radius of affected area
     * @param NewMaterial - Target material type
     * @param BlendStrength - Strength of material blending (0-1)
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Materials")
    void ModifyTerrainMaterial(
        const FVector& Location,
        float Radius,
        ECore_TerrainMaterial NewMaterial,
        float BlendStrength = 1.0f
    );

    // === PERFORMANCE AND LOD SYSTEM ===
    
    /**
     * Sets LOD level for terrain deformation detail
     * @param LODLevel - Level of detail (0=highest, 3=lowest)
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDeformationLOD(int32 LODLevel);

    /**
     * Enables or disables deformation processing
     * @param bEnabled - Whether to process deformations
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDeformationEnabled(bool bEnabled);

    // === TERRAIN RECOVERY SYSTEM ===
    
    /**
     * Starts gradual recovery of terrain to original state
     * @param RecoveryLocation - Center of recovery area
     * @param RecoveryRadius - Radius of recovery area
     * @param RecoveryTime - Time in seconds for full recovery
     */
    UFUNCTION(BlueprintCallable, Category = "Terrain Recovery")
    void StartTerrainRecovery(
        const FVector& RecoveryLocation,
        float RecoveryRadius,
        float RecoveryTime = 300.0f
    );

    // === DEBUG AND VISUALIZATION ===
    
    /**
     * Enables debug visualization of deformation areas
     * @param bShowDebug - Whether to show debug visuals
     */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void SetDebugVisualization(bool bShowDebug);

    /**
     * Gets current deformation statistics
     * @return Structure containing performance and usage stats
     */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FCore_TerrainDeformationStats GetDeformationStats() const;

protected:
    // === CORE PROPERTIES ===
    
    /** Maximum number of active deformations */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxActiveDeformations = 100;

    /** Maximum deformation radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Limits", meta = (ClampMin = "50", ClampMax = "5000"))
    float MaxDeformationRadius = 1000.0f;

    /** Maximum deformation depth in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Limits", meta = (ClampMin = "1", ClampMax = "500"))
    float MaxDeformationDepth = 200.0f;

    /** Current LOD level for deformation detail */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0", ClampMax = "3"))
    int32 CurrentLODLevel = 1;

    /** Whether deformation system is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bDeformationEnabled = true;

    /** Whether to show debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bShowDebugVisualization = false;

    // === TERRAIN REFERENCES ===
    
    /** Reference to the main landscape actor */
    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    class ALandscape* MainLandscape;

    /** Cached landscape info for performance */
    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    class ULandscapeInfo* LandscapeInfo;

    // === DEFORMATION TRACKING ===
    
    /** Array of active deformations */
    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    TArray<FCore_TerrainDeformation> ActiveDeformations;

    /** Array of recovery operations */
    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    TArray<FCore_TerrainRecovery> ActiveRecoveries;

private:
    // === INTERNAL METHODS ===
    
    /** Initializes landscape references */
    void InitializeLandscapeReferences();

    /** Processes deformation on landscape heightmap */
    void ProcessLandscapeDeformation(const FCore_TerrainDeformation& Deformation);

    /** Processes material blending on landscape */
    void ProcessMaterialBlending(const FVector& Location, float Radius, ECore_TerrainMaterial Material, float Strength);

    /** Updates recovery operations */
    void UpdateRecoveryOperations(float DeltaTime);

    /** Cleans up expired deformations */
    void CleanupExpiredDeformations();

    /** Calculates deformation shape based on type and parameters */
    TArray<FVector2D> CalculateDeformationShape(ECore_TerrainDeformationType Type, float Radius);

    /** Gets landscape heightmap data for modification */
    bool GetLandscapeHeightData(const FVector& WorldLocation, float Radius, TArray<uint16>& HeightData, FIntRect& DataRect);

    /** Sets landscape heightmap data after modification */
    bool SetLandscapeHeightData(const FIntRect& DataRect, const TArray<uint16>& HeightData);

    // === PERFORMANCE TRACKING ===
    
    /** Current frame deformation count */
    int32 CurrentFrameDeformations = 0;

    /** Total deformations processed */
    int32 TotalDeformationsProcessed = 0;

    /** Last performance check time */
    float LastPerformanceCheckTime = 0.0f;
};