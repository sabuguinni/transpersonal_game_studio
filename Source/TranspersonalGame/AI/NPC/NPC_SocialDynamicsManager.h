#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "NPC_SocialDynamicsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TargetNPC = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float AggressionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    FNPC_SocialRelationship()
    {
        TrustLevel = 0.5f;
        FearLevel = 0.0f;
        AggressionLevel = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* AssociatedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ImportanceLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription = TEXT("");

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        AssociatedActor = nullptr;
        Timestamp = 0.0f;
        ImportanceLevel = 1.0f;
        EventDescription = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_SocialDynamicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialDynamicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float SocialUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float MaxSocialRange = 2000.0f;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    // Daily Routines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float PatrolWaitTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bIsPatrolling = true;

    // Behavior State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPCBehaviorState CurrentBehaviorState = ENPCBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* CurrentTarget = nullptr;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateSocialRelationships();

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    FNPC_SocialRelationship* GetRelationshipWith(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void AddOrUpdateRelationship(AActor* TargetActor, float TrustDelta, float FearDelta, float AggressionDelta);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, AActor* Actor, const FString& Description, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesNear(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetupPatrolRoute(const TArray<FVector>& NewPatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void UpdatePatrolBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldReactToPlayer(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessPlayerInteraction(AActor* PlayerActor);

private:
    float LastSocialUpdate = 0.0f;
    float PatrolTimer = 0.0f;
    bool bReachedPatrolPoint = false;

    void CleanupOldMemories();
    void TransferMemoriesToLongTerm();
    FVector GetNextPatrolPoint();
    bool IsNearPatrolPoint(FVector Point, float Tolerance = 100.0f);
};