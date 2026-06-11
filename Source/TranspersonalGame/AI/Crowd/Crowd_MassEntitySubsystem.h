#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float LODDistance;

    FCrowd_EntityData()
    {
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        BehaviorState = ECrowd_BehaviorState::Idle;
        EntityID = 0;
        LODDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    FCrowd_LODConfig()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1500.0f;
        LowDetailDistance = 3000.0f;
        CullingDistance = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 SpawnCrowdEntity(const FVector& Location, ECrowd_BehaviorState InitialState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdEntity(int32 EntityID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdEntities(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetEntityBehaviorState(int32 EntityID, ECrowd_BehaviorState NewState);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateEntityLOD(int32 EntityID, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    ECrowd_LODLevel GetEntityLODLevel(int32 EntityID, const FVector& ViewerLocation);

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector GetNextPathPoint(int32 EntityID, const FVector& CurrentLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetEntityDestination(int32 EntityID, const FVector& Destination);

    // Crowd Behavior
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ProcessCrowdBehavior(int32 EntityID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void HandleCrowdInteractions(int32 EntityID);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxCrowdEntities(int32 MaxEntities);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODConfiguration(const FCrowd_LODConfig& NewLODConfig);

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_EntityData> GetNearbyEntities(const FVector& Location, float Radius);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Configuration")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Configuration")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Configuration")
    FCrowd_LODConfig LODConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd Configuration")
    float CrowdDensityPerSquareMeter;

private:
    TMap<int32, FCrowd_EntityData> ActiveEntities;
    int32 NextEntityID;
    float LastUpdateTime;

    // Internal helper functions
    void InitializeMassEntitySystem();
    void CleanupMassEntitySystem();
    FVector CalculateSteeringForce(const FCrowd_EntityData& Entity);
    void ApplyLODOptimizations(int32 EntityID, ECrowd_LODLevel LODLevel);
};