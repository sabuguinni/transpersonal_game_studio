#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_TribalBehaviorManager.generated.h"

// Forward declarations
class APawn;
class ACharacter;
class AAIController;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Leader      UMETA(DisplayName = "Tribal Leader"),
    Scout       UMETA(DisplayName = "Scout/Hunter"), 
    Gatherer    UMETA(DisplayName = "Resource Gatherer"),
    Guard       UMETA(DisplayName = "Camp Guard"),
    Crafter     UMETA(DisplayName = "Tool Crafter")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle/Resting"),
    Patrolling  UMETA(DisplayName = "Patrolling Area"),
    Gathering   UMETA(DisplayName = "Gathering Resources"),
    Hunting     UMETA(DisplayName = "Hunting/Scouting"),
    Fleeing     UMETA(DisplayName = "Fleeing from Threat"),
    Following   UMETA(DisplayName = "Following Leader"),
    Guarding    UMETA(DisplayName = "Guarding Location")
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    // Last known threat location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastThreatLocation;

    // Time when threat was last seen
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastThreatTime;

    // Known resource locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> ResourceLocations;

    // Known safe areas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    // Relationship with player (-1 to 1, hostile to friendly)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float PlayerRelationship;

    FNPC_TribalMemory()
    {
        LastThreatLocation = FVector::ZeroVector;
        LastThreatTime = 0.0f;
        PlayerRelationship = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsRestPoint;

    FNPC_PatrolPoint()
    {
        Location = FVector::ZeroVector;
        WaitTime = 3.0f;
        bIsRestPoint = false;
    }
};

/**
 * Manages complex tribal NPC behavior including social dynamics, memory, and decision making
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE BEHAVIOR PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FNPC_TribalMemory TribalMemory;

    // Group leadership and following
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    TWeakObjectPtr<UNPC_TribalBehaviorManager> GroupLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    TArray<TWeakObjectPtr<UNPC_TribalBehaviorManager>> GroupFollowers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    float GroupCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Dynamics")
    float LeadershipInfluence;

    // === PATROL AND MOVEMENT ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsPatrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    // === THREAT DETECTION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    TWeakObjectPtr<AActor> CurrentThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float AlertThreshold;

    // === DECISION MAKING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float DecisionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float LastDecisionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float Loyalty;

    // === CORE BEHAVIOR FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ReactToThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateTribalMemory(FVector Location, bool bIsThreat, bool bIsResource);

    // === GROUP DYNAMICS ===

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void SetGroupLeader(UNPC_TribalBehaviorManager* Leader);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void AddGroupFollower(UNPC_TribalBehaviorManager* Follower);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void RemoveGroupFollower(UNPC_TribalBehaviorManager* Follower);

    UFUNCTION(BlueprintCallable, Category = "Group Dynamics")
    void IssueGroupCommand(ENPC_BehaviorState CommandState, FVector TargetLocation);

    // === PATROL SYSTEM ===

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StopPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void AddPatrolPoint(FVector Location, float WaitTime = 3.0f, bool bIsRestPoint = false);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void MoveToNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetCurrentPatrolTarget();

    // === THREAT ASSESSMENT ===

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    float EvaluateThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void AlertNearbyTribalMembers(AActor* ThreatActor);

    // === DECISION MAKING ===

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    ENPC_BehaviorState DetermineOptimalBehavior();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFleeFromThreat(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFollowLeader();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    FVector FindSafeLocation();

    // === UTILITY FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Utility")
    ACharacter* GetOwnerCharacter();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    AAIController* GetOwnerAIController();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<UNPC_TribalBehaviorManager*> GetNearbyTribalMembers(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LogBehaviorState(const FString& Message);

private:
    // Internal timers
    FTimerHandle DecisionTimerHandle;
    FTimerHandle PatrolTimerHandle;
    FTimerHandle ThreatScanTimerHandle;

    // Internal state tracking
    FVector LastKnownPlayerLocation;
    float LastPlayerSightTime;
    bool bHasSeenPlayer;

    // Helper functions
    void InitializeTribalBehavior();
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateThreatResponse(float DeltaTime);
    void UpdateGroupDynamics(float DeltaTime);
    void ExecuteBehaviorState(float DeltaTime);
};