#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

// ─── Enums (global scope — UHT requirement) ──────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Socialise   UMETA(DisplayName = "Socialise"),
    Sleep       UMETA(DisplayName = "Sleep"),
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

// ─── Structs (global scope — UHT requirement) ─────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float GameHourStart = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    float GameHourEnd = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetState = ENPC_BehaviorState::Sleep;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FVector Destination = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Routine")
    FString ActivityDescription = TEXT("Sleeping");
};

// ─── Delegate ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPC_OnStateChanged,
    ENPC_BehaviorState, OldState,
    ENPC_BehaviorState, NewState);

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void TransitionToState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_ThreatLevel GetCurrentThreatLevel() const;

    // ── Memory ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void AddThreatMemory(AActor* ThreatActor, FVector LastKnownLocation, ENPC_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void PurgeExpiredMemories();

    // ── Navigation Helpers ─────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "NPC|Navigation")
    FVector GetFleeDestination() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Navigation")
    FVector GetPatrolDestination() const;

    // ── Daily Routine ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void SetDailyRoutine(const TArray<FNPC_DailyRoutineSlot>& NewRoutine);

    // ── Detection ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "NPC|Detection")
    bool IsActorInDetectionRange(AActor* Target) const;

    // ── Delegate ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
    FNPC_OnStateChanged OnStateChanged;

    // ── Config Properties ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bIsNocturnal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bCanFlee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bCanSocialise;

    // ── Runtime State ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> ThreatMemory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineSlot> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Navigation")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Navigation")
    FVector RoutineDestination;

private:
    // ── Internal State Machine Methods ─────────────────────────────────────
    void RunStateMachine(float DeltaTime);
    void State_Idle(float DeltaTime);
    void State_Patrol(float DeltaTime);
    void State_Forage(float DeltaTime);
    void State_Flee(float DeltaTime);
    void State_Alert(float DeltaTime);
    void State_Socialise(float DeltaTime);
    void State_Sleep(float DeltaTime);

    void UpdateThreatLevel();
    void TickDailyRoutine();

    // ── Internal Timers ────────────────────────────────────────────────────
    float IdleTimer = 0.0f;
    float AlertTimer = 0.0f;
    float ForageTimer = 0.0f;
    float SocialTimer = 0.0f;
    int32 CurrentRoutineIndex = 0;
    bool bHomeLocationSet = false;

    FTimerHandle RoutineTimerHandle;
};
