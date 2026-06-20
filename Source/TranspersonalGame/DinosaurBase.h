#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

/**
 * ADinosaurBase — Abstract base class for all dinosaur species.
 * Provides shared properties: health, detection radius, attack range, move speed.
 * Concrete subclasses: ATRexDinosaur, ARaptorDinosaur, ABrachiosaurusDinosaur.
 *
 * Architecture rule: all dino-specific logic lives in subclasses.
 * This base class only defines the shared interface and default values.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species Identity ──────────────────────────────────────────────────────

    /** Display name of this dinosaur species (e.g. "Tyrannosaurus Rex") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName SpeciesName;

    /** Maximum health points for this species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.f;

    /** Current health — initialised to MaxHealth in BeginPlay */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.f;

    /** Whether this species is a carnivore (attacks player on sight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsCarnivore = true;

    // ── AI Behaviour ──────────────────────────────────────────────────────────

    /** Radius within which this dinosaur detects the player (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius = 1500.f;

    /** Range within which this dinosaur can land an attack (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float AttackRange = 200.f;

    /** Base movement speed (cm/s) — applied to CharacterMovement in BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float MoveSpeed = 400.f;

    /** Whether this dinosaur is currently in aggressive state */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAggressive = false;

    /** Cooldown between attacks in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float AttackCooldown = 2.0f;

    // ── Combat ────────────────────────────────────────────────────────────────

    /** Damage dealt per attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage = 25.f;

    /**
     * Apply damage to this dinosaur.
     * @param Damage      Amount of damage to apply
     * @param DamageCauser Actor that caused the damage (player, trap, etc.)
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void TakeDamageDino(float Damage, AActor* DamageCauser);

    /**
     * Execute an attack against the target actor.
     * Subclasses override to apply species-specific damage and animations.
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void Attack(AActor* Target);

    /**
     * Set the aggressive state of this dinosaur.
     * When aggressive, the dinosaur pursues and attacks the player.
     */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void SetAggressive(bool bNewAggressive);

    /**
     * Called when this dinosaur's health reaches zero.
     * Subclasses can override to play death animations, drop loot, etc.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Combat")
    void OnDeath();
    virtual void OnDeath_Implementation();

    /** Returns true if this dinosaur is alive (CurrentHealth > 0) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const { return CurrentHealth > 0.f; }

protected:
    /** Time since last attack — used for cooldown tracking */
    float TimeSinceLastAttack = 0.f;

    /** Simple proximity check — returns nearest player pawn within DetectionRadius, or nullptr */
    AActor* FindNearestPlayer() const;
};
