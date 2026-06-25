#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Graze       UMETA(DisplayName = "Graze"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Rest        UMETA(DisplayName = "Rest"),
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AggroRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float SprintSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PackCoordinationRadius = 1500.0f;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsActive = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdatePerception(float DeltaTime);

    // ── Patrol ──
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    bool HasReachedPatrolPoint() const;

    // ── Combat ──
    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    void PerformAttack();

    // ── Memory ──
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void UpdateMemory(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasActiveThreatMemory() const { return ThreatMemory.bIsActive; }

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FVector GetLastKnownThreatLocation() const { return ThreatMemory.LastKnownLocation; }

    // ── Pack Behavior ──
    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void CoordinateWithPackMembers();

    UFUNCTION(BlueprintPure, Category = "NPC|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

    // ── Config ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FVector HomeLocation = FVector::ZeroVector;

    // ── Runtime State ──
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Patrol;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float HungerLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float AlertLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    FVector CurrentPatrolTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    TArray<UNPCBehaviorComponent*> PackMembers;

private:
    FNPC_MemoryEntry ThreatMemory;

    float StateTimer = 0.0f;
    float PerceptionTimer = 0.0f;
    float PatrolWaitTimer = 0.0f;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickGraze(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    void ScanForThreats();
    void ChooseNewPatrolPoint();
    bool IsTargetInRange(float Range) const;
    void DecayMemory(float DeltaTime);
};
