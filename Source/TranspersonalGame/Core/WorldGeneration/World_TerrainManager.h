#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_TerrainManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Rock        UMETA(DisplayName = "Rock Formation"),
    Tree        UMETA(DisplayName = "Tree"),
    Bush        UMETA(DisplayName = "Bush"),
    Boulder     UMETA(DisplayName = "Boulder"),
    Cliff       UMETA(DisplayName = "Cliff Face"),
    Valley      UMETA(DisplayName = "Valley"),
    Hill        UMETA(DisplayName = "Hill"),
    Transition  UMETA(DisplayName = "Biome Transition")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType = EWorld_TerrainType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TSoftObjectPtr<UStaticMesh> TerrainMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType AssociatedBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCollision = true;

    FWorld_TerrainFeature()
    {
        TerrainType = EWorld_TerrainType::Rock;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        AssociatedBiome = EBiomeType::Savana;
        LODDistance = 5000.0f;
        bEnableCollision = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<EWorld_TerrainType> AllowedTerrainTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 MinTerrainFeatures = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 MaxTerrainFeatures = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudget = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization = true;

    FWorld_BiomeTerrainConfig()
    {
        BiomeType = EBiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 5000.0f;
        MinTerrainFeatures = 20;
        MaxTerrainFeatures = 50;
        PerformanceBudget = 2.0f;
        bEnableLODOptimization = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain management
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateAllBiomeTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearBiomeTerrain(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void RegenerateTerrainFeature(const FWorld_TerrainFeature& Feature);

    // Terrain feature management
    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    AActor* SpawnTerrainFeature(const FWorld_TerrainFeature& Feature);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void RemoveTerrainFeature(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    TArray<AActor*> GetTerrainFeaturesInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    int32 GetTerrainFeatureCount(EBiomeType BiomeType);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTerrainPerformanceBudget(float BudgetMS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableTerrainCollisionOptimization(bool bEnable);

    // Biome transition management
    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    void CreateBiomeTransition(EBiomeType BiomeA, EBiomeType BiomeB, const FVector& TransitionCenter);

    UFUNCTION(BlueprintCallable, Category = "Biome Transitions")
    void UpdateBiomeTransitions();

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawBiomeBounds();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogTerrainStatistics();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<TSoftObjectPtr<UStaticMesh>> AvailableTerrainMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GlobalPerformanceBudget = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticLOD = true;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FWorld_TerrainFeature> ActiveTerrainFeatures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TMap<EBiomeType, TArray<AActor*>> BiomeTerrainActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float CurrentPerformanceUsage = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 TotalTerrainActors = 0;

private:
    // Internal methods
    void InitializeBiomeConfigurations();
    void LoadAvailableTerrainMeshes();
    FWorld_TerrainFeature GenerateRandomTerrainFeature(EBiomeType BiomeType, const FVector& BiomeCenter, float Radius);
    UStaticMesh* SelectTerrainMeshForType(EWorld_TerrainType TerrainType);
    void UpdateLODForActor(AActor* TerrainActor, float DistanceToViewer);
    void CleanupInvalidActors();

    // Performance tracking
    float LastLODUpdateTime = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameTimesamples = 0;
};