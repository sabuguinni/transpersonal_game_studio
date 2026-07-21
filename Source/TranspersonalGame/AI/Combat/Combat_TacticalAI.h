#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol         UMETA(DisplayName = "Patrol"),
    Investigate    UMETA(DisplayName = "Investigate"),
    Engage         UMETA(DisplayName = "Engage"),
    Flank          UMETA(DisplayName = "Flank"),
    Retreat        UMETA(DisplayName = "Retreat"),
    Ambush         UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None           UMETA(DisplayName = "None"),
    Line           UMETA(DisplayName = "Line"),
    Circle         UMETA(DisplayName = "Circle"),
    Pincer         UMETA(DisplayName = "Pincer"),
    Wedge          UMETA(DisplayName = "Wedge")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingTendency = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_Formation PreferredFormation = ECombat_Formation::None;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        FlankingTendency = 0.3f;
        RetreatThreshold = 0.2f;
        SightRange = 2000.0f;
        HearingRange = 1500.0f;
        PreferredFormation = ECombat_Formation::None;
    }
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> AlliedUnits;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownTargetLocation;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddAlliedUnit(AActor* AllyActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveAlliedUnit(AActor* AllyActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(AActor* Target, float FlankDistance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFormation(ECombat_Formation Formation);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool HasLineOfSightToTarget() const;

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void UpdateTacticalState();
    void ProcessCombatLogic();
    FVector CalculateOptimalPosition();
    void CommunicateWithAllies();

private:
    float LastStateUpdateTime;
    float StateUpdateInterval;
    bool bIsInCombat;
    float CombatStartTime;
};