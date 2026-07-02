#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// NPC Behavior Component — Agent #11
// Provides daily routine, memory, social relations, and
// emergent behavior for all non-player characters.
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Roar        UMETA(DisplayName = "Roar"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Social      UMETA(DisplayName = "Social")
};

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Hostile     UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    PlayerSighting  UMETA(DisplayName = "Player Sighting"),
    ThreatLocation  UMETA(DisplayName = "Threat Location"),
    FoodSource      UMETA(DisplayName = "Food Source"),
    PackMember      UMETA(DisplayName = "Pack Member"),
    TerritoryBreach UMETA(DisplayName = "Territory Breach")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_MemoryType MemoryType = ENPC_MemoryType::ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float Confidence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    AActor* SourceActor = nullptr;
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    // Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolWaitTime = 2.5f;

    // Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float SightRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float SightAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float SmellRange = 800.0f;

    // Chase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackRange = 300.0f;

    // Memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    // Pack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    float PackCommunicationRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    bool bIsAlphaLeader = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_AlertLevel GetAlertLevel() const { return AlertLevel; }

    // ── Perception ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    bool CanHearTarget(AActor* Target, float NoiseLevel = 1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnTargetDetected(AActor* Target, ENPC_MemoryType DetectionType);

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void OnNoiseMade(FVector NoiseLocation, float NoiseRadius, float Loudness);

    // ── Memory ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddMemory(ENPC_MemoryType Type, FVector Location, AActor* Source, float Confidence = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories();

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOf(ENPC_MemoryType Type) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FVector GetLastKnownThreatLocation() const;

    // ── Pack Behavior ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void AlertPackMembers(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void ReceivePackAlert(FVector ThreatLocation, AActor* AlertSource);

    UFUNCTION(BlueprintPure, Category = "NPC|Pack")
    bool IsPackLeader() const { return BehaviorConfig.bIsAlphaLeader; }

    // ── Daily Routine ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float TimeOfDay);

    // ── Configuration ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FVector HomeLocation = FVector::ZeroVector;

    // ── State Accessors ────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float TimeSinceLastStateChange = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float AlertDecayTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    TArray<FNPC_MemoryEntry> MemoryBank;

private:
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;
    ENPC_AlertLevel AlertLevel = ENPC_AlertLevel::Calm;

    float StateTimer = 0.0f;
    float PatrolWaitTimer = 0.0f;
    FVector CurrentPatrolTarget = FVector::ZeroVector;
    bool bHasPatrolTarget = false;

    void UpdateStateMachine(float DeltaTime);
    void UpdateAlertLevel(float DeltaTime);
    void SelectNewPatrolPoint();
    void EvaluateThreats();
    bool IsWithinPatrolBounds() const;
};
