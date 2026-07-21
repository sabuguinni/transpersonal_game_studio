#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "TranspersonalGame.h"
#include "SharedTypes.h"
#include "NPCBehaviorTreeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_NPCState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<AActor*> NearbyThreats;

    FNPC_BehaviorState()
    {
        CurrentState = ENPC_NPCState::Idle;
        StateTimer = 0.0f;
        LastKnownPlayerLocation = FVector::ZeroVector;
        AlertLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    bool bIsHostile;

    FNPC_MemoryEntry()
    {
        TargetActor = nullptr;
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        ThreatLevel = 0.0f;
        bIsHostile = false;
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
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorTree(UBehaviorTree* BehaviorTreeAsset);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_NPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_NPCState GetCurrentBehaviorState() const;

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemoryEntry(AActor* Target, FVector Location, float ThreatLevel, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateMemoryEntry(AActor* Target, FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_MemoryEntry GetMemoryEntry(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ClearOldMemories(float MaxAge);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetHighestThreat();

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void UpdateSocialRelations();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ReactToNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ShareThreatInformation(AActor* Threat, float Range);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    FNPC_BehaviorState BehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float MemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float SocialInteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float AlertDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float MaxAlertLevel;

    // Behavior Tree References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

private:
    void UpdateBehaviorTreeBlackboard();
    void ProcessMemoryDecay(float DeltaTime);
    void UpdateAlertLevel(float DeltaTime);
    AActor* FindNearestThreat();
    void BroadcastThreatToAllies(AActor* Threat);
};