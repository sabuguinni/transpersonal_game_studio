#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNPCBehaviorComponent;
class UBlackboardComponent;
class UBehaviorTreeComponent;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Chasing         UMETA(DisplayName = "Chasing"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpeciesType : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FCombat_SpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HearingRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackFlankRadius = 400.0f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float TimeSinceDetected = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FString ThreatActorName = TEXT("");
};

/**
 * ADinosaurCombatAIController
 * AI Controller for dinosaur combat behavior.
 * Manages state machine, threat detection, pack coordination, and NPC notification.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // === State Machine ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return CurrentState; }

    // === Threat Detection ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(const FVector& ThreatLocation, const FString& ThreatName, float ThreatLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearThreat(const FString& ThreatName);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FVector GetPrimaryThreatLocation() const;

    // === Pack Coordination ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyPackMembersOfThreat(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssignFlankPosition(int32 PackMemberIndex);

    // === NPC Notification ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertNearbyNPCs(const FVector& ThreatLocation, float AlertRadius = 1500.0f);

    // === Species Config ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetSpecies(ECombat_DinoSpeciesType Species);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FCombat_SpeciesConfig GetSpeciesConfig() const { return ActiveSpeciesConfig; }

    // === Combat Actions ===
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BeginChase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BeginFlee(const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ResumePatrol();

protected:
    // === Internal Tick Logic ===
    void TickThreatDecay(float DeltaTime);
    void TickStateTransitions(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);

    // === Perception ===
    void ScanForThreats();
    bool IsPlayerInSightRadius() const;
    float GetDistanceToPlayer() const;

    // === Species Configs ===
    FCombat_SpeciesConfig BuildSpeciesConfig(ECombat_DinoSpeciesType Species) const;

    // === Properties ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpeciesType DinoSpecies = ECombat_DinoSpeciesType::TRex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FCombat_SpeciesConfig ActiveSpeciesConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_ThreatEntry> ActiveThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatExpireTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float AttackCooldownRemaining = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float StateTimeElapsed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    int32 PackMemberIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackLeader = false;

    // Weak ref to current chase target
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TWeakObjectPtr<AActor> ChaseTarget;

private:
    float ScanInterval = 0.5f;
    float ScanTimer = 0.0f;
};
