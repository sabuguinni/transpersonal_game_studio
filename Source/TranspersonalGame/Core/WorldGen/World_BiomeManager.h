#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swampland   UMETA(DisplayName = "Swampland"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Desert      UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeTint = FLinearColor::White;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxVegetationPerBiome = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float BiomeTransitionDistance = 1000.0f;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateSwamplandBiome(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateCanyonBiome(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void CreateForestBiome(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowBiomeBounds();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ClearAllBiomes();

private:
    void SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData);
    void SpawnWaterFeatures(const FWorld_BiomeData& BiomeData);
    void ApplyBiomeLighting(const FWorld_BiomeData& BiomeData);

    UPROPERTY()
    TArray<AActor*> SpawnedBiomeActors;
};