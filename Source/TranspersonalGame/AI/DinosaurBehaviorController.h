#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorController.generated.h"

class UBehaviorTree;
class APawn;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* PerceptionComponent;

    // Behavior Trees for different dinosaur types
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* RaptorBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* HerbivoreeBehaviorTree;

    // AI Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float ChaseRange;

    // Dinosaur Type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurType DinosaurType;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    EDinosaurBehaviorState CurrentBehaviorState;

    // Target References
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector LastKnownPlayerLocation;

public:
    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetRandomPatrolPoint() const;

    // Pack Behavior (for Raptors)
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurBehaviorController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    bool IsInPack() const;

    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ADinosaurBehaviorController*> GetPackMembers() const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Pack System
    UPROPERTY()
    ADinosaurBehaviorController* PackLeader;

    UPROPERTY()
    TArray<ADinosaurBehaviorController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRange;

private:
    void SetupPerception();
    void SetupBlackboard();
    UBehaviorTree* GetBehaviorTreeForType() const;
    void UpdateBlackboardValues();
};