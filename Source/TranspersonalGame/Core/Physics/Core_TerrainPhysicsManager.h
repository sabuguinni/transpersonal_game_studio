#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/CollisionProfile.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysicsManager.generated.h"

/**
 * CYCLE 009 - CORE SYSTEMS TERRAIN PHYSICS MANAGER
 * Manages terrain-specific physics interactions, surface materials, and collision optimization
 * Integrates with biome system for realistic surface physics (mud, rock, sand, grass)
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainSurfaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    EBiomeType BiomeType = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float Restitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    bool bCanDeform = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float DeformationThreshold = 100.0f;

    FCore_TerrainSurfaceData()
    {
        BiomeType = EBiomeType::Plains;
        Friction = 0.7f;
        Restitution = 0.1f;
        Density = 1.0f;
        bCanDeform = false;
        DeformationThreshold = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainCollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<FVector> CollisionVertices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<int32> CollisionIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FBox BoundingBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionComplexity = 1.0f;

    FCore_TerrainCollisionData()
    {
        BoundingBox = FBox(ForceInit);
        CollisionComplexity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN SURFACE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateSurfaceProperties(EBiomeType BiomeType, const FCore_TerrainSurfaceData& SurfaceData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainSurfaceData GetSurfaceDataAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplySurfaceEffectsToActor(AActor* Actor, const FVector& ContactPoint);

    // === COLLISION OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void OptimizeTerrainCollision(ALandscape* Landscape);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void GenerateCollisionMesh(const FCore_TerrainCollisionData& CollisionData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateCollisionComplexity(float NewComplexity);

    // === DEFORMATION SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainDeformation(const FVector& Location, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanDeformAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ResetDeformation(const FVector& Location, float Radius);

    // === PHYSICS INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterTerrainActor(AActor* TerrainActor, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UnregisterTerrainActor(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysicsSettings();

protected:
    // === SURFACE PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FCore_TerrainSurfaceData> BiomeSurfaceData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    float GlobalFrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    float GlobalRestitutionMultiplier = 1.0f;

    // === COLLISION SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    float CollisionUpdateDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    int32 MaxCollisionVertices = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    bool bEnableComplexCollision = true;

    // === DEFORMATION SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    bool bEnableTerrainDeformation = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    float MaxDeformationDepth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deformation", meta = (AllowPrivateAccess = "true"))
    float DeformationRecoveryRate = 1.0f;

    // === RUNTIME DATA ===
    UPROPERTY()
    TArray<AActor*> RegisteredTerrainActors;

    UPROPERTY()
    TMap<FVector, float> DeformationMap;

    UPROPERTY()
    ALandscape* CachedLandscape;

private:
    void InitializeBiomeSurfaceData();
    void UpdateActorPhysicsProperties(AActor* Actor, const FCore_TerrainSurfaceData& SurfaceData);
    FCore_TerrainSurfaceData GetDefaultSurfaceData(EBiomeType BiomeType);
    void ProcessDeformationRecovery(float DeltaTime);
    bool IsValidTerrainLocation(const FVector& Location);
};