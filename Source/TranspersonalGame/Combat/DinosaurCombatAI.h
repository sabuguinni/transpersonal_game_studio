#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// Combat AI enums — ECombat_ prefix to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Charge          UMETA(DisplayName = "Charge"),
    Swipe           UMETA(DisplayName = "Swipe"),
    Stomp           UMETA(DisplayName = "Stomp"),
    DiveStrike      UMETA(DisplayName = "Dive Strike")
};

// ============================================================
// Combat AI structs — FCombat_ prefix
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatLevel = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float LastSeenTimestamp = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bHasLineOfSight = false;
};

USTRUCT(BlueprintType)
struct FCombat_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    ECombat_DinoState SignalState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    float SignalTimestamp = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    bool bIsAlphaSignal = false;
};

// ============================================================
// Main Combat AI Component
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    // ---- Species & State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // ---- Threat tracking ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    TArray<FCombat_ThreatEntry> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* PrimaryTarget = nullptr;

    // ---- Pack coordination ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsAlpha = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FCombat_PackSignal LastPackSignal;

    // ---- Attack timing ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Attack")
    float LastAttackTime = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Attack")
    ECombat_AttackType LastAttackType = ECombat_AttackType::Bite;

    // ---- Tick interval ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Performance")
    float AITickInterval = 0.1f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, bool bHasLOS);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void OnTakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    ECombat_AttackType SelectAttackType() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void BroadcastPackSignal(ECombat_DinoState SignalState, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ReceivePackSignal(const FCombat_PackSignal& Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void AddPackMember(UDinosaurCombatAI* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    FVector GetLastKnownTargetLocation() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float AITickAccumulator = 0.f;

    void UpdateAI(float DeltaTime);
    void UpdateState();
    void UpdateThreats(float DeltaTime);
    void SelectPrimaryTarget();
    void DecayThreatEntries(float DeltaTime);
    bool HasLineOfSightToTarget() const;
    float GetDistanceToTarget() const;
    void TransitionToState(ECombat_DinoState NewState);
    void ApplySpeciesDefaults();
};
