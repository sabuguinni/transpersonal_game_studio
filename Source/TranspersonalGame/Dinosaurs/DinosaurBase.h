#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roaming     UMETA(DisplayName = "Roaming"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEng_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Mass = 500.0f;
};

/**
 * Base class for all dinosaurs in the prehistoric survival game.
 * Provides health, AI state machine, detection, and attack logic.
 * All specific dinosaur types (TRex, Raptor, Brachiosaurus) inherit from this.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Combat ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    // ---- AI State ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetDinosaurState(EEng_DinosaurState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    EEng_DinosaurState GetDinosaurState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    AActor* FindNearestThreat() const;

    // ---- Detection ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Detection")
    bool CanDetectActor(AActor* TargetActor) const;

    // ---- Species info ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FString SpeciesName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinosaurDiet Diet = EEng_DinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats Stats;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    EEng_DinosaurState CurrentState = EEng_DinosaurState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    // Time accumulator for roaming behaviour
    float RoamTimer = 0.0f;
    float RoamInterval = 5.0f;

    // Internal AI tick
    void UpdateAI(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandleRoamingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);

    virtual void OnDeath();
};
