#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "Combat_AIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrol,
    Investigating,
    Hunting,
    Attacking,
    Retreating,
    Flanking,
    Coordinating
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float CautiousDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FlankingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanCoordinate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    int32 PackSize = 1;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        CautiousDistance = 1000.0f;
        AttackRange = 300.0f;
        FlankingRadius = 800.0f;
        bCanCoordinate = true;
        PackSize = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_AIState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AIState GetAIState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInPackFormation() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack();

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void UpdateTacticalState(float DeltaTime);
    void ProcessPackCoordination();
    FVector CalculateOptimalPosition(AActor* Target) const;
    bool CanSeeTarget(AActor* Target) const;
    float GetDistanceToTarget(AActor* Target) const;
};

#include "Combat_AIController.generated.h"