#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Fleeing,
    Socializing,
    Working,
    Resting
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState ActivityType = ENPC_BehaviorState::Working;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Priority = 1.0f;
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsHostile = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    // Daily Schedule System
    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    void AddScheduleEntry(const FNPC_DailySchedule& ScheduleEntry);

    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    void UpdateSchedule(float CurrentTimeOfDay);

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RememberActor(AActor* Actor, float Importance, bool bIsHostile = false);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_Memory GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetActor(AActor* Actor);

    // Perception and Reactions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnPlayerSighted(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnDangerDetected(AActor* DangerSource);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void OnSocialInteraction(AActor* OtherNPC);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    bool IsPlayerInSight() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    int32 CurrentScheduleIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 20;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float FleeDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float SocialDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float RestDuration = 30.0f;

private:
    void UpdateMemoryDecay(float DeltaTime);
    void ExecuteCurrentBehavior(float DeltaTime);
    FVector GetRandomPatrolPoint();
    AActor* FindNearestNPC();
};