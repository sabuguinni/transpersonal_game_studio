// DinosaurBase.h — Engine Architect #02 — PROD_CYCLE_AUTO_20260702_002
// Base class for all dinosaur species in the prehistoric survival world.
// All dinosaur types (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinoSpeciesDiet : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinoSpeciesDiet DietType;

    FDinosaurSpeciesData()
        : SpeciesName(NAME_None)
        , MaxHealth(100.0f)
        , MoveSpeed(400.0f)
        , AttackDamage(20.0f)
        , DetectionRadius(1500.0f)
        , bIsPredator(false)
        , DietType(EDinoSpeciesDiet::Herbivore)
    {}
};

// ─── Class ───────────────────────────────────────────────────────────────────

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

    // ─── Species Configuration ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FDinosaurSpeciesData SpeciesData;

    // ─── Runtime State ────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinosaurState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAlerted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    float AlertCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    AActor* AlertTarget;

    // ─── Public Interface ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetState(EDinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void AlertDinosaur(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    bool IsAlive() const { return !bIsDead && CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|State")
    float GetHealthPercent() const
    {
        return (SpeciesData.MaxHealth > 0.0f)
            ? (CurrentHealth / SpeciesData.MaxHealth)
            : 0.0f;
    }

protected:
    // ─── Behavior Hooks (override in subclasses) ──────────────────────────
    virtual void OnIdle(float DeltaTime);
    virtual void OnRoam(float DeltaTime);
    virtual void OnAlert(float DeltaTime);
    virtual void OnAttack(float DeltaTime);
    virtual void OnFlee(float DeltaTime);
    virtual void OnDeath();
    virtual void OnDamageReceived(float Damage, AActor* DamageCauser);
    virtual void OnStateChanged(EDinosaurState OldState, EDinosaurState NewState);

private:
    FTimerHandle BehaviorTickHandle;

    UFUNCTION()
    void UpdateBehaviorState();
};
