#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ============================================================
// ENUMS — NPC_ prefixed, global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_AlertLevel : uint8
{
    Unaware      UMETA(DisplayName = "Unaware"),
    Curious      UMETA(DisplayName = "Curious"),
    Suspicious   UMETA(DisplayName = "Suspicious"),
    Alarmed      UMETA(DisplayName = "Alarmed"),
    Panicked     UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class ENPC_RoutinePhase : uint8
{
    Sleeping      UMETA(DisplayName = "Sleeping"),
    Waking        UMETA(DisplayName = "Waking"),
    Foraging      UMETA(DisplayName = "Foraging"),
    Resting       UMETA(DisplayName = "Resting"),
    Socializing   UMETA(DisplayName = "Socializing"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating"),
    Patrolling    UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class ENPC_SocialRelation : uint8
{
    Stranger    UMETA(DisplayName = "Stranger"),
    Acquainted  UMETA(DisplayName = "Acquainted"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Trusted     UMETA(DisplayName = "Trusted"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Feared      UMETA(DisplayName = "Feared")
};

// ============================================================
// STRUCTS — NPC_ prefixed, global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsVisible = false;
};

USTRUCT(BlueprintType)
struct FNPC_SocialRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    AActor* OtherActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    ENPC_SocialRelation Relation = ENPC_SocialRelation::Stranger;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    float TrustScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Social")
    float LastInteractionTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    ENPC_RoutinePhase Phase = ENPC_RoutinePhase::Resting;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float StartHour = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    float EndHour = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Schedule")
    bool bHasFixedLocation = false;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Alert System ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Alert", meta = (AllowPrivateAccess = "true"))
    ENPC_AlertLevel CurrentAlertLevel = ENPC_AlertLevel::Unaware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertThresholdCurious = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertThresholdSuspicious = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertThresholdAlarmed = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Alert")
    float AlertThresholdPanicked = 90.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Alert")
    float CurrentAlertScore = 0.0f;

    // ---- Routine System ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine")
    ENPC_RoutinePhase CurrentRoutinePhase = ENPC_RoutinePhase::Resting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyScheduleEntry> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine")
    float CurrentGameHour = 8.0f;

    // ---- Memory System ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxThreatMemories = 5;

    // ---- Social System ----
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Social")
    TArray<FNPC_SocialRecord> SocialRecords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Social")
    float SocialRadius = 800.0f;

    // ---- Survival Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Survival")
    float Fear = 0.0f;

    // ---- UFUNCTIONs ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void AddThreat(AActor* ThreatSource, float ThreatAmount);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    void ClearThreat(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "NPC|Alert")
    ENPC_AlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetRoutinePhase(ENPC_RoutinePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    ENPC_RoutinePhase GetCurrentPhase() const { return CurrentRoutinePhase; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    FNPC_DailyScheduleEntry GetScheduledPhaseForHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordThreatSighting(AActor* Threat, FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    bool HasActiveThreatMemory() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    FNPC_ThreatMemory GetMostDangerousThreat() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    void UpdateSocialRelation(AActor* OtherActor, ENPC_SocialRelation NewRelation, float TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "NPC|Social")
    ENPC_SocialRelation GetRelationTo(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Survival")
    bool NeedsFoodOrWater() const;

private:
    void UpdateAlertLevel(float DeltaTime);
    void UpdateRoutinePhase(float DeltaTime);
    void DecayThreatMemories(float DeltaTime);
    void UpdateSurvivalStats(float DeltaTime);

    float ElapsedTime = 0.0f;
};
