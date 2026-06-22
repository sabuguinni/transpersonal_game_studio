#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ECore_DinoState — Behaviour state machine for all dinosaurs
// ============================================================
UENUM(BlueprintType)
enum class ECore_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// ============================================================
// ECore_DinoSize — Rough size category used for AI decisions
// ============================================================
UENUM(BlueprintType)
enum class ECore_DinoSize : uint8
{
    Small       UMETA(DisplayName = "Small"),    // < 2m — Compsognathus-class
    Medium      UMETA(DisplayName = "Medium"),   // 2-5m — Raptor-class
    Large       UMETA(DisplayName = "Large"),    // 5-12m — Carnotaurus-class
    Massive     UMETA(DisplayName = "Massive")   // >12m  — T-Rex / Brachiosaurus-class
};

// ============================================================
// FCore_DinoStats — Runtime survival stats
// ============================================================
USTRUCT(BlueprintType)
struct FCore_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.f;   // 0 = starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.f;  // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRadius = 200.f;      // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.f;         // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.f;          // cm/s
};

// ============================================================
// FCore_PatrolPoint — Single waypoint in a patrol route
// ============================================================
USTRUCT(BlueprintType)
struct FCore_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    float WaitTimeSeconds = 2.f;
};

// ============================================================
// ADinosaurBase — Base class for all playable/AI dinosaurs
// ============================================================
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ---- ACharacter overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ECore_DinoSize SizeCategory = ECore_DinoSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPackHunter = false;

    // ---- Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FCore_DinoStats Stats;

    // ---- State machine ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    ECore_DinoState CurrentState = ECore_DinoState::Idle;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetDinoState(ECore_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    ECore_DinoState GetDinoState() const { return CurrentState; }

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<FCore_PatrolPoint> PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    bool bLoopPatrol = true;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void StopPatrol();

    // ---- Combat ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void ApplyMeleeDamage(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsTargetInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsTargetInDetectionRange(AActor* Target) const;

    // ---- Health ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Health")
    void HealDinosaur(float Amount);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const { return Stats.Health > 0.f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    // ---- Events (override in subclasses) ----
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoStateChanged(ECore_DinoState OldState, ECore_DinoState NewState);
    virtual void OnDinoStateChanged_Implementation(ECore_DinoState OldState, ECore_DinoState NewState);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoDeath();
    virtual void OnDinoDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnTargetDetected(AActor* DetectedTarget);
    virtual void OnTargetDetected_Implementation(AActor* DetectedTarget);

protected:
    // ---- Internal patrol state ----
    int32 CurrentPatrolIndex = 0;
    float PatrolWaitTimer = 0.f;
    bool bWaitingAtPatrolPoint = false;
    bool bPatrolActive = false;

    // ---- Tick helpers ----
    void TickPatrol(float DeltaTime);
    void TickHunger(float DeltaTime);
    void AdvanceToNextPatrolPoint();
    void MoveToPatrolPoint(const FCore_PatrolPoint& Point);

    // ---- Cached AI controller ----
    UPROPERTY()
    AAIController* CachedAIController = nullptr;
};
