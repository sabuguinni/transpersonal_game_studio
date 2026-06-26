// DinosaurBase.h
// Transpersonal Game Studio — Core Systems Programmer (Agent #03)
// Prehistoric survival game — dinosaur base pawn with biome-aware behavior

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────────

UENUM(BlueprintType)
enum class EDino_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class EDino_DietType : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

UENUM(BlueprintType)
enum class EDino_SizeClass : uint8
{
    Small       UMETA(DisplayName = "Small"),   // Raptor-class
    Medium      UMETA(DisplayName = "Medium"),  // Triceratops-class
    Large       UMETA(DisplayName = "Large"),   // T-Rex-class
    Colossal    UMETA(DisplayName = "Colossal"),// Brachiosaurus-class
};

// ─── Structs (global scope — UHT requirement) ─────────────────────────────────

USTRUCT(BlueprintType)
struct FDino_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDino_DietType DietType = EDino_DietType::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDino_SizeClass SizeClass = EDino_SizeClass::Large;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float MaxWalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float MaxRunSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FDino_CombatStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float CurrentHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float Aggression = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown = 1.5f;
};

// ─── ADinosaurBase ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

public:
    // ── Species Configuration ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FDino_SpeciesTraits SpeciesTraits;

    // ── Combat State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat")
    FDino_CombatStats CombatStats;

    // ── Behavior State ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EDino_BehaviorState CurrentBehaviorState = EDino_BehaviorState::Idle;

    // ── Detection Sphere ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    // ── Attack Sphere ──────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* AttackSphere;

    // ── Patrol Origin ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    FVector PatrolOrigin;

    // ── Current Target ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    AActor* CurrentTarget;

    // ── Biome Danger Level (set by BiomeManager query) ─────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Biome")
    float BiomeDangerLevel = 0.5f;

    // ── Tick interval for behavior update (seconds) ────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Performance")
    float BehaviorTickInterval = 0.5f;

    // ── Public API ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void ApplyDamage(float Damage, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EDino_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void AlertToThreat(AActor* Threat, float ThreatStrength);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    EDino_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Biome")
    void UpdateBiomeDangerLevel(float NewDangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    FVector GetRandomPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void OnDeath();

    // ── Detection callbacks ────────────────────────────────────────────────
    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                               bool bFromSweep, const FHitResult& SweepResult);

private:
    float BehaviorTickAccumulator = 0.0f;
    float HungerDecayRate = 2.0f;   // per second
    float AggressionDecayRate = 5.0f; // per second when no threat
};
