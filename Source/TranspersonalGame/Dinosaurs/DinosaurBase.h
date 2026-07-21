// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260702_008
// Base class for all dinosaur pawns. Inherits ACharacter for movement.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EEng_DinosaurSize : uint8
{
    Small       UMETA(DisplayName = "Small"),       // Compsognathus, Velociraptor
    Medium      UMETA(DisplayName = "Medium"),      // Dilophosaurus, Carnotaurus
    Large       UMETA(DisplayName = "Large"),       // T-Rex, Allosaurus
    Massive     UMETA(DisplayName = "Massive"),     // Brachiosaurus, Argentinosaurus
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ─── Delegate ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurDeath, ADinosaurBase*, DeadDinosaur);

// ─── Class ───────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs),
    meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Species Identity ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FText SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurSize DinoSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPredator;

    // ── Health ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    // ── Movement ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ── Combat ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float ThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    // ── Behavior ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehavior CurrentBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    TWeakObjectPtr<AActor> ThreatTarget;

    // ── Events ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurDeath OnDinosaurDeath;

    // ── Public API ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehavior(EEng_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const;

protected:
    UFUNCTION()
    void UpdateBehavior();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

private:
    FTimerHandle BehaviorUpdateTimer;
};
