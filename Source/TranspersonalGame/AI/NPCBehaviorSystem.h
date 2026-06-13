#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Core/Shared/SharedTypes.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Fleeing,
    Socializing,
    Working,
    Resting,
    Hunting,
    Gathering
};

UENUM(BlueprintType)
enum class ENPC_Personality : uint8
{
    Aggressive,
    Cautious,
    Curious,
    Social,
    Solitary,
    Leader,
    Follower,
    Cowardly,
    Brave
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType;

    FNPC_Memory()
    {
        LastSeenLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.0f;
        EventType = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Affinity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    FNPC_Relationship()
    {
        TargetActor = nullptr;
        Affinity = 0.0f;
        Trust = 0.0f;
        Fear = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_Personality PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AwarenessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float SocialRadius;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate;

    // Relationship system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Relationships")
    TArray<FNPC_Relationship> Relationships;

    // Daily routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    float PatrolSpeed;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(FVector Location, FString EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationship(AActor* Target, float AffinityChange, float TrustChange, float FearChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetNearestFriend();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StopPatrol();

private:
    // Internal behavior functions
    void UpdateBehavior(float DeltaTime);
    void ProcessMemories(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void ScanEnvironment();
    void ReactToPlayer();
    void ReactToDinosaurs();

    // Timers
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle MemoryProcessTimer;
    FTimerHandle EnvironmentScanTimer;

    // Internal state
    float LastBehaviorUpdate;
    float LastMemoryProcess;
    FVector HomeLocation;
    bool bIsPatrolling;
    AActor* CurrentTarget;
};