#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "SharedTypes.h"
#include "Crowd_PopulationManager.generated.h"

class UMassEntityConfigAsset;
class UMassSpawnerSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PopulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 MaxEntitiesPerBiome = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float DespawnRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float SpawnInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    bool bEnableSpawning = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    TArray<FMassEntityHandle> Entities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 GroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowd_BehaviorState GroupBehavior = ECrowd_BehaviorState::Idle;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PopulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PopulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    TArray<FCrowd_PopulationSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    int32 MaxTotalEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    float PopulationUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    bool bEnableGroupBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    int32 MaxGroupSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Settings")
    float GroupFormationRadius = 300.0f;

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void SpawnCrowdEntities(const FCrowd_PopulationSettings& Settings, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void DespawnDistantEntities();

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void UpdatePopulation();

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    int32 GetCurrentEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void FormGroups();

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void UpdateGroupBehavior();

    UFUNCTION(BlueprintCallable, Category = "Population Management")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FMassEntityHandle> ManagedEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FCrowd_EntityGroup> EntityGroups;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float LastPopulationUpdate = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 NextGroupID = 0;

private:
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    FVector GetPlayerLocation() const;
    bool IsLocationValid(const FVector& Location, const FCrowd_PopulationSettings& Settings) const;
    FVector GetRandomSpawnLocation(const FCrowd_PopulationSettings& Settings) const;
    void CleanupInvalidEntities();
    void AssignEntityToGroup(const FMassEntityHandle& Entity, const FVector& EntityLocation);
    FCrowd_EntityGroup* FindNearestGroup(const FVector& Location);
    void CreateNewGroup(const FMassEntityHandle& Entity, const FVector& Location);
    void UpdateGroupCenters();
};

#include "Crowd_PopulationManager.generated.h"