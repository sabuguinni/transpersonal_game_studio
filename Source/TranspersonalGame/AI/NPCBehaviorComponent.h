#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Rest        UMETA(DisplayName = "Rest"),
    Roar        UMETA(DisplayName = "Roar"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bIsWaypoint = true;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsThreat = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Species & State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|State")
    ENPC_BehaviorState DefaultState = ENPC_BehaviorState::Patrol;

    // --- Detection ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float SightAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
    float FleeHealthThreshold = 0.25f;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolRadius = 5000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentPatrolIndex = 0;

    // --- Memory ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FNPC_MemoryEntry PlayerMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDuration = 15.0f;

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float ChaseSpeedMultiplier = 1.5f;

    // --- Runtime ---
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Runtime")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Runtime")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Runtime")
    float TimeSinceLastPlayerSight = 0.0f;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerDetected(AActor* Player, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TakeDamageNPC(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsPlayerInSightCone(AActor* Player) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsPlayerInAttackRange(AActor* Player) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void GenerateRandomPatrolPoints(int32 Count = 4);

private:
    void UpdateBehaviorTick(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void EvaluateStateTransition();

    UPROPERTY()
    AActor* TrackedPlayer = nullptr;

    float PatrolWaitTimer = 0.0f;
    bool bWaitingAtPatrolPoint = false;
    FVector HomeLocation = FVector::ZeroVector;
};
