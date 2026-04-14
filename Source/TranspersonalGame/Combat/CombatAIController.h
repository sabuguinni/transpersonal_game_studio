#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "CombatAIController.generated.h"

// Combat AI States for tactical decision making
UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"), 
    Investigate     UMETA(DisplayName = "Investigate"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Combat          UMETA(DisplayName = "Combat"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Flank           UMETA(DisplayName = "Flank"),
    Ambush          UMETA(DisplayName = "Ambush")
};

// Combat tactics for different enemy types
UENUM(BlueprintType)
enum class ECombat_TacticType : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    Ambusher        UMETA(DisplayName = "Ambusher"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Opportunist     UMETA(DisplayName = "Opportunist")
};

// Combat AI configuration data
USTRUCT(BlueprintType)
struct FCombat_AIConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticType TacticType = ECombat_TacticType::Aggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HelpCallRadius = 1000.0f;
};

/**
 * Advanced Combat AI Controller with tactical decision making
 * Handles pack coordination, flanking maneuvers, and adaptive combat behavior
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

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception") 
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_AIConfig CombatConfig;

    // Current AI State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentState;

    // Target Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> KnownEnemies;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<ACombatAIController*> PackMembers;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddKnownEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveKnownEnemy(AActor* Enemy);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp(AActor* Threat);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInPack() const { return PackMembers.Num() > 0; }

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal State Management
    void UpdateAIState(float DeltaTime);
    void ProcessCombatLogic();
    void UpdateBlackboard();

    // Pack Communication
    void BroadcastToPackMembers(const FString& Message, AActor* Subject = nullptr);
    void RespondToPackCall(ACombatAIController* Caller, AActor* Threat);

private:
    float LastStateChangeTime;
    float StateChangeDelay = 0.5f;
    
    // Combat timing
    float LastAttackTime;
    float AttackCooldown = 2.0f;
    
    // Pack coordination
    bool bIsPackLeader;
    ACombatAIController* PackLeader;
};