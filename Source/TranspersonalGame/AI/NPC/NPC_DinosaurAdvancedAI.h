#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAdvancedAI.generated.h"

class TRANSPERSONALGAME_API UNPC_DinosaurAdvancedAI : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPC_DinosaurAdvancedAI();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Advanced AI States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI")
	ENPC_DinosaurAIState CurrentAIState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI")
	float ThreatDetectionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI")
	float HuntingRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced AI")
	float FleeRadius;

	// Learning and Adaptation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	TMap<FString, float> LearnedBehaviors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	float LearningRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	int32 ExperiencePoints;

	// Environmental Awareness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TArray<AActor*> KnownThreats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TArray<AActor*> KnownFood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FVector SafeZoneLocation;

	// Advanced Decision Making
	UFUNCTION(BlueprintCallable, Category = "Advanced AI")
	void EvaluateThreatLevel(AActor* ThreatActor);

	UFUNCTION(BlueprintCallable, Category = "Advanced AI")
	void UpdateBehaviorBasedOnExperience();

	UFUNCTION(BlueprintCallable, Category = "Advanced AI")
	void LearnFromInteraction(AActor* InteractionTarget, bool bPositiveOutcome);

	UFUNCTION(BlueprintCallable, Category = "Advanced AI")
	FVector CalculateOptimalPosition();

	UFUNCTION(BlueprintCallable, Category = "Advanced AI")
	void ExecuteAdvancedBehavior();

private:
	float LastThreatEvaluation;
	float ThreatEvaluationInterval;
	
	void UpdateEnvironmentalAwareness();
	void ProcessLearning();
	bool ShouldChangeState(ENPC_DinosaurAIState NewState);
};