#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "../SharedTypes.h"
#include "Crowd_HerdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    APawn* DinosaurPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    float StaminaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    bool bIsAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    float LastFeedTime;

    FCrowd_HerdMember()
    {
        DinosaurPawn = nullptr;
        LocalPosition = FVector::ZeroVector;
        StaminaLevel = 100.0f;
        bIsAlpha = false;
        LastFeedTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    ECrowd_HerdState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    APawn* ThreatTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float CohesionRadius;

    FCrowd_HerdBehaviorState()
    {
        CurrentState = ECrowd_HerdState::Grazing;
        TargetLocation = FVector::ZeroVector;
        StateTimer = 0.0f;
        ThreatTarget = nullptr;
        CohesionRadius = 1000.0f;
    }
};

/**
 * Advanced herd management system for dinosaur populations
 * Handles group behavior, migration patterns, and predator response
 * Optimized for 50-200 dinosaurs per herd with realistic AI
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_HerdManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_HerdManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === HERD COMPOSITION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Setup")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Setup")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Setup")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Setup")
    TSubclassOf<APawn> DinosaurClass;

    // === BEHAVIOR PARAMETERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    FCrowd_HerdBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float GrazingDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float MigrationDistance;

    // === FLOCKING PARAMETERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius;

    // === MIGRATION SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsMigrating;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxProcessedPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    // === CORE METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SpawnHerd(FVector CenterLocation, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void AddHerdMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void RemoveHerdMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SetHerdState(ECrowd_HerdState NewState);

    // === BEHAVIOR METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void UpdateHerdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void ProcessGrazingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void ProcessMigrationBehavior();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void ProcessFleeingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void DetectThreats();

    // === FLOCKING METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateFlockingForce(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion(const FCrowd_HerdMember& Member);

    // === MIGRATION METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void SetMigrationWaypoints(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    FVector GetNextMigrationTarget();

    // === UTILITY METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Herd Utility")
    float GetHerdCohesion();

    UFUNCTION(BlueprintCallable, Category = "Herd Utility")
    FVector GetHerdCenter();

    UFUNCTION(BlueprintCallable, Category = "Herd Utility")
    APawn* GetAlphaMember();

    UFUNCTION(BlueprintCallable, Category = "Herd Utility")
    int32 GetActiveHerdSize();

private:
    // Internal state tracking
    float LastUpdateTime;
    int32 CurrentProcessIndex;
    TArray<APawn*> NearbyThreats;
    
    // Performance optimization
    void UpdateHerdMemberLOD(FCrowd_HerdMember& Member);
    bool ShouldProcessMember(const FCrowd_HerdMember& Member);
    void OptimizeHerdPerformance();
};