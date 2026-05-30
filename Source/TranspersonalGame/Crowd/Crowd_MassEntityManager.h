#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ECrowd_EntityType EntityType = ECrowd_EntityType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 Count = 1;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    int32 MaxEntitiesPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    float SpawnRadius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_EntitySpawnData> BiomeSpawnData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    int32 TotalActiveEntities = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity", meta = (AllowPrivateAccess = "true"))
    TArray<FMassEntityHandle> ManagedEntities;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeBiomeSpawning();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnEntitiesInBiome(EBiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnMassEntitiesAcrossAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityPositions();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CleanupInvalidEntities();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mass Entity")
    void TestMassEntitySpawning();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    void InitializeMassSubsystems();
    FVector GetBiomeCenter(EBiomeType BiomeType) const;
    FMassEntityHandle SpawnSingleMassEntity(const FCrowd_EntitySpawnData& SpawnData);
};