#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Core/SharedTypes.h"
#include "DinosaurBehaviorController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior Tree Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Sight Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    // Hearing Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // Blackboard Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* GetTargetActor() const;

    // Dinosaur-specific behavior
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTerritoryCenter(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetPackLeader(ADinosaurBehaviorController* Leader);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AddPackMember(ADinosaurBehaviorController* Member);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void RemovePackMember(ADinosaurBehaviorController* Member);

protected:
    // Territory data
    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    float TerritoryRadius;

    // Pack behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ADinosaurBehaviorController* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ADinosaurBehaviorController*> PackMembers;

    // Current behavior state
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    ENPC_BehaviorState CurrentBehaviorState;

    // Perception range settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float HearingRadius;

private:
    void SetupPerception();
    void SetupBlackboard();
};