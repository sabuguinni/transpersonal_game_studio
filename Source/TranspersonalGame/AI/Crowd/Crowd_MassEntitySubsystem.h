#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = -1;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ECrowd_BehaviorState> AllowedBehaviors;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FCrowd_SpawnConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDestination(FVector NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllEntities();

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    FCrowd_SpawnConfig DefaultSpawnConfig;

    UPROPERTY()
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY()
    FVector CurrentDestination = FVector::ZeroVector;

    void InitializeMassEntity();
    void ProcessMovement(float DeltaTime);
    void ProcessBehaviorStates(float DeltaTime);
    FVector CalculateFlockingForce(const FMassEntityHandle& Entity, const FVector& CurrentLocation);
};