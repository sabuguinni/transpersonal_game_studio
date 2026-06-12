#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/Landscape.h"
#include "Core/SharedTypes.h"
#include "Core_TerrainPhysicsManager.generated.h"

class ALandscape;
class ULandscapeComponent;
class UPhysicalMaterial;

/**
 * Core_TerrainPhysicsManager
 * 
 * Manages physics interactions with terrain surfaces including:
 * - Dynamic physics material assignment based on terrain type
 * - Terrain collision optimization for large landscapes
 * - Surface friction and bounce parameters
 * - Footstep sound and particle effect triggers
 * - Terrain deformation physics simulation
 * 
 * Integrates with UE5 Landscape system and Chaos Physics for realistic
 * terrain interaction in the prehistoric survival environment.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_TerrainPhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // CORE COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Components")
    USceneComponent* RootSceneComponent;

    // TERRAIN PHYSICS SETTINGS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float TerrainFrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainBounciness = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float TerrainDensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float DeformationStrength = 10.0f;

    // PHYSICS MATERIALS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    UPhysicalMaterial* GrassPhysicsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    UPhysicalMaterial* DirtPhysicsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    UPhysicalMaterial* RockPhysicsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    UPhysicalMaterial* MudPhysicsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    UPhysicalMaterial* SandPhysicsMaterial;

    // TERRAIN REFERENCES
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain References")
    ALandscape* ManagedLandscape;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain References")
    TArray<ULandscapeComponent*> LandscapeComponents;

    // OPTIMIZATION SETTINGS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float PhysicsUpdateRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float PhysicsUpdateFrequency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "1", ClampMax = "4"))
    int32 PhysicsLODLevels = 3;

public:
    // TERRAIN PHYSICS INTERFACE
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    UPhysicalMaterial* GetPhysicsMaterialAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainHardnessAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ApplyTerrainDeformation(const FVector& Location, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainPhysicsLOD(int32 LODLevel);

    // TERRAIN ANALYSIS
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetTerrainSlopeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    bool IsLocationWalkable(const FVector& WorldLocation, float MaxSlope = 45.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation);

    // OPTIMIZATION METHODS
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizePhysicsForDistance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnablePhysicsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void DisablePhysicsInRadius(const FVector& Center, float Radius);

protected:
    // INTERNAL METHODS
    void CacheLandscapeComponents();
    void UpdatePhysicsMaterials();
    void ProcessTerrainDeformation();
    void UpdatePhysicsLOD(const FVector& ViewerLocation);
    
    // TERRAIN SAMPLING
    float SampleTerrainHeight(const FVector& Location);
    FVector SampleTerrainNormal(const FVector& Location);
    ECore_TerrainType SampleTerrainType(const FVector& Location);

private:
    // INTERNAL STATE
    float LastPhysicsUpdate = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;
    TMap<ULandscapeComponent*, int32> ComponentLODLevels;
    
    // PERFORMANCE TRACKING
    int32 ActivePhysicsComponents = 0;
    float AveragePhysicsUpdateTime = 0.0f;
};