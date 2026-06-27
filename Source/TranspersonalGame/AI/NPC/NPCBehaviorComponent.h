#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Seek        UMETA(DisplayName = "Seek"),
    Interact    UMETA(DisplayName = "Interact"),
    Rest        UMETA(DisplayName = "Rest"),
    Alert       UMETA(DisplayName = "Alert")
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
enum class ENPC_DailyPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatScore = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString Tag = TEXT("");
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitDuration = 2.f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = false;
};

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_DailyPhase CurrentPhase = ENPC_DailyPhase::Morning;

    // ── Patrol ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol",
              meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float PatrolAcceptanceRadius = 150.f;

    // ── Memory ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory",
              meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayRate = 0.05f;

    // ── Awareness ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float SightRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float HearingRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float FleeThresholdDistance = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Awareness")
    float AlertCooldown = 5.f;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(FVector ThreatLocation, float ThreatScore, const FString& ThreatTag);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateDailyPhase(float GameHour);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState EvaluateNextState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearExpiredMemories(float CurrentTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool HasActiveMemoryWithTag(const FString& Tag) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    float GetHighestThreatScore() const;

private:
    float TimeSinceLastAlert = 0.f;
    float StateTimer = 0.f;

    void TickPatrol(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickIdle(float DeltaTime);
    void DecayMemories(float DeltaTime);
};
