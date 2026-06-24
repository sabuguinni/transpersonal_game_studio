#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// NPC behavior states — used by Behavior Tree and AnimInstance
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

// NPC memory entry — what this NPC remembers about a stimulus
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsVisible = false;
};

// NPC patrol waypoint
USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

/**
 * UNPCBehaviorComponent
 * Core NPC behavior component — drives state machine, memory, and patrol logic.
 * Attach to any ACharacter or APawn to give it NPC behavior.
 * Designed to work with UE5 Behavior Trees via Blackboard keys.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === STATE MACHINE ===

    UFUNCTION(BlueprintCallable, Category = "NPC|State")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsHostile() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsFleeing() const { return CurrentState == ENPC_BehaviorState::Flee; }

    // === THREAT DETECTION ===

    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, FVector LastKnownLoc);

    UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "NPC|Detection")
    float GetHighestThreatLevel() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Detection")
    AActor* GetHighestThreat() const;

    // === PATROL ===

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AddPatrolPoint(FVector Location, float WaitTime = 2.0f, bool bLookAround = true);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolIndex();

    // === MEMORY ===

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void UpdateMemory(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOf(AActor* Actor) const;

    // === PROPERTIES ===

    // Detection ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FleeThreshold = 0.2f;  // Health ratio below which NPC flees

    // Memory decay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate = 0.1f;  // Threat level lost per second when not visible

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryForgetTime = 30.0f;  // Seconds until memory is fully forgotten

    // Patrol config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopPatrol = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolAcceptanceRadius = 150.0f;

    // Current health ratio (0-1), set by health system
    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float HealthRatio = 1.0f;

    // Fear level (0-1), drives flee behavior
    UPROPERTY(BlueprintReadWrite, Category = "NPC|State")
    float FearLevel = 0.0f;

private:
    UPROPERTY()
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    ENPC_BehaviorState PreviousState = ENPC_BehaviorState::Idle;

    UPROPERTY()
    TArray<FNPC_MemoryEntry> ThreatMemory;

    UPROPERTY()
    TArray<FNPC_PatrolPoint> PatrolPoints;

    int32 CurrentPatrolIndex = 0;

    void EvaluateStateTransition();
    void OnStateEnter(ENPC_BehaviorState NewState);
    void OnStateExit(ENPC_BehaviorState OldState);
};
