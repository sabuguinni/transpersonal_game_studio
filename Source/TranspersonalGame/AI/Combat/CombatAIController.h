#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../Behavior/NPCBehaviorTreeManager.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Hunting     UMETA(DisplayName = "Hunting"), 
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Coordinating UMETA(DisplayName = "Coordinating")
};

UENUM(BlueprintType)
enum class ECombat_AttackPattern : uint8
{
    DirectAssault   UMETA(DisplayName = "Direct Assault"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    AmbushStrike    UMETA(DisplayName = "Ambush Strike"),
    TerritorialDefense UMETA(DisplayName = "Territorial Defense"),
    HitAndRun       UMETA(DisplayName = "Hit And Run")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CoordinationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackPattern PreferredPattern = ECombat_AttackPattern::DirectAssault;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        FlankingDistance = 500.0f;
        RetreatHealthThreshold = 0.3f;
        CoordinationRadius = 800.0f;
        PreferredPattern = ECombat_AttackPattern::DirectAssault;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core Combat Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Combat", meta = (AllowPrivateAccess = "true"))
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Combat", meta = (AllowPrivateAccess = "true"))
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Combat", meta = (AllowPrivateAccess = "true"))
    class UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    class UBehaviorTree* CombatBehaviorTree;

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_CombatState CurrentCombatState = ECombat_CombatState::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float CombatEngagementRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float CombatDisengagementRange = 1500.0f;

    // Pack Coordination
    UPROPERTY(BlueprintReadWrite, Category = "Pack Combat")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack Combat")
    ACombatAIController* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Combat")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Combat")
    int32 MaxPackSize = 5;

    // Target Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat Target")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Target")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target")
    float TargetLostTimeout = 10.0f;

    // Damage and Health
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Stats")
    float LastAttackTime = 0.0f;

public:
    // Combat State Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_CombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EngageCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DisengageCombat();

    // Pack Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    TArray<ACombatAIController*> GetNearbyAllies(float SearchRadius = 800.0f);

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteAttackPattern(ECombat_AttackPattern Pattern);

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    void DealDamageToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Damage")
    float CalculateDamage(AActor* Target);

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void UpdateTargetTracking(float DeltaTime);
    void ProcessTacticalDecisions();

    // Blackboard Keys
    FName TargetActorKey = TEXT("TargetActor");
    FName CombatStateKey = TEXT("CombatState");
    FName LastKnownLocationKey = TEXT("LastKnownLocation");
    FName PackLeaderKey = TEXT("PackLeader");
    FName AttackPatternKey = TEXT("AttackPattern");

private:
    float TargetLostTimer = 0.0f;
    float PackCoordinationTimer = 0.0f;
    float TacticalUpdateTimer = 0.0f;
    
    // Performance optimization
    int32 FrameCounter = 0;
    static const int32 UpdateFrequency = 5; // Update every 5 frames
};