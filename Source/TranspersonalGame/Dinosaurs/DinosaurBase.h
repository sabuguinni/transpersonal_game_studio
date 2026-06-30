// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260630_004
// Base class for all dinosaur species in the prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ── Enums (global scope — RULE 1) ──────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead")
};

// ── Main Class ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
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

    // ── Mesh Components ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Mesh",
        meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Mesh",
        meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* HeadMesh;

    // ── Species Identity ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinosaurDiet DietType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName;

    // ── Survival Stats ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    // ── Territory & Detection ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Territory")
    bool bHomeLocationSet;

    // ── Combat ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    // ── Pack Behavior ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    bool bIsInPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    int32 PackSize;

    // ── Behavior State ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinosaurBehavior CurrentBehaviorState;

    // ── Public API ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    EDinosaurBehavior GetCurrentBehavior() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinosaurBehavior NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    FORCEINLINE float GetHealthPercent() const { return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    FORCEINLINE float GetHungerPercent() const { return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    FORCEINLINE float GetStaminaPercent() const { return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f; }

protected:
    // ── Internal Methods ──────────────────────────────────────────────────
    UFUNCTION()
    void OnDeath();

    UFUNCTION()
    void DrainHunger();

    UFUNCTION()
    void UpdateBehavior();

private:
    FTimerHandle HungerTimerHandle;
    FTimerHandle BehaviorTimerHandle;
};
