#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Crowd_FlockingManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_FlockingBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Drinking    UMETA(DisplayName = "Drinking"), 
    Resting     UMETA(DisplayName = "Resting"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Alerting    UMETA(DisplayName = "Alerting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    ECrowd_FlockingBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    bool bIsFlockLeader;

    FCrowd_FlockingUnit()
    {
        Actor = nullptr;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        CurrentBehavior = ECrowd_FlockingBehavior::Grazing;
        SeparationRadius = 200.0f;
        AlignmentRadius = 500.0f;
        CohesionRadius = 800.0f;
        bIsFlockLeader = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    TArray<FCrowd_FlockingUnit> FlockMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector FlockCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector TargetDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    ECrowd_FlockingBehavior GroupBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    bool bIsMigrating;

    FCrowd_FlockingGroup()
    {
        FlockCenter = FVector::ZeroVector;
        TargetDestination = FVector::ZeroVector;
        GroupBehavior = ECrowd_FlockingBehavior::Grazing;
        GroupRadius = 1000.0f;
        MovementSpeed = 150.0f;
        bIsMigrating = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACrowd_FlockingManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_FlockingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Flocking Groups Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_FlockingGroup> FlockingGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float FlockingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float PredatorDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float FleeingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableFlockingSimulation;

    // Flocking Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    float AvoidanceWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Parameters")
    float SeekWeight;

    // Core Flocking Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeFlockingGroups();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterFlockingUnit(AActor* Actor, int32 GroupIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemoveFlockingUnit(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupBehavior(int32 GroupIndex, ECrowd_FlockingBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGroupDestination(int32 GroupIndex, FVector Destination);

    // Flocking Algorithm Functions
    UFUNCTION(BlueprintCallable, Category = "Flocking Algorithm")
    FVector CalculateSeparation(const FCrowd_FlockingUnit& Unit, const TArray<FCrowd_FlockingUnit>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking Algorithm")
    FVector CalculateAlignment(const FCrowd_FlockingUnit& Unit, const TArray<FCrowd_FlockingUnit>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking Algorithm")
    FVector CalculateCohesion(const FCrowd_FlockingUnit& Unit, const TArray<FCrowd_FlockingUnit>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking Algorithm")
    FVector CalculateAvoidance(const FCrowd_FlockingUnit& Unit);

    UFUNCTION(BlueprintCallable, Category = "Flocking Algorithm")
    FVector CalculateSeek(const FCrowd_FlockingUnit& Unit, FVector Target);

    // Predator Detection and Response
    UFUNCTION(BlueprintCallable, Category = "Predator Response")
    void DetectPredators();

    UFUNCTION(BlueprintCallable, Category = "Predator Response")
    void TriggerFleeingBehavior(int32 GroupIndex, FVector PredatorLocation);

    UFUNCTION(BlueprintCallable, Category = "Predator Response")
    bool IsPredatorNearby(FVector Position, float DetectionRadius);

    // Migration and Movement
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration(int32 GroupIndex, const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateMigrationProgress(int32 GroupIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool HasReachedDestination(const FCrowd_FlockingGroup& Group);

private:
    float LastFlockingUpdate;
    TArray<FVector> PredatorLocations;
    
    void UpdateFlockCenter(FCrowd_FlockingGroup& Group);
    void ApplyFlockingForces(FCrowd_FlockingUnit& Unit, const TArray<FCrowd_FlockingUnit>& GroupMembers);
    TArray<FCrowd_FlockingUnit> GetNeighbors(const FCrowd_FlockingUnit& Unit, const TArray<FCrowd_FlockingUnit>& AllUnits, float Radius);
};