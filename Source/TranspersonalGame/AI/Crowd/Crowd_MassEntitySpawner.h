#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "Crowd_MassEntitySpawner.generated.h"

UENUM(BlueprintType)
enum class ECrowd_SpawnerType : uint8
{
    Villagers,
    Hunters,
    Traders,
    Guards,
    Animals
};

USTRUCT(BlueprintType)
struct FCrowd_SpawnConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    ECrowd_SpawnerType SpawnerType = ECrowd_SpawnerType::Villagers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    int32 MaxEntities = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    bool bUseNavMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
    float MinDistanceBetweenEntities = 100.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntitySpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    FCrowd_SpawnConfiguration SpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    TObjectPtr<UMassEntityConfigAsset> EntityConfigAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bRespawnDeadEntities = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float RespawnDelay = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SpawnEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void DestroyAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SetSpawnConfiguration(const FCrowd_SpawnConfiguration& NewConfig);

private:
    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    float LastRespawnTime = 0.0f;

    void InitializeMassSystem();
    bool IsValidSpawnLocation(const FVector& Location) const;
    FVector GetRandomSpawnLocation() const;
    void CleanupDeadEntities();
};