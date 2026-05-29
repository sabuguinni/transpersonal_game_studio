#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Crowd_MassEntitySpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float MinSpawnDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    bool bUseRandomHeight = true;

    FCrowd_SpawnConfig()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        SpawnCenter = FVector::ZeroVector;
        MinSpawnDistance = 100.0f;
        bUseRandomHeight = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FCrowd_SpawnConfig SpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> CrowdMeshes;

    FCrowd_BiomeSpawnData()
    {
        BiomeName = TEXT("DefaultBiome");
        BiomeCenter = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntitySpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    TArray<FCrowd_BiomeSpawnData> BiomeSpawnData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    int32 MaxTotalEntities = 5000;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Spawning")
    int32 CurrentEntityCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnEntitiesInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnEntitiesInAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void ClearAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SetBiomeSpawnConfig(const FString& BiomeName, const FCrowd_SpawnConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    int32 GetEntityCountInBiome(const FString& BiomeName) const;

protected:
    UFUNCTION()
    void InitializeBiomes();

    UFUNCTION()
    void SpawnEntitiesAtLocation(const FVector& Location, int32 Count, const TArray<TSoftObjectPtr<UStaticMesh>>& Meshes);

    UFUNCTION()
    FVector GetRandomSpawnLocation(const FCrowd_SpawnConfig& Config) const;

    UFUNCTION()
    bool IsValidSpawnLocation(const FVector& Location) const;

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    TMap<FString, TArray<FMassEntityHandle>> BiomeEntityMap;

    float LastUpdateTime = 0.0f;
    bool bIsInitialized = false;
};