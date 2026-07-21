#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "SharedTypes.h"
#include "Crowd_MassSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    ECrowd_EntityType EntityType = ECrowd_EntityType::Herbivore;

    FCrowd_SpawnConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        BiomeType = ECrowd_BiomeType::Savana;
        EntityType = ECrowd_EntityType::Herbivore;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    FCrowd_SpawnConfig SpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    TArray<FCrowd_SpawnConfig> BiomeConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Spawning")
    int32 CurrentEntityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Mass Spawning")
    bool bIsSpawning = false;

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void StartMassSpawning();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void StopMassSpawning();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void DistributeEntitiesAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning", CallInEditor = true)
    void InitializeBiomeConfigs();

private:
    void SpawnMassEntities(const FCrowd_SpawnConfig& Config, const FVector& CenterLocation);
    FVector GetBiomeCenterLocation(ECrowd_BiomeType BiomeType);
    void UpdateEntityCount();

    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    FMassEntityQuery EntityQuery;
    TArray<FMassEntityHandle> SpawnedEntities;

    float LastUpdateTime = 0.0f;
    const float UpdateInterval = 1.0f;
};