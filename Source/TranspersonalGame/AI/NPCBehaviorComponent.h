#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Resting         UMETA(DisplayName = "Resting"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Following       UMETA(DisplayName = "Following"),
    Investigating   UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class ENPCRole : uint8
{
    Shaman          UMETA(DisplayName = "Shaman"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Child           UMETA(DisplayName = "Child"),
    Elder           UMETA(DisplayName = "Elder"),
    Leader          UMETA(DisplayName = "Leader"),
    Guard           UMETA(DisplayName = "Guard")
};

USTRUCT(BlueprintType)
struct FNPCMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RememberedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> RelationshipValues;

    FNPCMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        ThreatLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPCSchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCState ScheduledState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ActivityDescription;

    FNPCSchedule()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        ScheduledState = ENPCState::Idle;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("Default Activity");
    }
};

/**
 * Advanced NPC Behavior Component
 * Handles AI behavior, memory, social interactions, and daily routines
 * Designed for immersive prehistoric tribal NPCs
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE NPC PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPCState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    FVector WorkLocation;

    // === BEHAVIOR TREE INTEGRATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBlackboardData* BlackboardAsset;

    // === MEMORY SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPCMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MaxMemoryDistance;

    // === DAILY SCHEDULE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPCSchedule> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float CurrentGameTime;

    // === SOCIAL SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FString> SocialGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float InteractionCooldown;

    // === PERCEPTION SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees;

    // === PUBLIC FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetNPCState(ENPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdatePlayerMemory(const FVector& PlayerLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemoryEvent(const FString& EventDescription);

    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    void UpdateSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
    FNPCSchedule GetCurrentScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void InteractWithNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    float GetRelationshipValue(const FString& TargetName);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ModifyRelationship(const FString& TargetName, float DeltaValue);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    void OnPlayerSighted(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    void OnPlayerLost();

    // === BLUEPRINT EVENTS ===
    
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnStateChanged(ENPCState OldState, ENPCState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnPlayerDetected(const FVector& PlayerLocation);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnScheduleActivityChanged(const FNPCSchedule& NewActivity);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Events")
    void OnSocialInteraction(const FString& InteractionType, const FString& TargetNPC);

private:
    // === INTERNAL FUNCTIONS ===
    
    void InitializeNPC();
    void UpdateMemory(float DeltaTime);
    void ProcessSchedule();
    void UpdateBlackboard();
    void HandleStateTransitions();
    
    // === INTERNAL VARIABLES ===
    
    UPROPERTY()
    class AAIController* NPCAIController;

    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY()
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    float LastInteractionTime;
    float StateChangeTime;
    ENPCState PreviousState;
    
    // Schedule tracking
    int32 CurrentScheduleIndex;
    bool bScheduleInitialized;
    
    // Memory decay timer
    float MemoryDecayTimer;
    
    // Social interaction timer
    float SocialInteractionTimer;
};