#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysicsSystem.generated.h"

/**
 * Terrain Physics System - Manages realistic physics interactions with terrain
 * Handles surface materials, friction, deformation, and environmental physics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN SURFACE PROPERTIES ===
    
    /** Surface material types with different physics properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<FString, float> SurfaceFrictionMap;
    
    /** Surface bounce/restitution values */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<FString, float> SurfaceBounciness;
    
    /** Surface density affecting sinking/deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<FString, float> SurfaceDensity;

    // === TERRAIN DEFORMATION ===
    
    /** Enable dynamic terrain deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    bool bEnableTerrainDeformation;
    
    /** Minimum force required to deform terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (ClampMin = "0.0"))
    float DeformationThreshold;
    
    /** Maximum deformation depth in centimeters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (ClampMin = "0.0"))
    float MaxDeformationDepth;
    
    /** Deformation recovery rate per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation", meta = (ClampMin = "0.0"))
    float DeformationRecoveryRate;

    // === ENVIRONMENTAL PHYSICS ===
    
    /** Wind force affecting objects on terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    FVector WindForce;
    
    /** Gravity modifier for different terrain types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    float TerrainGravityModifier;
    
    /** Enable mud/quicksand physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics")
    bool bEnableMudPhysics;
    
    /** Mud viscosity affecting movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Physics", meta = (ClampMin = "0.0"))
    float MudViscosity;

    // === COLLISION OPTIMIZATION ===
    
    /** Enable terrain collision LOD system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTerrainLOD;
    
    /** Distance for high-detail collision (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float HighDetailDistance;
    
    /** Distance for medium-detail collision (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MediumDetailDistance;
    
    /** Distance for low-detail collision (meters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float LowDetailDistance;

    // === BLUEPRINT INTERFACE ===
    
    /** Get surface friction at world location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetSurfaceFriction(const FVector& WorldLocation);
    
    /** Get surface material type at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FString GetSurfaceMaterial(const FVector& WorldLocation);
    
    /** Apply force to terrain (for deformation) */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ApplyTerrainForce(const FVector& WorldLocation, const FVector& Force, float Radius = 100.0f);
    
    /** Check if location is in mud/quicksand */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationInMud(const FVector& WorldLocation);
    
    /** Get terrain slope angle at location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainSlope(const FVector& WorldLocation);
    
    /** Initialize terrain physics for landscape */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics(ALandscape* TargetLandscape);

    // === DEBUG FUNCTIONS ===
    
    /** Draw terrain physics debug information */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DrawTerrainPhysicsDebug();
    
    /** Log terrain physics stats */
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogTerrainPhysicsStats();

private:
    // === INTERNAL SYSTEMS ===
    
    /** Reference to the main landscape */
    UPROPERTY()
    ALandscape* MainLandscape;
    
    /** Cached terrain deformation data */
    TMap<FIntPoint, float> DeformationMap;
    
    /** Performance monitoring */
    float LastPerformanceCheck;
    int32 PhysicsCalculationsPerSecond;
    
    // === INTERNAL METHODS ===
    
    /** Update terrain physics simulation */
    void UpdateTerrainPhysics(float DeltaTime);
    
    /** Process terrain deformation */
    void ProcessTerrainDeformation(float DeltaTime);
    
    /** Update collision LOD system */
    void UpdateTerrainLOD();
    
    /** Calculate surface properties at location */
    void CalculateSurfaceProperties(const FVector& Location, float& OutFriction, float& OutBounciness, FString& OutMaterial);
    
    /** Apply environmental forces */
    void ApplyEnvironmentalForces(float DeltaTime);
    
    /** Initialize default surface materials */
    void InitializeDefaultSurfaces();
    
    /** Validate terrain physics configuration */
    bool ValidateTerrainConfiguration();
};