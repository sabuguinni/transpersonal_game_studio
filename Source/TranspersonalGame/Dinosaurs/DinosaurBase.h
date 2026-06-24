// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species. Provides: species identity, behavior state machine,
// survival stats (hunger/thirst), territory system, combat, and AI integration hooks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ENUMS — declared at global scope (UE5 rule)
// Prefixed with EDino_ to avoid conflicts with other agents
// ============================================================

UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    Unknown             UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Dilophosaurus       UMETA(DisplayName = "Dilophosaurus")
};

UENUM(BlueprintType)
enum class EDinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// ADinosaurBase — Base class for all dinosaur pawns
// ============================================================

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs",
    meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --------------------------------------------------------
    // Species Identity
    // --------------------------------------------------------
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName SpeciesDisplayName;

    // --------------------------------------------------------
    // Behavior State Machine
    // --------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDinoBehaviorState BehaviorState;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDinoBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Behavior")
    EDinoBehaviorState GetBehaviorState() const { return BehaviorState; }

    // --------------------------------------------------------
    // Health & Combat
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAttacking;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    float GetHealthPercent() const;

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // --------------------------------------------------------
    // AI Detection
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float AggroRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    int32 PackSize;

    // --------------------------------------------------------
    // Survival Stats
    // --------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float HungerLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float HungerDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float ThirstDecayRate;

    // --------------------------------------------------------
    // Territory
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    bool bHasTerritoryHome;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    // --------------------------------------------------------
    // Rideable (for future mount system)
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Interaction")
    bool bCanBeRidden;

    // --------------------------------------------------------
    // AI Perception
    // --------------------------------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

protected:
    // --------------------------------------------------------
    // Internal helpers
    // --------------------------------------------------------
    virtual void InitializeSpeciesStats();
    virtual void UpdateBehaviorState(float DeltaTime);
    virtual void Die();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDied();
    virtual void OnDinosaurDied_Implementation();

    // Survival timers
    FTimerHandle HungerTimerHandle;
    FTimerHandle ThirstTimerHandle;
    float LastAttackTime;

    UFUNCTION()
    void TickHunger();

    UFUNCTION()
    void TickThirst();
};
