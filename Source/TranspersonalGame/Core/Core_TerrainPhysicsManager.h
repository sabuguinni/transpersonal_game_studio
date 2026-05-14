#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Core_TerrainPhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSupportsFootprints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DeformationResistance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_TerrainType TerrainType = ECore_TerrainType::Dirt;

    FCore_TerrainPhysicsData()
    {
        Friction = 0.7f;
        Restitution = 0.1f;
        Density = 1.0f;
        bSupportsFootprints = false;
        DeformationResistance = 1.0f;
        TerrainType = ECore_TerrainType::Dirt;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_FootprintData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Depth = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Radius = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    ECore_TerrainType TerrainType = ECore_TerrainType::Dirt;

    FCore_FootprintData()
    {
        Location = FVector::ZeroVector;
        Depth = 2.0f;
        Radius = 15.0f;
        TimeStamp = 0.0f;
        TerrainType = ECore_TerrainType::Dirt;
    }
};

/**
 * Core Terrain Physics Manager - Manages physics interactions with terrain in the prehistoric world
 * Handles terrain deformation, footprints, material properties, and environmental physics
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

    // Terrain Physics Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_TerrainType, FCore_TerrainPhysicsData> TerrainPhysicsMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_FootprintData> ActiveFootprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float MaxFootprints = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    float FootprintLifetime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnableFootprints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxPhysicsDistance = 5000.0f;

    // Physics Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_TerrainType, TSoftObjectPtr<UPhysicalMaterial>> PhysicsMaterials;

    // Cached References
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    ALandscape* LandscapeRef;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TArray<AStaticMeshActor*> TerrainMeshActors;

    // Internal State
    float LastPhysicsUpdate = 0.0f;
    bool bIsInitialized = false;

public:
    // Core Terrain Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsData GetTerrainPhysicsData(ECore_TerrainType TerrainType) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainPhysicsData(ECore_TerrainType TerrainType, const FCore_TerrainPhysicsData& PhysicsData);

    // Footprint System
    UFUNCTION(BlueprintCallable, Category = "Footprints")
    void CreateFootprint(const FVector& Location, float Depth, float Radius, ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Footprints")
    void RemoveOldFootprints(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "Footprints")
    void ClearAllFootprints();

    // Terrain Deformation
    UFUNCTION(BlueprintCallable, Category = "Deformation")
    void DeformTerrainAtLocation(const FVector& Location, float Radius, float Intensity, bool bRaise = false);

    UFUNCTION(BlueprintCallable, Category = "Deformation")
    void CreateImpactCrater(const FVector& Location, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Deformation")
    void CreateExplosionDeformation(const FVector& Location, float Radius, float Force);

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void ApplyPhysicsMaterialToTerrain(ECore_TerrainType TerrainType, UPhysicalMaterial* PhysicsMaterial);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    UPhysicalMaterial* GetPhysicsMaterialForTerrain(ECore_TerrainType TerrainType) const;

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void RefreshTerrainPhysicsMaterials();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void RegisterTerrainMeshActor(AStaticMeshActor* MeshActor);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void UnregisterTerrainMeshActor(AStaticMeshActor* MeshActor);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation) const;

    // Performance and Debug
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugFootprints() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogTerrainPhysicsStats() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void EditorRefreshTerrainPhysics();

private:
    void InitializeDefaultTerrainPhysics();
    void CacheTerrainReferences();
    void UpdateFootprintSystem(float DeltaTime);
    void OptimizePhysicsPerformance();
    bool IsLocationWithinPhysicsRange(const FVector& Location) const;
};