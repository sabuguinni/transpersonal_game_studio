#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Crowd_MassEntitySpawner.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_SpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    int32 MaxEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float MinSpawnDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bSpawnOnBeginPlay = true;

    FCrowd_SpawnSettings()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        MinSpawnDistance = 100.0f;
        bSpawnOnBeginPlay = true;
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

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    FCrowd_SpawnSettings SpawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    UStaticMesh* CrowdMemberMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Spawning")
    UStaticMeshComponent* SpawnerMesh;

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SpawnCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning", CallInEditor)
    void TestSpawnSingleEntity();

protected:
    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    void InitializeMassSystem();
    FVector GetRandomSpawnLocation() const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    void CleanupInvalidEntities();

private:
    float LastSpawnTime;
    bool bMassSystemInitialized;
};