#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Attack UMETA(DisplayName = "Attack"),
    Flee UMETA(DisplayName = "Flee"),
    Pack UMETA(DisplayName = "Pack Behavior")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None UMETA(DisplayName = "No Threat"),
    Low UMETA(DisplayName = "Low Threat"),
    Medium UMETA(DisplayName = "Medium Threat"),
    High UMETA(DisplayName = "High Threat"),
    Critical UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    AActor* ThreatTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float ThreatDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float ThreatScore;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float LastSeenTime;

    FCombat_ThreatAssessment()
    {
        ThreatTarget = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        ThreatDistance = 0.0f;
        ThreatScore = 0.0f;
        LastSeenTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector PackCenter;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float PackCohesion;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunting;

    FCombat_PackBehavior()
    {
        PackLeader = nullptr;
        PackCenter = FVector::ZeroVector;
        PackCohesion = 1.0f;
        bIsPackHunting = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    // Combat State
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FCombat_ThreatAssessment PrimaryThreat;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_ThreatAssessment> ThreatList;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FCombat_PackBehavior PackBehavior;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsApexPredator;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatList();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SelectPrimaryThreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteCombatBehavior();

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ACombat_DinosaurAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void UpdatePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack();

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldAmbush(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteAmbushTactic();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteHuntingPattern();

protected:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    float CalculateThreatScore(AActor* Target);
    bool IsValidThreat(AActor* Target);
    void CleanupThreatList();
};

#include "Combat_DinosaurAI.generated.h"