#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// Enums — NPC_-prefixed to avoid collisions across agents
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek"),
    Alert       UMETA(DisplayName = "Alert"),
    Rest        UMETA(DisplayName = "Rest"),
    Socialise   UMETA(DisplayName = "Socialise"),
    Dead        UMETA(DisplayName = "Dead")
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

UENUM(BlueprintType)
enum class ENPC_DailyRoutineSlot : uint8
{
    EarlyMorning    UMETA(DisplayName = "EarlyMorning"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Evening         UMETA(DisplayName = "Evening"),
    Night           UMETA(DisplayName = "Night")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceObserved = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPlayer = false;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsPredator = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_DailyRoutineSlot TimeSlot = ENPC_DailyRoutineSlot::Morning;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState DesiredState = ENPC_BehaviorState::Patrol;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 120.0f;
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    AActor* RelatedActor = nullptr;

    // -1.0 = enemy, 0.0 = neutral, 1.0 = ally
    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    float TrustLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    bool bIsTribemate = false;
};

// ============================================================
// Component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void EvaluateThreats();

    // ── Memory ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore, bool bIsPredator);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetStaleMemories(float MaxAge);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FNPC_MemoryEntry GetHighestThreatMemory() const;

    // ── Daily Routine ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void UpdateDailyRoutine(float GameHour);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    ENPC_DailyRoutineSlot GetCurrentTimeSlot(float GameHour) const;

    // ── Social ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void UpdateTrustLevel(AActor* OtherActor, float Delta);

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    float GetTrustLevel(AActor* OtherActor) const;

    // ── Properties ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryMaxAge = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineEntry> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryBank;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Social")
    TArray<FNPC_SocialRelation> SocialRelations;

private:
    void TickMemoryDecay(float DeltaTime);
    void TickBehaviorState(float DeltaTime);

    FVector HomeLocation = FVector::ZeroVector;
    float StateTimer = 0.0f;
};
