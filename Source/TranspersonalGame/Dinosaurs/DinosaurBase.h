// DinosaurBase.h
// Core Systems Programmer — Agent #3
// Base pawn class for all dinosaur species in the prehistoric survival game.
// Provides: health, territory radius, aggro state, attack damage, movement speed.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) inherit from this class.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "DinosaurBase.generated.h"

// Aggro state enum — drives behavior tree transitions
UENUM(BlueprintType)
enum class ECore_DinoAggroState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead"),
};

// Diet type — determines prey/predator relationships
UENUM(BlueprintType)
enum class ECore_DinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

// Struct for dinosaur base stats — used for data-driven species configuration
USTRUCT(BlueprintType)
struct FCore_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    ECore_DinoDiet Diet = ECore_DinoDiet::Carnivore;
};

/**
 * ADinosaurBase
 *
 * Base ACharacter subclass for all dinosaur species.
 * Provides health/damage system, territory/aggro detection via sphere components,
 * and state machine (ECore_DinoAggroState) for behavior tree integration.
 *
 * Usage: Subclass this for each species (ATRex, ARaptor, ABrachiosaurus, etc.)
 * and override OnAttack(), OnDeath(), and GetSpeciesName().
 */
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Health System ---

    /** Current health. Reaches 0 → triggers OnDeath(). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    /** Apply damage to this dinosaur. Returns actual damage dealt. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    /** Heal the dinosaur by the given amount (clamped to MaxHealth). */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    void HealDinosaur(float HealAmount);

    /** Returns true if the dinosaur is alive. */
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const;

    // --- Species Configuration ---

    /** Data-driven stats for this species. Set per-species in Blueprint defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FCore_DinoStats DinoStats;

    /** Human-readable species name (e.g., "Tyrannosaurus Rex"). */
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Species")
    virtual FString GetSpeciesName() const;

    // --- Aggro State Machine ---

    /** Current behavioral state. Drives animation and AI. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    ECore_DinoAggroState AggroState;

    /** Set a new aggro state and broadcast the change. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetAggroState(ECore_DinoAggroState NewState);

    /** The actor this dinosaur is currently targeting (player or other prey). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget;

    /** Home position — center of territory. Set on BeginPlay from spawn location. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector TerritoryCenter;

    // --- Attack System ---

    /** Execute a melee attack on CurrentTarget. Called by behavior tree. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void PerformAttack();

    /** Returns true if attack cooldown has elapsed and attack is ready. */
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    // --- Blueprint Events ---

    /** Called when this dinosaur dies. Override in Blueprint for death FX/ragdoll. */
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath(AActor* Killer);
    virtual void OnDeath_Implementation(AActor* Killer);

    /** Called when aggro state changes. Override in Blueprint for audio/animation. */
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAggroStateChanged(ECore_DinoAggroState OldState, ECore_DinoAggroState NewState);
    virtual void OnAggroStateChanged_Implementation(ECore_DinoAggroState OldState, ECore_DinoAggroState NewState);

protected:
    /** Sphere component for territory detection (patrol boundary). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TerritoryVolume;

    /** Sphere component for aggro detection (triggers chase). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* AggroVolume;

    /** Timer handle for attack cooldown tracking. */
    FTimerHandle AttackCooldownTimer;

    /** True when attack is on cooldown. */
    bool bAttackOnCooldown;

    /** Called when a pawn enters the aggro sphere. */
    UFUNCTION()
    void OnAggroVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                   bool bFromSweep, const FHitResult& SweepResult);

    /** Called when a pawn leaves the aggro sphere. */
    UFUNCTION()
    void OnAggroVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /** Reset attack cooldown flag. */
    UFUNCTION()
    void ResetAttackCooldown();
};
