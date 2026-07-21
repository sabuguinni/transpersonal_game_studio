#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    Swampland   UMETA(DisplayName = "Misty Swampland"),
    Canyon      UMETA(DisplayName = "Rocky Canyon"),
    Riverbank   UMETA(DisplayName = "River Valley"),
    Highlands   UMETA(DisplayName = "Mountain Highlands")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D CenterLocation = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        CenterLocation = FVector2D::ZeroVector;
        Radius = 5000.0f;
        VegetationDensity = 0.7f;
        TerrainRoughness = 0.5f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureType = TEXT("Rock");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType AssociatedBiome = EWorld_BiomeType::Forest;

    FWorld_TerrainFeature()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        FeatureType = TEXT("Rock");
        AssociatedBiome = EWorld_BiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorld_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorldBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiome(EWorld_BiomeType BiomeType, FVector2D Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    // Terrain Features
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetationClusters();

    // Performance-Aware Generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetLODBasedGeneration(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateGenerationBasedOnPerformance();

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "World Generation")
    void RegenerateAllBiomes();

    UFUNCTION(CallInEditor, Category = "World Generation")
    void ClearGeneratedContent();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ValidateWorldGeneration() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeData> ActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bLODBasedGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxGenerationDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxActiveFeatures = 500;

private:
    void InitializeBiomePresets();
    void CreateForestBiome(FVector2D Location, float Radius);
    void CreateSwamplandBiome(FVector2D Location, float Radius);
    void CreateCanyonBiome(FVector2D Location, float Radius);
    void CreatePlainsBiome(FVector2D Location, float Radius);
    void CreateRiverbankBiome(FVector2D Location, float Radius);
    void CreateHighlandsBiome(FVector2D Location, float Radius);
    
    float CalculateDistanceToPlayer(FVector Location) const;
    bool ShouldGenerateAtLocation(FVector Location) const;
    void CleanupDistantFeatures();

    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomePresets;
    TArray<AActor*> GeneratedActors;
    bool bIsInitialized = false;
};