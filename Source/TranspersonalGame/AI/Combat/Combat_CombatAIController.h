#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Combat_CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Engaging UMETA(DisplayName = "Engaging"),
    Attacking UMETA(DisplayName = "Attacking"),
    Retreating UMETA(DisplayName = "Retreating"),
    Stunned UMETA(DisplayName = "Stunned")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None UMETA(DisplayName = "None"),
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_CombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Combat State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold;

    // Threat Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<FCombat_ThreatData> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatForgetTime;

    // Combat Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanFlank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanAmbush;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_CombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_CombatState GetCombatState() const { return CurrentCombatState; }

    // Threat Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreat(AActor* ThreatActor, ECombat_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetHighestThreat();

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetThreatDistance(AActor* ThreatActor);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(AActor* Target);

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Combat Logic
    void UpdateThreatAssessment();
    void UpdateCombatBehavior(float DeltaTime);
    ECombat_ThreatLevel CalculateThreatLevel(AActor* Actor);
    void CleanupOldThreats();
};