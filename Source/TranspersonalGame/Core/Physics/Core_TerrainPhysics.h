#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Chaos/ChaosSolverActor.h"
#include "Core_TerrainPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water"),
    Lava        UMETA(DisplayName = "Lava"),
    Ice         UMETA(DisplayName = "Ice")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Hardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanDeform = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bIsSlippery = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCausesFootprints = false;

    FCore_TerrainPhysicsProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        Hardness = 1.0f;
        Temperature = 20.0f;
        bCanDeform = false;
        bIsSlippery = false;
        bCausesFootprints = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    AActor* ImpactingActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float Timestamp = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_TerrainPhysics : public AActor
{
    GENERATED_BODY()

public:
    ACore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, FCore_TerrainPhysicsProperties> TerrainProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float TerrainUpdateRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float MaxImpactForce = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainDeformation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableFootprints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTemperatureEffects = true;

    // Terrain analysis functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainPropertiesAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainSlopeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector GetTerrainNormalAtLocation(const FVector& Location);

    // Terrain interaction functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainImpact(const FCore_TerrainImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateFootprint(const FVector& Location, float Size, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DeformTerrain(const FVector& Location, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationWalkable(const FVector& Location);

    // Physics material functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdatePhysicsMaterialForTerrain(ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainEffectsToActor(AActor* Actor, const FVector& Location);

    // Temperature and environmental effects
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTemperatureAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainTemperature(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ProcessEnvironmentalEffects(float DeltaTime);

    // Validation and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Physics")
    void ValidateTerrainPhysics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Physics")
    void TestTerrainInteraction();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Physics")
    void DebugDrawTerrainInfo();

protected:
    // Internal terrain tracking
    UPROPERTY()
    TArray<FCore_TerrainImpactData> RecentImpacts;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    TMap<FVector, float> TemperatureMap;

    UPROPERTY()
    TArray<ALandscape*> TrackedLandscapes;

    // Internal helper functions
    void InitializeTerrainProperties();
    void UpdateTrackedLandscapes();
    void ProcessTerrainImpacts(float DeltaTime);
    void CleanupOldImpacts();
    ECore_TerrainType SampleTerrainTypeFromLandscape(const FVector& Location);
    float CalculateTerrainHardness(ECore_TerrainType TerrainType, float Temperature);
};