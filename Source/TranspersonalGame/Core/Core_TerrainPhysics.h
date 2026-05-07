#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Core_TerrainPhysics.generated.h"

/**
 * Core_TerrainPhysics - Advanced terrain physics system for prehistoric survival
 * Handles dynamic terrain deformation, erosion, landslides, and ground stability
 * Essential for realistic dinosaur footprints, cave-ins, and environmental destruction
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === TERRAIN DEFORMATION ===
    
    /** Enable dynamic terrain deformation (footprints, impacts) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainDeformation = true;
    
    /** Maximum deformation depth in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxDeformationDepth = 15.0f;
    
    /** Recovery rate for terrain deformation (cm per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float DeformationRecoveryRate = 2.0f;

    // === GROUND STABILITY ===
    
    /** Enable ground stability calculations for cave-ins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Stability")
    bool bEnableGroundStability = true;
    
    /** Slope angle threshold for landslides (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Stability", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float LandslideAngleThreshold = 45.0f;
    
    /** Weight threshold that triggers ground collapse (kg) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Stability", meta = (ClampMin = "0.0", ClampMax = "50000.0"))
    float CollapseWeightThreshold = 5000.0f;

    // === EROSION SYSTEM ===
    
    /** Enable water erosion simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bEnableWaterErosion = true;
    
    /** Erosion rate multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float ErosionRate = 1.0f;
    
    /** Minimum water flow required for erosion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MinWaterFlowForErosion = 10.0f;

public:
    // === TERRAIN INTERACTION METHODS ===
    
    /** Apply deformation at world location with given force and radius */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainDeformation(const FVector& WorldLocation, float Force, float Radius);
    
    /** Check ground stability at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsGroundStable(const FVector& WorldLocation, float CheckRadius = 100.0f);
    
    /** Trigger landslide at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void TriggerLandslide(const FVector& WorldLocation, float AffectedRadius = 500.0f);
    
    /** Calculate terrain hardness at location (affects deformation) */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainHardness(const FVector& WorldLocation);
    
    /** Apply water erosion to terrain area */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyWaterErosion(const FVector& WorldLocation, float WaterFlow, float DeltaTime);

    // === PHYSICS VALIDATION ===
    
    /** Validate terrain physics systems are working */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void ValidateTerrainPhysics();
    
    /** Reset all terrain deformations */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void ResetTerrainDeformations();

private:
    // === INTERNAL STATE ===
    
    /** Reference to the landscape actor */
    UPROPERTY()
    class ALandscape* LandscapeRef;
    
    /** Deformation data storage */
    TMap<FIntPoint, float> DeformationMap;
    
    /** Stability calculation cache */
    TMap<FIntPoint, float> StabilityCache;
    
    /** Last update time for erosion */
    float LastErosionUpdate = 0.0f;
    
    // === INTERNAL METHODS ===
    
    /** Initialize landscape reference */
    void InitializeLandscapeReference();
    
    /** Update deformation recovery */
    void UpdateDeformationRecovery(float DeltaTime);
    
    /** Calculate slope at location */
    float CalculateSlope(const FVector& WorldLocation);
    
    /** Get height at world location */
    float GetHeightAtLocation(const FVector& WorldLocation);
    
    /** Apply height modification to landscape */
    void ModifyLandscapeHeight(const FVector& WorldLocation, float HeightDelta, float Radius);
};