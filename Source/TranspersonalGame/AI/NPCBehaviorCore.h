// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "NPCBehaviorCore.generated.h"

UENUM(BlueprintType)
enum class ENPCPersonality : uint8
{
    Curious         UMETA(DisplayName = "Curious"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Timid          UMETA(DisplayName = "Timid"),
    Social         UMETA(DisplayName = "Social"),
    Territorial    UMETA(DisplayName = "Territorial"),
    Peaceful       UMETA(DisplayName = "Peaceful")
};

UENUM(BlueprintType)
enum class ENPCMoodState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Alert          UMETA(DisplayName = "Alert"),
    Frightened     UMETA(DisplayName = "Frightened"),
    Hungry         UMETA(DisplayName = "Hungry"),
    Tired          UMETA(DisplayName = "Tired"),
    Aggressive     UMETA(DisplayName = "Aggressive"),
    Curious        UMETA(DisplayName = "Curious")
};

UENUM(BlueprintType)
enum class ENPCActivity : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Foraging       UMETA(DisplayName = "Foraging"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Resting        UMETA(DisplayName = "Resting"),
    Socializing    UMETA(DisplayName = "Socializing"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Drinking       UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPositive;

    FNPCMemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalWeight = 0.0f;
        EventDescription = TEXT("");
        bIsPositive = true;
    }
};

USTRUCT(BlueprintType)
struct FNPCDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hours in day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCActivity Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    FNPCDailyRoutine()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        Activity = ENPCActivity::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCMoodState CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float SocialNeed;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPCMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate;

    // Daily Routine System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FNPCDailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    float CurrentDayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    bool bFollowDailyRoutine;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Territory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bDefendTerritory;

    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> SocialRelationships; // Actor -> Relationship value (-1 to 1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemoryEntry(FVector Location, const FString& Description, float EmotionalWeight, bool bPositive = true);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(ENPCMoodState NewMood);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ChangeActivity(ENPCActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPCDailyRoutine GetCurrentRoutineActivity();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateSocialRelationship(AActor* OtherActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetRelationshipWith(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayerPresence(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessNaturalNeeds(float DeltaTime);

private:
    void UpdateMemoryDecay(float DeltaTime);
    void UpdateDailyRoutine(float DeltaTime);
    void UpdateNaturalNeeds(float DeltaTime);
    void EvaluateMoodChanges();
    FNPCMemoryEntry* FindMemoryNear(FVector Location, float Radius = 500.0f);
    
    // Internal state
    float LastMoodUpdateTime;
    float LastRoutineCheckTime;
    FNPCDailyRoutine CurrentRoutineActivity;
};