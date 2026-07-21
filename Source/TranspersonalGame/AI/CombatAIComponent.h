// CombatAIComponent.h
// Agent #12 — Combat & Enemy AI Agent
// Tactical combat AI component for dinosaur enemies

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CombatAIComponent.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TWeakObjectPtr<AActor> ThreatActor;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float Damage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float Range = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float Cooldown = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bIsAreaAttack = false;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnStateChanged, ECombat_AIState, OldState, ECombat_AIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnThreatDetected, AActor*, ThreatActor, float, ThreatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnAttackExecuted, AActor*, Target, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnDamageTaken, float, Damage, AActor*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCombat_OnDeath);

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Combat AI Component")
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInCombat() const;

    // ---- Species ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplySpeciesProfile(ECombat_DinoSpecies Species);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinoSpecies GetSpecies() const { return CurrentSpecies; }

    // ---- Health & Damage ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeCombatDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetMaxHealth() const { return MaxHealth; }

    // ---- Threat ----
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    int32 GetThreatCount() const { return ThreatList.Num(); }

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnAttackExecuted OnAttackExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI|Events")
    FCombat_OnDeath OnDeath;

    // ---- Config Properties ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    ECombat_DinoSpecies CurrentSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    bool bCanFlanking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float ChaseSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    float AlertDuration;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    ECombat_AIState CurrentState;

    UPROPERTY()
    float CurrentHealth;

    UPROPERTY()
    TArray<FCombat_ThreatEntry> ThreatList;

    float AlertTimer = 0.0f;
    bool bAttackOnCooldown = false;

    FTimerHandle AttackCooldownHandle;
    FTimerHandle PatrolTimerHandle;

    void UpdateThreatList();
    void UpdateStateMachine(float DeltaTime);
    void ExecuteAttack(AActor* Target);
    FCombat_ThreatEntry* GetHighestThreat();
    void OnPatrolTimerExpired();
};
