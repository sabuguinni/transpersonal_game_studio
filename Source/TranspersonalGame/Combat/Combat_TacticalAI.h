#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Ambushing   UMETA(DisplayName = "Ambushing")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator    UMETA(DisplayName = "Predator"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
    PackHunter  UMETA(DisplayName = "Pack Hunter")
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
    float AmbushWaitTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        FlankingDistance = 500.0f;
        RetreatHealthThreshold = 0.3f;
        AmbushWaitTime = 5.0f;
        PackSize = 3;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeTacticalAI(ECombat_DinosaurType DinosaurType, const FCombat_TacticalData& TacticalData);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalBehavior(float DeltaTime);

    // Tactical behaviors
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteStalkingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAmbushBehavior();

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPackFormation();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<UCombat_TacticalAI*> GetNearbyPackMembers(float SearchRadius = 1000.0f);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool HasClearLineOfSight() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    APawn* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AAIController* AIController;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsInPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<UCombat_TacticalAI*> PackMembers;

private:
    void UpdateBlackboardValues();
    void HandleStateTransition(ECombat_TacticalState NewState);
    bool ValidateTarget() const;
    FVector GetRandomFlankingPosition() const;
};

#include "Combat_TacticalAI.generated.h"