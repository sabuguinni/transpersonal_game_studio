#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "CombatAIController.generated.h"

// Combat AI states for tactical decision making
UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Flanking,
    Attacking,
    Retreating,
    Coordinating
};

// Combat tactics for different enemy types
UENUM(BlueprintType)
enum class ECombat_TacticType : uint8
{
    Aggressive,
    Defensive,
    Flanking,
    Ambush,
    PackHunt,
    Territorial
};

// Combat AI data structure for coordination
USTRUCT(BlueprintType)
struct FCombat_AICoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class ACombatAIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class ACombatAIController* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_TacticType CurrentTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationRadius;

    FCombat_AICoordination()
    {
        PackLeader = nullptr;
        TargetLocation = FVector::ZeroVector;
        CurrentTactic = ECombat_TacticType::Aggressive;
        CoordinationRadius = 1000.0f;
    }
};

/**
 * Advanced Combat AI Controller with tactical intelligence
 * Implements pack behavior, flanking maneuvers, and adaptive combat tactics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    class UBlackboardData* BlackboardAsset;

    // Combat AI State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticType PreferredTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatHealthThreshold;

    // Pack Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    FCombat_AICoordination PackCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    float PackCoordinationRadius;

    // Target Information
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    class APawn* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float TimeSinceTargetSeen;

public:
    // Core AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    APawn* GetCurrentTarget() const { return CurrentTarget; }

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticType ChooseBestTactic();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    TArray<ACombatAIController*> FindNearbyAllies();

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
    // Internal AI Logic
    void UpdateCombatState(float DeltaTime);
    void UpdateTargetInformation(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    
    // Tactical Calculations
    FVector FindBestCoverPosition();
    FVector CalculateInterceptPosition();
    bool IsFlankingPositionValid(const FVector& Position);
    
    // Pack Communication
    void SendPackCommand(const FString& Command);
    void ReceivePackCommand(const FString& Command, ACombatAIController* Sender);

private:
    // Internal state tracking
    float StateChangeTimer;
    float LastTargetUpdateTime;
    FVector PreviousTargetLocation;
    
    // Performance optimization
    float NextPerceptionUpdate;
    float PerceptionUpdateInterval;
};