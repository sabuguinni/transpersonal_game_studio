#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsPackHunter = false;
};

/**
 * ADinosaurBase — Base class for all dinosaur pawns.
 * Provides survival stats, state machine, detection, and combat foundation.
 * All specific dinosaur types (TRex, Raptor, etc.) inherit from this.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Identity ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FText DisplayName;

    // ── Stats ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats Stats;

    // ── State Machine ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EEng_DinoState CurrentState = EEng_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    AActor* CurrentTarget = nullptr;

    // ── Combat ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void TakeDamageFromPlayer(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    float GetHealthPercent() const;

    // ── AI Behaviour ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void SetState(EEng_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void ScanForPlayer();

    // ── Events (override in subclasses) ───────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnStateChanged(EEng_DinoState OldState, EEng_DinoState NewState);
    virtual void OnStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState);

protected:
    // ── Internal timers ───────────────────────────────────────────────────
    float TimeSinceLastScan = 0.0f;
    float ScanInterval = 0.5f;

    float TimeSinceLastAttack = 0.0f;
    float AttackCooldown = 1.5f;

    // ── Roaming ───────────────────────────────────────────────────────────
    FVector HomeLocation;
    FVector RoamTarget;
    float TimeSinceLastRoam = 0.0f;
    float RoamInterval = 5.0f;

    void UpdateRoaming(float DeltaTime);
    void PickNewRoamTarget();
    void UpdateHunting(float DeltaTime);
    void UpdateAttacking(float DeltaTime);
};
