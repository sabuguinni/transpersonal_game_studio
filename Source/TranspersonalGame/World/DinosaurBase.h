#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
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
};

USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float CurrentHealth = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackDamage = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float DetectionRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float RunSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float HungerLevel = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    bool bIsCarnivore = true;
};

/**
 * ADinosaurBase — Base class for all dinosaur actors in the prehistoric survival game.
 * Inherits from ACharacter to use UE5 CharacterMovementComponent for locomotion.
 * All dinosaur species inherit from this class.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === SPECIES & IDENTITY ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    FName DinoName = NAME_None;

    // === STATS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    FEng_DinoStats DinoStats;

    // === STATE MACHINE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|State")
    EEng_DinoState CurrentState = EEng_DinoState::Idle;

    // === PATROL ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Patrol")
    float PatrolRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Patrol")
    float PatrolWaitTime = 3.f;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void TakeDinoHit(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    void SetDinoState(EEng_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino|State")
    EEng_DinoState GetDinoState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dino|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|Stats")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Dino|AI")
    void StopPatrol();

protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Dino|Events")
    void OnDinoStateChanged(EEng_DinoState OldState, EEng_DinoState NewState);
    virtual void OnDinoStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dino|Events")
    void OnDinoDeath();
    virtual void OnDinoDeath_Implementation();

    // Internal patrol logic
    void UpdatePatrol(float DeltaTime);
    FVector GetRandomPatrolPoint() const;

    float PatrolWaitTimer = 0.f;
    bool bWaitingAtPatrolPoint = false;
    FVector PatrolOrigin = FVector::ZeroVector;
    FVector CurrentPatrolTarget = FVector::ZeroVector;
};
