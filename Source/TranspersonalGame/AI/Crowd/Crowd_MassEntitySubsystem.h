#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float LODDistance;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        MovementSpeed = 100.0f;
        BehaviorState = ECrowd_BehaviorState::Idle;
        EntityID = -1;
        LODDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    TArray<FCrowd_EntityData> ActiveEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    ECrowd_GroupType GroupType;

    FCrowd_SpawnGroup()
    {
        GroupCenter = FVector::ZeroVector;
        GroupRadius = 500.0f;
        MaxEntities = 20;
        GroupType = ECrowd_GroupType::Gathering;
    }
};

/**
 * Mass Entity Subsystem for managing large-scale crowd simulation
 * Handles up to 50,000 crowd entities with LOD and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdGroup(const FCrowd_SpawnGroup& GroupConfig);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(float NewDensity);

    // Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Entities")
    int32 CreateCrowdEntity(const FVector& SpawnLocation, const FRotator& SpawnRotation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Entities")
    void RemoveCrowdEntity(int32 EntityID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Entities")
    void UpdateEntityBehavior(int32 EntityID, ECrowd_BehaviorState NewState);

    // LOD System
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateCrowdLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void SetLODDistances(const TArray<float>& NewLODDistances);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    float GetCrowdSimulationFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetMaxCrowdEntities(int32 NewMaxEntities);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void TriggerCrowdReaction(const FVector& EventLocation, float EventRadius, ECrowd_ReactionType ReactionType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGlobalCrowdBehavior(ECrowd_BehaviorState NewBehaviorState);

protected:
    // Core crowd data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_SpawnGroup> SpawnGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    float UpdateFrequency;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Config")
    TArray<float> LODDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Config")
    float MaxRenderDistance;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActiveEntityCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 VisibleEntityCount;

private:
    // Internal systems
    void UpdateEntityPositions(float DeltaTime);
    void UpdateEntityBehaviors(float DeltaTime);
    void CullDistantEntities(const FVector& ViewerLocation);
    void OptimizeEntityCount();
    
    // Utility functions
    FVector CalculateEntityMovement(const FCrowd_EntityData& Entity, float DeltaTime);
    bool IsEntityVisible(const FCrowd_EntityData& Entity, const FVector& ViewerLocation);
    void RecycleInactiveEntities();

    // Entity ID management
    int32 NextEntityID;
    TArray<int32> FreeEntityIDs;

    // Timing
    float LastUpdateTime;
    float AccumulatedTime;
};