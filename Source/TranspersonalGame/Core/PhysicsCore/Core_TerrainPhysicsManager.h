#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Core_TerrainPhysicsManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - TERRAIN PHYSICS MANAGER
 * Core Systems Programmer Agent #03
 * 
 * Manages terrain-specific physics interactions including:
 * - Landscape collision configuration
 * - Terrain material properties (mud, rock, sand)
 * - Slope-based physics (sliding, rolling)
 * - Vegetation physics integration
 * - Terrain destruction and deformation
 * 
 * This system ensures realistic terrain physics for the prehistoric survival game.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    FString MaterialName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    float Restitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    float Density = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    bool bCanDeform = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Material")
    float DeformationThreshold = 1000.0f;

    FCore_TerrainMaterial()
    {
        MaterialName = TEXT("Generic");
        Friction = 0.7f;
        Restitution = 0.1f;
        Density = 1.5f;
        bCanDeform = false;
        DeformationThreshold = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SlopePhysics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float MaxWalkableAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float SlideThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float SlideFriction = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float RollThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    bool bEnableAvalanche = true;

    FCore_SlopePhysics()
    {
        MaxWalkableAngle = 45.0f;
        SlideThreshold = 60.0f;
        SlideFriction = 0.2f;
        RollThreshold = 75.0f;
        bEnableAvalanche = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ═══════════════════════════════════════════════════════════════
    // TERRAIN MATERIAL SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TArray<FCore_TerrainMaterial> TerrainMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    FCore_TerrainMaterial DefaultMaterial;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterTerrainMaterial(const FCore_TerrainMaterial& Material);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainMaterial GetTerrainMaterialAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyMaterialToLandscape(ALandscape* Landscape, const FCore_TerrainMaterial& Material);

    // ═══════════════════════════════════════════════════════════════
    // SLOPE PHYSICS SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    FCore_SlopePhysics SlopeSettings;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetSlopeAngleAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationWalkable(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplySlopeForces(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void TriggerAvalanche(const FVector& Location, float Radius);

    // ═══════════════════════════════════════════════════════════════
    // TERRAIN DEFORMATION SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    bool bEnableDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float DeformationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Deformation")
    float MaxDeformationDepth = 50.0f;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DeformTerrain(const FVector& Location, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateCrater(const FVector& Location, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RestoreTerrain(const FVector& Location, float Radius);

    // ═══════════════════════════════════════════════════════════════
    // VEGETATION PHYSICS INTEGRATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Physics")
    bool bEnableVegetationPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Physics")
    float VegetationWindStrength = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyWindToVegetation(const FVector& WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DestroyVegetationInRadius(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SpawnDebrisFromVegetation(const FVector& Location, int32 DebrisCount);

    // ═══════════════════════════════════════════════════════════════
    // COLLISION AND PHYSICS CONFIGURATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    EEng_PhysicsLayer TerrainPhysicsLayer = EEng_PhysicsLayer::Terrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Configuration")
    bool bEnableComplexCollision = true;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ConfigureLandscapeCollision(ALandscape* Landscape);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdatePhysicsProperties();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void EnablePhysicsSimulation(bool bEnable);

    // ═══════════════════════════════════════════════════════════════
    // SYSTEM MONITORING AND DEBUG
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawSlopeAngles = false;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DrawDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FString GetTerrainPhysicsReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

private:
    // Internal state tracking
    TArray<FVector> DeformedLocations;
    TMap<FVector, float> SlopeCache;
    float LastUpdateTime;

    // Helper functions
    FVector CalculateTerrainNormal(const FVector& Location);
    float CalculateSlopeFromNormal(const FVector& Normal);
    void ClearSlopeCache();
    void UpdateDeformationTracking();
};