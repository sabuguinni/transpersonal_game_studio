#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// Enums — Combat AI State Machine
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Charge          UMETA(DisplayName = "Charge"),
    TailSwipe       UMETA(DisplayName = "Tail Swipe"),
    ClawSlash       UMETA(DisplayName = "Claw Slash"),
    Stomp           UMETA(DisplayName = "Stomp"),
    HornGore        UMETA(DisplayName = "Horn Gore"),
    PinDown         UMETA(DisplayName = "Pin Down")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoSenses
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    float SightRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    float SmellRange = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    bool bHuntsbyMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    bool bHuntsByScent = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    bool bHuntsBySound = true;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float TurnRateDegreesPerSec = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldownSeconds = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.15f;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Damage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float WindupTime = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float RecoveryTime = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanBeCountered = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float CounterWindowSeconds = 0.3f;
};

// ============================================================
// UDinosaurCombatAIComponent
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Species Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Senses")
    FCombat_DinoSenses Senses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attacks")
    TArray<FCombat_AttackData> AvailableAttacks;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsInWindup = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bCounterWindowOpen = false;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(int32 AttackIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInSightRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInSmellRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ForceStateTransition(ECombat_DinoState NewState);

    // ---- Pack Coordination ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackMember = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAIComponent*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RegisterPackMember(UDinosaurCombatAIComponent* Member);

    // ---- Tactical Behavior ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bUsesAmbushTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bCirclesTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float CircleRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    bool bAttacksFromDownwind = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Tactics")
    float AggressionLevel = 0.7f;

    // ---- Events (Blueprint-assignable) ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FSimpleMulticastDelegate OnAttackWindupStart;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FSimpleMulticastDelegate OnCounterWindowOpen;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FSimpleMulticastDelegate OnDinosaurDeath;

private:
    void UpdateStateMachine(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandleStalkingState(float DeltaTime);
    void HandleChargingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleRetreatingState(float DeltaTime);

    bool CanSeeTarget(AActor* Target) const;
    bool CanSmellTarget(AActor* Target) const;
    float GetDistanceToTarget() const;
    void SelectBestAttack();
    void ApplySpeciesDefaults();

    float WindupTimer = 0.0f;
    float CounterWindowTimer = 0.0f;
    int32 SelectedAttackIndex = 0;
};
