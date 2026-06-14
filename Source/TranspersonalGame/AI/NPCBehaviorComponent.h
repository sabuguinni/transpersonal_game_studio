#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm = 0,
    Alert = 1,
    Aggressive = 2,
    Fearful = 3,
    Curious = 4,
    Territorial = 5,
    Hunting = 6,
    Fleeing = 7
};

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Lone = 0,
    PackLeader = 1,
    PackMember = 2,
    Alpha = 3,
    Beta = 4,
    Omega = 5,
    Juvenile = 6,
    Elder = 7
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerMemory;

    FNPC_Memory()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        ThreatLevel = 0;
        bIsPlayerMemory = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHostile;

    FNPC_SocialRelation()
    {
        RelatedActor = nullptr;
        RelationshipValue = 0.0f;
        LastInteractionTime = 0.0f;
        bIsHostile = false;
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
    // Core Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_SocialRole SocialRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AwarenessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector HomeLocation;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayTime;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float SocialUpdateInterval;

    // Behavior Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float EmotionalDecayRate;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "NPC State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "NPC State")
    float LastPlayerInteractionTime;

    UPROPERTY(BlueprintReadOnly, Category = "NPC State")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "NPC State")
    bool bIsPatrolling;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(FVector Location, int32 ThreatLevel, bool bIsPlayer = false);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateSocialRelation(AActor* Actor, float RelationChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldAttack();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetPatrolDestination();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartCombatBehavior(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void EndCombatBehavior();

private:
    FTimerHandle PatrolTimer;
    FTimerHandle MemoryDecayTimer;
    FTimerHandle SocialUpdateTimer;

    void UpdatePatrol();
    void DecayMemories();
    void UpdateSocialRelations();
    void UpdateEmotionalState(float DeltaTime);
    float CalculateThreatLevel(AActor* Actor);
};