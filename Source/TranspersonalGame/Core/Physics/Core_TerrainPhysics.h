#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysics.generated.h"

/**
 * Terrain Physics System for Prehistoric Survival Game
 * Manages dynamic terrain physics including:
 * - Surface material detection and response
 * - Slope stability and landslide simulation
 * - Erosion and weathering effects
 * - Footprint and track generation
 * - Ground deformation from heavy impacts
 * - Terrain-based movement modifiers
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainSurface
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Hardness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Moisture = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bCanDeform = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bCanErode = true;

    FCore_TerrainSurface()
    {
        SurfaceType = ECore_SurfaceType::Dirt;
        Friction = 0.7f;
        Hardness = 0.5f;
        Moisture = 0.3f;
        Temperature = 20.0f;
        bCanDeform = true;
        bCanErode = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_FootprintData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Depth = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Width = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Length = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float CreationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    ECore_CreatureType CreatureType = ECore_CreatureType::Human;

    FCore_FootprintData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Depth = 2.0f;
        Width = 25.0f;
        Length = 30.0f;
        CreationTime = 0.0f;
        CreatureType = ECore_CreatureType::Human;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainDeformation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float ImpactForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float DeformationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float MaxDepth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    float RecoveryTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deformation")
    bool bPermanent = false;

    FCore_TerrainDeformation()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactForce = 1000.0f;
        DeformationRadius = 100.0f;
        MaxDepth = 50.0f;
        RecoveryTime = 30.0f;
        bPermanent = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN SURFACE ANALYSIS ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainSurface AnalyzeSurfaceAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetSlopeAngleAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector GetSurfaceNormalAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationStable(const FVector& Location, float ObjectMass = 100.0f);

    // === FOOTPRINT SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateFootprint(const FVector& Location, const FRotator& Rotation, ECore_CreatureType CreatureType, float Force = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    TArray<FCore_FootprintData> GetFootprintsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ClearOldFootprints(float MaxAge = 300.0f);

    // === TERRAIN DEFORMATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainDeformation(const FCore_TerrainDeformation& DeformationData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateImpactCrater(const FVector& ImpactLocation, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ProcessTerrainErosion(float DeltaTime);

    // === MOVEMENT MODIFIERS ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetMovementSpeedModifier(const FVector& Location, ECore_MovementType MovementType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetStaminaCostModifier(const FVector& Location, ECore_MovementType MovementType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanPlaceStructureAtLocation(const FVector& Location, float StructureWeight = 1000.0f);

    // === PHYSICS INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdatePhysicsMaterialAtLocation(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyEnvironmentalForces(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ValidateTerrainPhysicsIntegrity();

protected:
    // === TERRAIN DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_TerrainSurface> TerrainSurfaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_FootprintData> ActiveFootprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_TerrainDeformation> ActiveDeformations;

    // === PHYSICS SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float MaxFootprintDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float FootprintLifetime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float DeformationThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    float ErosionRate = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableFootprints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableErosion = false;

    // === PERFORMANCE OPTIMIZATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxFootprints = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxDeformations = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PhysicsLODDistance = 2000.0f;

private:
    // === INTERNAL SYSTEMS ===
    
    float LastUpdateTime;
    
    ALandscape* CachedLandscape;
    
    TMap<FVector, float> SurfaceHardnessCache;
    
    TMap<FVector, ECore_SurfaceType> SurfaceTypeCache;

    // === HELPER FUNCTIONS ===
    
    void InitializeTerrainPhysics();
    
    void UpdateFootprintSystem(float DeltaTime);
    
    void UpdateDeformationSystem(float DeltaTime);
    
    void CacheTerrainData();
    
    float CalculateImpactForce(const FVector& Velocity, float Mass);
    
    ECore_SurfaceType DetermineSurfaceType(const FVector& Location);
    
    void ApplyPhysicsMaterialToLocation(const FVector& Location, float Radius, ECore_SurfaceType SurfaceType);
};