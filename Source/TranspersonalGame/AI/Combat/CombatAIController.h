#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboard;
class ACombatAICharacter;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Retreating,
    Circling,
    Ambushing,
    Fleeing
};

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
    SoloPredator,
    PackLeader,
    PackMember,
    Ambusher,
    Charger,
    Defender
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* LastKnownTarget;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime;

    UPROPERTY(BlueprintReadWrite)
    EThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite)
    bool bHasBeenDamaged;

    UPROPERTY(BlueprintReadWrite)
    float DamageReceived;

    UPROPERTY(BlueprintReadWrite)
    int32 AttackAttempts;

    UPROPERTY(BlueprintReadWrite)
    float LastAttackTime;

    FCombatMemory()
    {
        LastKnownTarget = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        ThreatLevel = EThreatLevel::None;
        bHasBeenDamaged = false;
        DamageReceived = 0.0f;
        AttackAttempts = 0;
        LastAttackTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    // Combat Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboard* CombatBlackboard;

    // Combat State
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    ECombatRole CombatRole;

    // Combat Memory
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FCombatMemory CombatMemory;

    // Combat Parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Detection")
    float SightRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Detection")
    float HearingRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Detection")
    float PeripheralVisionAngle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack")
    float AttackRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack")
    float AttackCooldown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Movement")
    float PreferredCombatDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Movement")
    float CirclingRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Movement")
    float FleeDistance;

    // Gameplay Tags for Combat States
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Tags")
    FGameplayTagContainer CombatTags;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatRole GetCombatRole() const { return CombatRole; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTargetMemory(AActor* Target, const FVector& Location);

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldCircle() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAmbush() const;

    // Combat Actions
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void ExecuteRetreat();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void ExecuteCircling();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void ExecuteAmbush();

    // Damage Handling
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnDamageReceived(float DamageAmount, AActor* DamageSource);

    // Pack Coordination (for pack hunters)
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RequestPackSupport();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    EThreatLevel AssessThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel();

protected:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Combat Logic
    void UpdateCombatState();
    void ProcessCombatMemory();
    FVector CalculateCirclingPosition() const;
    FVector CalculateAmbushPosition() const;
    FVector CalculateRetreatPosition() const;

    // Blackboard Keys
    static const FName TargetActorKey;
    static const FName TargetLocationKey;
    static const FName CombatStateKey;
    static const FName ThreatLevelKey;
    static const FName LastKnownLocationKey;
    static const FName CanAttackKey;
    static const FName ShouldRetreatKey;
    static const FName PreferredDistanceKey;
};