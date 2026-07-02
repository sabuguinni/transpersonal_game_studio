#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Rest        UMETA(DisplayName = "Rest"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Cautious    UMETA(DisplayName = "Cautious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Curious     UMETA(DisplayName = "Curious"),
    Timid       UMETA(DisplayName = "Timid"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float TimeOfDayHour = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    ENPC_BehaviorState TargetState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    float Duration = 60.f;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    AActor* Subject = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsHostile = false;
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRange = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ForageRadius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float WalkSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float RunSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MemoryDecayTime = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_PersonalityTrait Personality = ENPC_PersonalityTrait::Cautious;
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
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordThreat(AActor* ThreatActor, float ThreatLevel, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void ForgetOldMemories(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void AddRoutineEntry(const FNPC_DailyRoutineEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "NPC|Routine")
    void EvaluateRoutine(float CurrentHour);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsAwareOfThreat() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetHighestThreatLevel() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Routine")
    TArray<FNPC_DailyRoutineEntry> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryRecord> MemoryRecords;

private:
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;
    FVector HomeLocation = FVector::ZeroVector;
    FVector PatrolTarget = FVector::ZeroVector;
    float StateTimer = 0.f;
    float PatrolWaitTime = 3.f;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickForage(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickAlert(float DeltaTime);
    void ChooseNewPatrolTarget();
};
