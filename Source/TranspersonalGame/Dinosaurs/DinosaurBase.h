// DinosaurBase.h
// Engine Architect #02 — Cycle AUTO_20260629_011
// Base class for all dinosaur species. Provides movement, AI state, health, hunger, detection.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ─────────────────────────────────────────────
// ENUMS — global scope (UHT requirement)
// ─────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Allosaurus          UMETA(DisplayName = "Allosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Returning   UMETA(DisplayName = "Returning to Territory"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────
class UAIPerceptionStimuliSourceComponent;

// ─────────────────────────────────────────────
// ADinosaurBase
// ─────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs),
    meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species Identity ──────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName;

    // ── Health ────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float DamageResistance;

    // ── Combat ────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    // ── Movement ─────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float MoveSpeed_Walk;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float MoveSpeed_Run;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float MoveSpeed_Chase;

    // ── AI Behavior ───────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinosaurBehavior BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float HearingRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAwareOfPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector LastKnownPlayerLocation;

    // ── Ecology ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Ecology")
    bool bIsCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Ecology")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Ecology")
    bool bIsNocturnal;

    // ── Hunger ────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Hunger")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Hunger")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Hunger")
    float HungerDecayRate;

    // ── Territory ─────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    bool bIsTerritory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector TerritoryCenter;

    // ── Roar ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float RoarCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    float LastRoarTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsDead;

    // ── Perception ────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Perception")
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // ── Blueprint-callable functions ──────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void Die();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    virtual void Roar();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void DetectPlayer();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Territory")
    FVector GetTerritoryCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EDinosaurBehavior NewState);

protected:
    // Timer handles
    FTimerHandle HungerTimerHandle;
    FTimerHandle BehaviorTimerHandle;

    UFUNCTION()
    void UpdateHunger();

    UFUNCTION()
    void UpdateBehaviorState();
};
