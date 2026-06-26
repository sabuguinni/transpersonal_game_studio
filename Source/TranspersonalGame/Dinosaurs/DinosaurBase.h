#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Alert       UMETA(DisplayName = "Alert"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops       UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus       UMETA(DisplayName = "Tsintaosaurus")
};

UENUM(BlueprintType)
enum class EEng_DinoDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Thirst = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 500.f;
};

UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = (Dinosaurs))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Identity ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoDiet Diet = EEng_DinoDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName = NAME_None;

    // ── Combat & Survival Stats ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats Stats;

    // ── Behaviour State ─────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    EEng_DinoState CurrentState = EEng_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float PatrolRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    bool bIsPackAnimal = false;

    // ── Internal helpers ────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    FVector HomeLocation = FVector::ZeroVector;

    float StateTimer = 0.f;

public:
    // ── Public API ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDamageFromAttack(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetDinoState(EEng_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    EEng_DinoState GetDinoState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const { return Stats.CurrentHealth > 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void OnDetectPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    virtual void OnLosePlayer();

protected:
    // ── State machine ───────────────────────────────────────────────────
    virtual void UpdateIdleState(float DeltaTime);
    virtual void UpdatePatrolState(float DeltaTime);
    virtual void UpdateAlertState(float DeltaTime);
    virtual void UpdateChaseState(float DeltaTime);
    virtual void UpdateAttackState(float DeltaTime);
    virtual void UpdateFleeState(float DeltaTime);

    void ScanForPlayer();
    bool IsPlayerInRange(float Range) const;
    float DistanceToPlayer() const;
};
