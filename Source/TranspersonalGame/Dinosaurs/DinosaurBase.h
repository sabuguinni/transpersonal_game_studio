// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species in the prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor          UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Generic         UMETA(DisplayName = "Generic Dinosaur")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Resting     UMETA(DisplayName = "Resting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Socializing UMETA(DisplayName = "Socializing")
};

// ============================================================
// ADinosaurBase — Base class for all dinosaur actors
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Dinosaurs), meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Species Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsJuvenile;

    // ---- Health & Survival ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHunger;

    // ---- Movement ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ---- Combat ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    // ---- Behavior ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    AActor* CurrentTarget;

    // ---- Pack Dynamics ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Pack")
    ADinosaurBase* PackLeader;

    // ---- AI Perception ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    UAIPerceptionStimuliSourceComponent* StimuliSource;

    // ---- Public Interface ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void SetPackLeader(ADinosaurBase* Leader);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Pack")
    ADinosaurBase* GetPackLeader() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHungerPercent() const;

    // Damage override
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

protected:
    // ---- Species Configuration ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Setup")
    void ApplySpeciesTraits();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Behavior")
    void OnBehaviorStateChanged(EDinosaurBehaviorState NewState);
    virtual void OnBehaviorStateChanged_Implementation(EDinosaurBehaviorState NewState) {}

    UFUNCTION()
    void OnDinosaurDeath();

    // ---- Internal Timers ----
    UFUNCTION()
    void UpdateBehaviorState();

    UFUNCTION()
    void TickHunger();

private:
    FTimerHandle BehaviorTickHandle;
    FTimerHandle HungerTickHandle;

    bool bCanAttack;
    float TimeSinceLastAttack;

    FVector HomeLocation;
    bool bHasHomeLocation;
};
