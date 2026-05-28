#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class EAI_NPCEmotionalState_A78 : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Happy       UMETA(DisplayName = "Happy"),
    Angry       UMETA(DisplayName = "Angry"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Hostile     UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class ENPCActivity : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Trading     UMETA(DisplayName = "Trading"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FAI_NPCMemoryEntry_A78
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelationshipValue; // -100 to 100 (hostile to friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_NPCEmotionalState_A78 LastEmotionalResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> InteractionHistory;

    FAI_NPCMemoryEntry_A78()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        RelationshipValue = 0.0f;
        LastInteractionTime = 0.0f;
        LastEmotionalResponse = EAI_NPCEmotionalState_A78::Neutral;
    }
};

USTRUCT(BlueprintType)
struct FNPCRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duration in hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority; // Higher values take precedence

    FNPCRoutineEntry()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        Activity = ENPCActivity::Idle;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCEmotionalStateChanged, EAI_NPCEmotionalState_A78, OldState, EAI_NPCEmotionalState_A78, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCActivityChanged, ENPCActivity, OldActivity, ENPCActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNPCInteraction, AActor*, OtherActor, float, RelationshipChange, FString, InteractionType);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAI_NPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAI_NPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCRole; // Hunter, Gatherer, Shaman, Warrior, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    int32 NPCAge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float BaseAggressiveness; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float BaseFriendliness; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float BaseCuriosity; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float BaseFearfulness; // 0-1

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    EAI_NPCEmotionalState_A78 CurrentEmotionalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    ENPCActivity CurrentActivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    float CurrentStress; // 0-1

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    float CurrentEnergy; // 0-1

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    float CurrentHunger; // 0-1

    // Memory System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FAI_NPCMemoryEntry_A78> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate; // How fast relationships decay over time

    // Daily Routine System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FNPCRoutineEntry> DailyRoutine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Routine")
    FNPCRoutineEntry CurrentRoutineEntry;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<AActor*> TribeMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    AActor* TribalLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float SocialRadius; // How far NPC can detect other actors

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "NPC Events")
    FOnNPCEmotionalStateChanged OnEmotionalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC Events")
    FOnNPCActivityChanged OnActivityChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC Events")
    FOnNPCInteraction OnInteraction;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(EAI_NPCEmotionalState_A78 NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetActivity(ENPCActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RememberActor(AActor* Actor, float RelationshipChange, const FString& InteractionType);

// [UHT-FIX]     UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FAI_NPCMemoryEntry_A78* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float GetRelationshipWith(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void UpdateCurrentRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    FNPCRoutineEntry GetCurrentTimeRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    TArray<AActor*> GetNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ReactToActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void UpdateBlackboardValues();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    float GetCurrentTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    bool IsNightTime();

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void ModifyStress(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void ModifyEnergy(float Amount);

    UFUNCTION(BlueprintCallable, Category = "NPC Utility")
    void ModifyHunger(float Amount);

private:
    // Internal timers
    FTimerHandle RoutineUpdateTimer;
    FTimerHandle MemoryDecayTimer;
    FTimerHandle SocialUpdateTimer;
    FTimerHandle NeedsUpdateTimer;

    // Internal functions
    void UpdateMemoryDecay();
    void UpdateSocialAwareness();
    void UpdateBasicNeeds();
    void ProcessEmotionalDecay();

    // AI Controller reference
    UPROPERTY()
    class AAIController* AIController;
};