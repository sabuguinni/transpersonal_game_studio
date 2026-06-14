#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientTint = FLinearColor::White;

    FWorld_BiomeData()
    {
        BiomeType = EBiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        VegetationDensity = 0.5f;
        WaterLevel = 0.0f;
        RockDensity = 0.3f;
        AmbientTint = FLinearColor::White;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateSwamplandBiome(FVector CenterLocation, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateCanyonBiome(FVector CenterLocation, float Radius = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateForestBiome(FVector CenterLocation, float Radius = 1200.0f);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateDesertBiome(FVector CenterLocation, float Radius = 1500.0f);

    // Biome Query
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FWorld_BiomeData> GetAllBiomes() const { return ActiveBiomes; }

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainForBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateWaterBodies(const FWorld_BiomeData& BiomeData);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void OptimizeBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetTotalBiomeActors() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FWorld_BiomeData> ActiveBiomes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biomes")
    TMap<EBiomeType, int32> BiomeActorCounts;

private:
    void SpawnBiomeActor(FVector Location, FVector Scale, const FString& Label);
    float CalculateDistanceFromBiomeCenter(FVector Location, const FWorld_BiomeData& BiomeData) const;
    bool IsLocationInBiome(FVector Location, const FWorld_BiomeData& BiomeData) const;
};

#include "World_BiomeManager.generated.h"