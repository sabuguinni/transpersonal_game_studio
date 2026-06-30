#pragma once

// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Behavior state machine for prehistoric survival NPCs

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle          UMETA(DisplayName = "Idle"),
    Patrol        UMETA(DisplayName = "Patrol"),
    Gather        UMETA(DisplayName = "Gather"),
    Flee          UMETA(DisplayName = "Flee"),
    Seek          UMETA(DisplayName = "Seek"),
    Investigate   UMETA(DisplayName = "Investigate"),
    Interact      UMETA(DisplayName = "Interact"),
    Dead          UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None     UMETA(DisplayName = "None"),
    Low      UMETA(DisplayName = "Low"),
    Medium   UMETA(DisplayName = "Medium"),
    High     UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ENPC_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Morning UMETA(DisplayName = "Morning"),
    Midday  UMETA(DisplayName = "Midday"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastThreat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bHasSeenPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float FearLevel = 0.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void UpdateThreatLevel(ENPC_ThreatLevel NewThreat, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnTimeOfDayChanged(ENPC_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetFearLevel() const { return Memory.FearLevel; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ThreatDetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    FNPC_BehaviorMemory Memory;

private:
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateFleeBehavior(float DeltaTime);
    void DecayMemory(float DeltaTime);

    float StateTimer = 0.0f;
    FVector PatrolOrigin = FVector::ZeroVector;
};
