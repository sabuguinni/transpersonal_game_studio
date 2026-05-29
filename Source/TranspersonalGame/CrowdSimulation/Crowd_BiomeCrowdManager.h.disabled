#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_BiomeCrowdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeDensityConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Density")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Density")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Density")
    int32 SpeciesVariety = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Density")
    float DensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Density")
    bool bEnableDynamicSpawning = true;

    FCrowd_BiomeDensityConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        SpeciesVariety = 3;
        DensityMultiplier = 1.0f;
        bEnableDynamicSpawning = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FString> HerbivoreSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FString> CarnivoreSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FString> OmnivoreSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float HerbivoreRatio = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float CarnivoreRatio = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float OmnivoreRatio = 0.1f;

    FCrowd_BiomeSpeciesConfig()
    {
        HerbivoreRatio = 0.7f;
        CarnivoreRatio = 0.2f;
        OmnivoreRatio = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_BiomeCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_BiomeCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    ECrowd_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    FCrowd_BiomeDensityConfig DensityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Configuration")
    FCrowd_BiomeSpeciesConfig SpeciesConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    bool bEnablePredatorAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    bool bEnableHerdBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f;

private:
    UPROPERTY()
    TArray<FVector> SpawnPoints;

    UPROPERTY()
    int32 CurrentEntityCount;

    UPROPERTY()
    float LastSpawnTime;

    UPROPERTY()
    bool bIsInitialized;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeBiomeCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdEntities(int32 EntityCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdDensity(float NewDensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetBiomeType(ECrowd_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetCurrentEntityCount() const { return CurrentEntityCount; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void EnableDynamicSpawning(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetLODDistances(float LOD1, float LOD2, float Culling);

protected:
    void GenerateSpawnPoints();
    void UpdateCrowdBehavior(float DeltaTime);
    void ManageLODSystem();
    void HandlePredatorInteractions();
    void UpdateHerdFormations();

    FVector GetRandomSpawnPoint() const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    void CleanupInvalidEntities();
};