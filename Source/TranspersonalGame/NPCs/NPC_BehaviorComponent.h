#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPC_BehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert")
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector LastKnownThreatLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TimeSinceThreatSeen;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float LastFoodTime;

    FNPC_Memory()
    {
        LastKnownThreatLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        TimeSinceThreatSeen = 0.0f;
        LastFoodTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior state
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float AlertRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float FleeRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    // Memory system
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FNPC_Memory NPCMemory;

    // Social behavior
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    TArray<AActor*> TribeMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    AActor* CurrentTarget;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void DetectThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemory(FVector ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void AddTribeMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void BroadcastAlert(const FString& AlertMessage);

private:
    // Internal behavior logic
    void ProcessIdleBehavior(float DeltaTime);
    void ProcessPatrolBehavior(float DeltaTime);
    void ProcessAlertBehavior(float DeltaTime);
    void ProcessFleeingBehavior(float DeltaTime);

    // Timers
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle MemoryDecayTimer;

    void UpdateBehaviorLogic();
    void DecayMemory();

    // Current patrol index
    int32 CurrentPatrolIndex;
    
    // Time tracking
    float StateChangeTime;
    float LastBehaviorUpdate;
};