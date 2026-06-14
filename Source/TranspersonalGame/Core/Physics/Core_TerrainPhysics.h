#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysics.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SurfaceFriction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Hardness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Stability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float DeformationResistance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float WaterAbsorption = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ETerrainType TerrainType = ETerrainType::Grass;

    FCore_TerrainPhysicsData()
    {
        SurfaceFriction = 0.7f;
        Hardness = 1.0f;
        Stability = 1.0f;
        DeformationResistance = 0.8f;
        WaterAbsorption = 0.3f;
        TerrainType = ETerrainType::Grass;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_FootprintData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Depth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Radius = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint")
    ECreatureSize CreatureSize = ECreatureSize::Medium;

    FCore_FootprintData()
    {
        Location = FVector::ZeroVector;
        Depth = 0.0f;
        Radius = 5.0f;
        Timestamp = 0.0f;
        CreatureSize = ECreatureSize::Medium;
    }
};

/**
 * Manages terrain physics interactions including surface properties, footprint tracking, and biome-based physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ETerrainType, FCore_TerrainPhysicsData> TerrainPhysicsMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    TArray<FCore_FootprintData> ActiveFootprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    int32 MaxFootprints = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    float FootprintLifetime = 300.0f;

    // Terrain Physics Methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsData GetTerrainPhysicsAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetTerrainPhysicsData(ETerrainType TerrainType, const FCore_TerrainPhysicsData& PhysicsData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateMovementSpeedModifier(const FVector& Location, ECreatureSize CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CanCreateFootprint(const FVector& Location, ECreatureSize CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateFootprint(const FVector& Location, ECreatureSize CreatureSize, float Force);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ClearOldFootprints();

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    TArray<FCore_FootprintData> GetFootprintsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ETerrainType DetectTerrainTypeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateTerrainSlope(const FVector& Location, float SampleRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationStable(const FVector& Location, float MinStability = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainDeformation(const FVector& Location, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysicsDefaults();

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void UpdatePhysicsFromBiome(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    float GetBiomeInfluencedFriction(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    float GetBiomeInfluencedHardness(const FVector& Location);

private:
    void CleanupFootprintArray();
    float GetTerrainHeightAtLocation(const FVector& Location);
    FVector GetTerrainNormalAtLocation(const FVector& Location);
};