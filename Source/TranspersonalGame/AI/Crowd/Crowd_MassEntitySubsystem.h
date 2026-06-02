#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_GroupBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsFlocking = true;
};

/**
 * Mass Entity subsystem for crowd simulation in prehistoric world
 * Manages up to 50,000 crowd entities using UE5 Mass Entity framework
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdEntities(const TArray<FCrowd_EntitySpawnData>& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void DespawnAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveCrowdEntityCount() const;

    // Group Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetGroupBehavior(const FCrowd_GroupBehavior& GroupBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateGroupTarget(int32 GroupID, const FVector& NewTarget);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetMaxCrowdEntities(int32 MaxEntities);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void EnableLODSystem(bool bEnable);

protected:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd", meta = (AllowPrivateAccess = "true"))
    int32 MaxCrowdEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd", meta = (AllowPrivateAccess = "true"))
    bool bLODSystemEnabled = true;

    UPROPERTY()
    TArray<FMassEntityHandle> ActiveCrowdEntities;

    UPROPERTY()
    TMap<int32, FCrowd_GroupBehavior> GroupBehaviors;

private:
    void InitializeMassEntity();
    void CleanupMassEntity();
    
    FMassEntityHandle CreateCrowdEntity(const FCrowd_EntitySpawnData& SpawnData);
    void UpdateCrowdEntities(float DeltaTime);
};