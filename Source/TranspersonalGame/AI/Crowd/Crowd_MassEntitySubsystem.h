#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowdBehaviorState BehaviorState;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 150.0f;
        EntityID = -1;
        BehaviorState = ECrowdBehaviorState::Wandering;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    TSubclassOf<APawn> EntityClass;

    FCrowd_SpawnParameters()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRadius = 500.0f;
        EntityCount = 50;
        EntityClass = nullptr;
    }
};

/**
 * Mass Entity Subsystem for crowd simulation
 * Manages up to 50,000 crowd entities using UE5 Mass Entity framework
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FCrowd_SpawnParameters& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    // Pathfinding and navigation
    UFUNCTION(BlueprintCallable, Category = "Crowd Navigation")
    void AddWaypoint(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Navigation")
    void RemoveWaypoint(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Navigation")
    void SetGatheringPoint(const FVector& Location, float Radius);

    // Behavior control
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGlobalBehaviorState(ECrowdBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void TriggerCrowdAlert(const FVector& AlertLocation, float AlertRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetCrowdAvoidanceRadius(float NewRadius);

protected:
    // Core crowd data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float MaxSimulationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdEntities;

    // Navigation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    FVector GatheringPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float GatheringRadius;

    // Behavior settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowdBehaviorState GlobalBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AvoidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    FVector AlertLocation;

private:
    // Internal simulation methods
    void UpdateEntityMovement(FCrowd_EntityData& Entity, float DeltaTime);
    void ApplyFlocking(FCrowd_EntityData& Entity, const TArray<FCrowd_EntityData>& NearbyEntities);
    void ApplyPathfinding(FCrowd_EntityData& Entity);
    void ApplyAvoidance(FCrowd_EntityData& Entity, const TArray<FCrowd_EntityData>& NearbyEntities);
    TArray<FCrowd_EntityData> GetNearbyEntities(const FCrowd_EntityData& Entity, float Radius);
    
    // Performance optimization
    void OptimizeLOD();
    bool IsEntityInPlayerView(const FCrowd_EntityData& Entity) const;
    
    // Internal state
    float LastUpdateTime;
    int32 NextEntityID;
    bool bIsSimulationActive;
};