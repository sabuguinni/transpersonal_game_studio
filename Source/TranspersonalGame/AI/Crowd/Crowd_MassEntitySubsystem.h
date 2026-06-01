#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Position;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_NPCBehaviorState BehaviorState;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        BiomeType = ECrowd_BiomeType::Savanna;
        BehaviorState = ECrowd_NPCBehaviorState::Wandering;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxEntitiesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MinDistanceBetweenEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<ECrowd_BiomeType> AllowedBiomes;

    FCrowd_SpawnConfig()
    {
        MaxEntitiesPerBiome = 200;
        SpawnRadius = 15000.0f;
        MinDistanceBetweenEntities = 500.0f;
        AllowedBiomes = {ECrowd_BiomeType::Savanna, ECrowd_BiomeType::Forest, ECrowd_BiomeType::Desert, ECrowd_BiomeType::Swamp, ECrowd_BiomeType::Mountain};
    }
};

/**
 * Mass Entity subsystem for crowd simulation in prehistoric world
 * Manages up to 50,000 NPCs using UE5 Mass Entity framework
 * Distributes entities across 5 biomes with realistic population limits
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateEntityBehavior(FMassEntityHandle EntityHandle, ECrowd_NPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetEntityCountInBiome(ECrowd_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_EntityData> GetEntitiesInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetCrowdDensity(ECrowd_BiomeType BiomeType, float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void PauseSimulation(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllEntities();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FCrowd_SpawnConfig SpawnConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TMap<ECrowd_BiomeType, int32> BiomeEntityCounts;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TArray<FMassEntityHandle> ActiveEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float SimulationTime;

private:
    void UpdateEntityMovement(float DeltaTime);
    void ProcessBiomeDistribution();
    FVector GetBiomeCenter(ECrowd_BiomeType BiomeType) const;
    bool IsValidSpawnLocation(FVector Location, ECrowd_BiomeType BiomeType) const;
    void CleanupInvalidEntities();
};