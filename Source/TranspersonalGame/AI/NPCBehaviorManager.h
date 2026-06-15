#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Gathering,
    Hunting,
    Socializing,
    Resting,
    Fleeing,
    Patrolling,
    Crafting
};

UENUM(BlueprintType)
enum class ENPC_Relationship : uint8
{
    Unknown,
    Hostile,
    Neutral,
    Friendly,
    Trusted,
    Family,
    Rival,
    Feared
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FString LocationName;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastVisited;

    FNPC_MemoryEntry()
    {
        LocationName = TEXT("");
        Position = FVector::ZeroVector;
        Importance = 0.0f;
        LastVisited = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    FString NPCName;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    ENPC_Relationship RelationType;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    float TrustLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    float LastInteraction;

    FNPC_SocialRelation()
    {
        NPCName = TEXT("");
        RelationType = ENPC_Relationship::Unknown;
        TrustLevel = 0.0f;
        LastInteraction = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    float StartTime;

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    float EndTime;

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    ENPC_BehaviorState Activity;

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    FString Description;

    FNPC_DailyRoutine()
    {
        StartTime = 0.0f;
        EndTime = 0.0f;
        Activity = ENPC_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        Description = TEXT("");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core NPC Identity
    UPROPERTY(BlueprintReadWrite, Category = "Identity")
    FString NPCName;

    UPROPERTY(BlueprintReadWrite, Category = "Identity")
    FString Role;

    UPROPERTY(BlueprintReadWrite, Category = "Identity")
    int32 Age;

    UPROPERTY(BlueprintReadWrite, Category = "Identity")
    float Experience;

    // Current State
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float StateTimer;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    FVector CurrentTarget;

    // Memory System
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> KnownLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FString> Fears;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FString> Skills;

    // Social System
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelation> Relationships;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    float SocialNeed;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    float LastSocialInteraction;

    // Daily Routine
    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_DailyRoutine> DailySchedule;

    UPROPERTY(BlueprintReadWrite, Category = "Routine")
    float CurrentGameTime;

    // Survival Needs
    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Energy;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryLocation(const FString& LocationName, const FVector& Position, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetBestKnownLocation(const FString& LocationType);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(const FString& OtherNPCName, ENPC_Relationship NewRelation, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    ENPC_Relationship GetRelationshipWith(const FString& OtherNPCName);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    ENPC_BehaviorState GetCurrentScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector FindSafeLocation();

private:
    void ProcessDailyRoutine();
    void HandleEmergencyBehavior();
    void UpdateSocialNeeds(float DeltaTime);
    float CalculateLocationImportance(const FString& LocationType);
};