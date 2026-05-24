#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorManager.generated.h"

// NPC Memory System - stores what NPCs remember about interactions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_Relationship Relationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FDateTime LastInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    FNPC_Memory()
    {
        RememberedActor = nullptr;
        Relationship = ENPC_Relationship::Neutral;
        TrustLevel = 0.5f;
        FearLevel = 0.0f;
        LastInteraction = FDateTime::Now();
        LastKnownLocation = FVector::ZeroVector;
    }
};

// NPC Daily Schedule Entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_Activity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Priority;

    FNPC_ScheduleEntry()
    {
        StartHour = 8.0f;
        EndHour = 18.0f;
        Activity = ENPC_Activity::Idle;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

// NPC Personality Traits
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Personality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    FNPC_Personality()
    {
        Courage = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Aggression = 0.3f;
        Intelligence = 0.5f;
    }
};

/**
 * NPC Behavior Manager - Controls individual NPC AI behavior, memory, and social interactions
 * This component is attached to NPC characters to give them autonomous behavior
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE NPC PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Archetype NPCArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Tribe NPCTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 NPCAge;

    // === PERSONALITY SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPC_Personality Personality;

    // === MEMORY SYSTEM ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // === DAILY SCHEDULE SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_ScheduleEntry> DailySchedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Schedule")
    ENPC_Activity CurrentActivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Schedule")
    FVector CurrentTarget;

    // === SOCIAL INTERACTION SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float ConversationCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    AActor* CurrentConversationPartner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    float LastConversationTime;

    // === BEHAVIOR TREE INTEGRATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // === PUBLIC METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetCurrentActivity(ENPC_Activity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(AActor* Actor, ENPC_Relationship Relationship, float Trust, float Fear);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_Memory* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartConversation(AActor* Partner);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanStartConversation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_Activity GetCurrentScheduledActivity() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToNoise(FVector NoiseLocation, float NoiseIntensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToThreat(AActor* ThreatActor);

private:
    // === INTERNAL METHODS ===
    
    void UpdateMemories(float DeltaTime);
    void UpdateSocialInteractions();
    void UpdateBehaviorTree();
    void ProcessSchedule();
    float GetCurrentGameHour() const;
    void InitializeDefaultSchedule();
    void DecayMemory(FNPC_Memory& Memory, float DeltaTime);
    bool IsActorInSocialRange(AActor* Actor) const;

    // === INTERNAL STATE ===
    
    float LastScheduleUpdate;
    float LastMemoryUpdate;
    float LastSocialUpdate;
    
    UPROPERTY()
    class AAIController* NPCAIController;
    
    UPROPERTY()
    class UBehaviorTreeComponent* BehaviorTreeComponent;
    
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
};