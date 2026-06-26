// DinosaurBase.h — Core Systems Programmer #03 — Cycle 008
// Prehistoric survival game: base class for all dinosaur AI pawns.
// Provides behavior state machine, damage handling, detection, and death ragdoll.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/SharedTypes.h"
#include "DinosaurBase.generated.h"

// Forward declarations
class USphereComponent;
class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboardComponent;

/** Dinosaur behavior states — drives animation and AI logic */
UENUM(BlueprintType)
enum class ECore_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead"),
};

/** Species classification for behavior differentiation */
UENUM(BlueprintType)
enum class ECore_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Generic         UMETA(DisplayName = "Generic"),
};

/** Runtime stats for a dinosaur instance */
USTRUCT(BlueprintType)
struct FCore_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float CurrentHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackDamage = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float DetectionRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MoveSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AggressionLevel = 0.5f;  // 0=passive, 1=always aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    bool bIsCarnivore = true;
};

/**
 * ADinosaurBase — Base class for all dinosaur actors in the prehistoric survival game.
 * Extends ACharacter to get free movement, capsule collision, and skeletal mesh.
 * Implements a behavior state machine: Idle → Alert → Hunting → Attacking → Dead.
 * Damage handling escalates aggression; death triggers ragdoll physics.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Lifecycle ──────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Damage & Death ─────────────────────────────────────────────────────────
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void ApplyMeleeDamage(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    bool IsAlive() const;

    // ── Behavior State Machine ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    void SetDinoState(ECore_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dino|Behavior")
    ECore_DinoState GetDinoState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    void UpdateBehavior(float DeltaTime);

    // ── Detection ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dino|Detection")
    AActor* FindNearestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Dino|Detection")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Dino|Detection")
    bool IsPlayerInDetectionRange() const;

    // ── Properties ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Config")
    ECore_DinoSpecies Species = ECore_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Config")
    FCore_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Config")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    // ── Events (Blueprint-implementable) ───────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Dino|Events")
    void OnDinoStateChanged(ECore_DinoState OldState, ECore_DinoState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dino|Events")
    void OnDinoDied(AActor* Killer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dino|Events")
    void OnDinoAttack(AActor* Target);

protected:
    // ── Internal State ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|State",
        meta = (AllowPrivateAccess = "true"))
    ECore_DinoState CurrentState = ECore_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|State",
        meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|State",
        meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastAttack = 0.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dino|Config")
    float AttackCooldown = 2.0f;

    // ── Internal helpers ───────────────────────────────────────────────────────
    void HandleDeath(AActor* Killer);
    void ActivateRagdoll();
    void TickIdle(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickHunting(float DeltaTime);
    void TickAttacking(float DeltaTime);
};
