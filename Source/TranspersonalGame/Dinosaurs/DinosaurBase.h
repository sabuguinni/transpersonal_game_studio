// DinosaurBase.h
// Engine Architect #02 — PROD_CYCLE_AUTO_20260630_005
// Base class for all dinosaur pawns. Inherits ACharacter for movement + capsule.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) derive from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Small   UMETA(DisplayName = "Small"),
    Medium  UMETA(DisplayName = "Medium"),
    Large   UMETA(DisplayName = "Large"),
    Massive UMETA(DisplayName = "Massive")
};

// ─── Main Class ──────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
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

    // ─── Species Identity ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSize DinosaurSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName;

    // ─── Survival Stats ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float StaminaRegenRate = 10.0f;

    // ─── Combat ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    // ─── Territory ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float AggressionLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    // ─── AI State ────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoAIState CurrentAIState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* TargetActor;

    // ─── Public API ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetAIState(EDinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Dinosaur")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur")
    bool IsWithinTerritory(const FVector& Location) const;

    // ─── Blueprint Events ────────────────────────────────────────────────────

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDamaged(float DamageAmount, AActor* DamageCauser);
    virtual void OnDinosaurDamaged_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDied();
    virtual void OnDinosaurDied_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAIStateChanged(EDinoAIState NewState);
    virtual void OnAIStateChanged_Implementation(EDinoAIState NewState);

private:
    FTimerHandle BehaviourTimerHandle;

    void UpdateBehaviour();
    void TickIdleBehaviour();
    void TickPatrolBehaviour();
    void TickFleeBehaviour();
    void TickAttackBehaviour();
};
