// DinosaurBase.h
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Prehistoric survival game — dinosaur base class
// All dinosaurs inherit from this class via ACharacter

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardData;
class UAIPerceptionComponent;
class UWidgetComponent;

// ─── Enums (global scope, unique prefixed names) ─────────────────────────────

UENUM(BlueprintType)
enum class ECore_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor          UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactylus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    COUNT           UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECore_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECore_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CapsuleRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CapsuleHalfHeight = 90.0f;
};

USTRUCT(BlueprintType)
struct FCore_DinosaurSpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ECore_DinosaurSpecies Species = ECore_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ECore_DinosaurDiet Diet = ECore_DinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FCore_DinosaurStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanFly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanSwim = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString CommonName = TEXT("Unknown Dinosaur");
};

// ─── DinosaurBase Class ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Overrides ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species Configuration ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    ECore_DinosaurSpecies Species = ECore_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FCore_DinosaurSpeciesConfig SpeciesConfig;

    // ── Live Stats ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FCore_DinosaurStats Stats;

    // ── Behavior State ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    ECore_DinosaurBehaviorState BehaviorState = ECore_DinosaurBehaviorState::Idle;

    // ── AI Assets ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    UBehaviorTree* BehaviorTree = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    UBlackboardData* BlackboardData = nullptr;

    // ── Current Target ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    AActor* CurrentTarget = nullptr;

    // ── Pack Reference ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Pack")
    TArray<ADinosaurBase*> PackMembers;

    // ── Public Methods ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(ECore_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    ECore_DinosaurBehaviorState GetBehaviorState() const { return BehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void JoinPack(ADinosaurBase* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Species")
    void InitializeSpecies(ECore_DinosaurSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void ConsumeStamina(float Amount);

protected:
    // ── Internal Timers ────────────────────────────────────────────────────
    FTimerHandle StaminaRegenTimer;
    FTimerHandle HungerDecayTimer;
    FTimerHandle BehaviorUpdateTimer;

    // ── Internal Methods ───────────────────────────────────────────────────
    void OnDeath();
    void RegenStamina();
    void DecayHunger();
    void UpdateBehavior();
    void ApplySpeciesConfig(const FCore_DinosaurSpeciesConfig& Config);
    FCore_DinosaurSpeciesConfig BuildSpeciesConfig(ECore_DinosaurSpecies InSpecies) const;

    // ── State flags ────────────────────────────────────────────────────────
    bool bIsDead = false;
    bool bIsAttacking = false;
    float LastAttackTime = 0.0f;
    float AttackCooldown = 1.5f;
};
