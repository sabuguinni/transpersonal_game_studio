#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek"),
    Rest        UMETA(DisplayName = "Rest"),
    Alert       UMETA(DisplayName = "Alert"),
    Interact    UMETA(DisplayName = "Interact")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
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
    bool bLookAround = true;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsPlayer = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // --- Threat Assessment ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore, bool bIsPlayer);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void ClearThreat();

    UFUNCTION(BlueprintPure, Category = "NPC|Threat")
    ENPC_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    // --- Patrol ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolPoints(const TArray<FNPC_PatrolPoint>& Points);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvancePatrolPoint();

    // --- Memory ---
    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasMemoryOfPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float MaxAge);

    // --- Daily Routine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float TimeOfDay);

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeSpeed = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryLog;

private:
    int32 CurrentPatrolIndex = 0;
    float PatrolWaitTimer = 0.0f;
    float ThreatDecayTimer = 0.0f;

    AActor* ActiveThreatActor = nullptr;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAlert(float DeltaTime);
    void DecayThreat(float DeltaTime);
    ENPC_ThreatLevel ScoreToThreatLevel(float Score) const;
};
