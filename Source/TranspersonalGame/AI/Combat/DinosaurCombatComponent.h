#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatComponent.generated.h"

// ============================================================
// Combat state enum — Agent #12 Combat AI
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
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

// ============================================================
// Attack type enum
// ============================================================
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    TailSwipe       UMETA(DisplayName = "Tail Swipe"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Roar            UMETA(DisplayName = "Roar")
};

// ============================================================
// Species behaviour profile — drives AI parameters per species
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_SpeciesProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FName SpeciesName = NAME_None;

    // Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightAngleDegrees = 120.f;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.f;

    // Behaviour flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    bool bIsAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    bool bFleeWhenInjured = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    float TerritoryRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;
};

// ============================================================
// Combat event — fired when attack connects
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_AttackEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    float DamageDealt = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Attacker = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Event")
    AActor* Target = nullptr;
};

// ============================================================
// Delegate declarations
// ============================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnAttackLanded, const FCombat_AttackEvent&, AttackEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnStateChanged, ECombat_DinoState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnDeath, AActor*, Killer);

// ============================================================
// DinosaurCombatComponent — core combat AI for dinosaur pawns
// ============================================================
UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent),
    BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatComponent();

    // ---- Species configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_SpeciesProfile SpeciesProfile;

    // ---- Runtime state ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 500.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float ThreatLevel = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsStunned = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float StunDuration = 0.f;

    // ---- Pack coordination ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatComponent*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 1500.f;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackLanded OnAttackLanded;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnDeath OnDeath;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReceiveDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyStun(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterPackMember(UDinosaurCombatComponent* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackToTarget(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsTargetInSight(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsTargetInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitialiseAsSpecies(FName Species);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateCombatTick(float DeltaTime);
    void UpdateStun(float DeltaTime);
    void EvaluateStateTransitions();
    void HandleChargingState(float DeltaTime);
    void HandleStalkingState(float DeltaTime);
    void TriggerDeathSequence(AActor* Killer);

    float StunTimer = 0.f;
    float ThreatDecayRate = 0.5f;
    FVector HomeLocation = FVector::ZeroVector;
    bool bHasHome = false;
};
