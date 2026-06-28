// NPCBehaviorComponent.h
// NPC Behavior Agent — Agent #11
// Transpersonal Game Studio
// Dinosaur survival game — NPC behavior, daily routines, memory, social dynamics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — NPC behavioral states
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Forage          UMETA(DisplayName = "Forage"),
    Rest            UMETA(DisplayName = "Rest"),
    Alert           UMETA(DisplayName = "Alert"),
    Flee            UMETA(DisplayName = "Flee"),
    Seek            UMETA(DisplayName = "Seek"),
    Interact        UMETA(DisplayName = "Interact"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    Suspicious      UMETA(DisplayName = "Suspicious"),
    Alarmed         UMETA(DisplayName = "Alarmed"),
    Panicked        UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Loner           UMETA(DisplayName = "Loner"),
    Scout           UMETA(DisplayName = "Scout"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Guard           UMETA(DisplayName = "Guard"),
    Elder           UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ENPC_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

// ============================================================
// STRUCTS — NPC memory and schedule
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    AActor* Subject = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel PerceivedThreat = ENPC_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsDangerous = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsKnownLocation = false;
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_TimeOfDay TimeSlot = ENPC_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_BehaviorState DesiredBehavior = ENPC_BehaviorState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    float Duration = 300.0f;  // seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    bool bUseRandomLocation = true;
};

USTRUCT(BlueprintType)
struct FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fatigue = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 0.0f;

    bool IsHungry() const { return Hunger < 30.0f; }
    bool IsThirsty() const { return Thirst < 30.0f; }
    bool IsExhausted() const { return Fatigue > 80.0f; }
    bool IsPanicking() const { return Fear > 70.0f; }
};

// ============================================================
// COMPONENT — Core NPC behavior driver
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateBehaviorTick(float DeltaTime);

    // ---- Memory System ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RegisterThreat(AActor* ThreatActor, FVector Location, ENPC_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasMemoryOf(AActor* Subject) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FNPC_MemoryEntry GetMemoryOf(AActor* Subject) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void DecayMemories(float DeltaTime);

    // ---- Daily Schedule ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    void SetDailySchedule(const TArray<FNPC_DailyScheduleEntry>& NewSchedule);

    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    void TickSchedule(ENPC_TimeOfDay CurrentTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
    FNPC_DailyScheduleEntry GetCurrentScheduleEntry() const;

    // ---- Survival Needs ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    FNPC_SurvivalNeeds GetSurvivalNeeds() const { return SurvivalNeeds; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    void SetFear(float NewFear);

    // ---- Threat Response ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void RespondToThreat(AActor* ThreatSource, float ThreatDistance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    FVector CalculateFleeDirection(FVector ThreatLocation) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Threat")
    void AlertNearbyNPCs(float AlertRadius = 2000.0f);

    // ---- Social ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void SetSocialRole(ENPC_SocialRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    ENPC_SocialRole GetSocialRole() const { return SocialRole; }

    // ---- Perception ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    bool CanSeeActor(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    bool CanHearActor(AActor* Target, float NoiseLevel = 1.0f) const;

    // ---- Debug ----
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "NPC|Debug")
    void DrawDebugBehaviorState() const;

public:
    // ---- Exposed Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_SocialRole SocialRole = ENPC_SocialRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SightRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 5.0f;  // units per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryForgetThreshold = 120.0f;  // seconds until forgotten

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bHasFixedPatrolRoute = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
    TArray<FNPC_DailyScheduleEntry> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    FNPC_SurvivalNeeds SurvivalNeeds;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> MemoryBank;

private:
    // ---- Internal State ----
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    int32 CurrentPatrolIndex = 0;
    int32 CurrentScheduleIndex = 0;
    float StateTimer = 0.0f;
    float ScheduleTimer = 0.0f;
    FVector HomeLocation = FVector::ZeroVector;
    FVector CurrentPatrolTarget = FVector::ZeroVector;

    // ---- Internal Behavior Handlers ----
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleForageState(float DeltaTime);
    void HandleRestState(float DeltaTime);
    void HandleAlertState(float DeltaTime);
    void HandleFleeState(float DeltaTime);

    FVector GetNextPatrolPoint() const;
    bool HasReachedLocation(FVector Target, float Tolerance = 150.0f) const;
    void ChooseNewBehaviorFromNeeds();
};
