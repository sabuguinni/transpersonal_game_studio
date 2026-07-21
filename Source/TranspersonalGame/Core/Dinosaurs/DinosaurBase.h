// DinosaurBase.h
// Core Systems Programmer — Agent #3
// Base class for all dinosaur pawns in the prehistoric survival game.
// Provides: territory system, aggression states, patrol behavior, survival stats.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinosaurBase.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinoAggressionState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class EDinoSpeciesType : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Generic             UMETA(DisplayName = "Generic")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FDinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 500.f;
};

USTRUCT(BlueprintType)
struct FDinoPatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float WaitTime = 2.f;
};

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Combat ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsInAttackRange(AActor* Target) const;

    // ── Aggression ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetAggressionState(EDinoAggressionState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Behavior")
    EDinoAggressionState GetAggressionState() const { return AggressionState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void AlertToThreat(AActor* ThreatActor);

    // ── Territory ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Territory")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    void SetTerritoryCenter(FVector Center);

    // ── Patrol ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void AddPatrolPoint(FVector Location, float WaitTime = 2.f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void StopPatrol();

    // ── Getters ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const { return bIsAlive; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    EDinoSpeciesType GetSpeciesType() const { return SpeciesType; }

protected:
    // ── Components ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TerritorySphere;

    // ── Config ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    EDinoSpeciesType SpeciesType = EDinoSpeciesType::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    FDinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    bool bIsHerbivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Config")
    int32 PackSize = 1;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State",
              meta = (AllowPrivateAccess = "true"))
    EDinoAggressionState AggressionState = EDinoAggressionState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State",
              meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|State",
              meta = (AllowPrivateAccess = "true"))
    FVector TerritoryCenter;

    // ── Patrol ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<FDinoPatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Patrol",
              meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Patrol",
              meta = (AllowPrivateAccess = "true"))
    bool bIsPatrolling = false;

    // ── Timers ─────────────────────────────────────────────────────────────
    FTimerHandle PatrolWaitTimer;
    FTimerHandle HungerTimer;
    FTimerHandle AttackCooldownTimer;

    bool bCanAttack = true;

    // ── Internal methods ───────────────────────────────────────────────────
    virtual void Die();
    void TickPatrol(float DeltaTime);
    void TickAggression(float DeltaTime);
    void MoveToNextPatrolPoint();
    void OnPatrolWaitComplete();
    void DrainHunger();
    void EnableAttack();

    // ── Overlap callbacks ──────────────────────────────────────────────────
    UFUNCTION()
    void OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComp,
                                   AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp,
                                   int32 OtherBodyIndex,
                                   bool bFromSweep,
                                   const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex);
};
