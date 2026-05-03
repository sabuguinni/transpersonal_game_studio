#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Investigate,
    Engage,
    Flank,
    Retreat,
    Ambush,
    PackCoordination
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        ThreatLevel = 0.0f;
        LastKnownPlayerLocation = FVector::ZeroVector;
        EngagementDistance = 1000.0f;
        bIsPackLeader = false;
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

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange;

public:
    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(bool bIsLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(const FVector& PlayerLocation, float FlankDistance);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Tactical AI
    void UpdateTacticalBehavior(float DeltaTime);
    void CalculateOptimalPosition();
    void CommunicateWithPack();

private:
    AActor* CurrentTarget;
    float LastPlayerSightTime;
    TArray<ACombatAIController*> PackMembers;
};