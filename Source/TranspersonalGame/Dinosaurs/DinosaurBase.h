#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus")
};

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HungerDecayRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsPackHunter = false;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void TakeDamageFromPlayer(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    // AI State
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetDinoState(EEng_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    EEng_DinoState GetDinoState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetPatrolTarget(FVector NewTarget);

    // Sensing callbacks
    UFUNCTION()
    void OnSeePawn(APawn* SeenPawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    EEng_DinoSpecies GetSpecies() const { return Species; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UPawnSensingComponent* PawnSensing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    EEng_DinoSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    FEng_DinoStats DinoStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EEng_DinoState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector PatrolTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    float AttackCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    float FleeHealthThreshold = 0.2f;

    // AI tick methods
    virtual void TickIdle(float DeltaTime);
    virtual void TickPatrol(float DeltaTime);
    virtual void TickChase(float DeltaTime);
    virtual void TickAttack(float DeltaTime);
    virtual void TickFlee(float DeltaTime);

    void ChooseNewPatrolPoint();
    bool IsTargetInAttackRange() const;
    void Die();

private:
    FVector SpawnLocation;
    float StateTimer = 0.0f;
};
