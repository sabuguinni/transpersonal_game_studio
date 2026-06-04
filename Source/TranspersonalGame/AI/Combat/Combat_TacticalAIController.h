#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/TargetPoint.h"
#include "SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

class UCombat_DamageSystem;
class UCombat_FormationManager;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Combat      UMETA(DisplayName = "Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Regrouping  UMETA(DisplayName = "Regrouping")
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Scout       UMETA(DisplayName = "Scout"),
    Defender    UMETA(DisplayName = "Defender")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalRole TacticalRole = ECombat_TacticalRole::Scout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackCoordination = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritorialRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TimeSinceLastSighting = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIController();

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

    // Combat Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombat_DamageSystem* DamageSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombat_FormationManager* FormationManager;

    // Tactical Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float OptimalCombatDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackCoordinationRadius = 2000.0f;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AIState GetCombatState() const { return TacticalData.CurrentState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCombatTarget() const { return TacticalData.CurrentTarget; }

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterPackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemovePackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAsPackLeader(bool bLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetNearbyPackMembers(float Radius = 2000.0f);

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankingPosition(AActor* Target, float Distance = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFleeFromCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector FindCoverPosition(AActor* ThreatActor, float MinDistance = 1000.0f);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFleeManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(AActor* Target);

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal State Logic
    void UpdateCombatState(float DeltaTime);
    void ProcessTacticalDecisions();
    void UpdatePackCoordination();
    void HandleCombatEngagement();
    void ManageAggression(float DeltaTime);

    // Utility Functions
    float CalculateThreatLevel(AActor* Target);
    bool IsInTerritorialRange(AActor* Actor);
    FVector GetOptimalAttackPosition(AActor* Target);
    void BroadcastStateToPackMembers(ECombat_AIState NewState);

private:
    float StateTimer = 0.0f;
    float LastPackCoordinationTime = 0.0f;
    FVector InitialTerritoryCenter = FVector::ZeroVector;
};