#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTreeManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0,
    Patrolling = 1,
    Investigating = 2,
    Fleeing = 3,
    Socializing = 4,
    Working = 5,
    Sleeping = 6,
    Hunting = 7,
    Gathering = 8
};

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Leader = 0,
    Hunter = 1,
    Gatherer = 2,
    Crafter = 3,
    Guard = 4,
    Elder = 5,
    Child = 6,
    Shaman = 7
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        RelatedActor = nullptr;
        Importance = 0.0f;
        TimeStamp = 0.0f;
        EventDescription = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    FNPC_SocialRelationship()
    {
        TargetActor = nullptr;
        RelationshipValue = 0.0f;
        Trust = 0.0f;
        Fear = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorTreeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorTreeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* SocialBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBlackboardComponent* NPCBlackboard;

    // NPC State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPC_SocialRole SocialRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float Alertness;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float CommunicationCooldown;

    // Daily Routines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routines")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routines")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routines")
    FVector WorkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routines")
    float DailyRoutineTimer;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(FVector Location, AActor* Actor, float Importance, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ProcessMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelationship(AActor* TargetActor, float RelationshipChange, float TrustChange, float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelationship GetSocialRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Routines")
    void ExecuteDailyRoutine(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void OnPerceptionUpdated(AActor* Actor, float Stimulus);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendSocialSignal(const FString& Message, float Range);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveSocialSignal(AActor* Sender, const FString& Message);

private:
    void UpdateNeeds(float DeltaTime);
    void ProcessSocialInteractions(float DeltaTime);
    void ManageMemoryCapacity();
    AActor* FindNearestActorOfType(TSubclassOf<AActor> ActorClass, float MaxDistance);
    bool IsInSocialRange(AActor* TargetActor);
};