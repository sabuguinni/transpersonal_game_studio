#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "ProductionDirector.generated.h"

/**
 * Production Director - Central coordination actor for Milestone 1 implementation
 * Monitors and coordinates the development of the playable prototype
 * Ensures all agents work towards the same goal: WALK AROUND gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
	GENERATED_BODY()

public:
	AProductionDirector();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Visual representation in editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DirectorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

public:
	// Milestone 1 tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
	bool bTerrainComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
	bool bCharacterMovementComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
	bool bDinosaursPlaced = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
	bool bLightingComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
	bool bSurvivalHUDComplete = false;

	// Progress tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float Milestone1Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FString CurrentFocus = TEXT("Terrain Generation");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FString> CompletedTasks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FString> PendingTasks;

	// Coordination functions
	UFUNCTION(BlueprintCallable, Category = "Coordination")
	void UpdateMilestoneProgress();

	UFUNCTION(BlueprintCallable, Category = "Coordination")
	void MarkTaskComplete(const FString& TaskName);

	UFUNCTION(BlueprintCallable, Category = "Coordination")
	void AddPendingTask(const FString& TaskName);

	UFUNCTION(BlueprintCallable, Category = "Coordination")
	FString GetNextPriorityTask();

	UFUNCTION(BlueprintCallable, Category = "Coordination")
	bool IsMilestone1Complete();

	// Agent coordination
	UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
	void NotifyAgentProgress(const FString& AgentName, const FString& TaskCompleted);

	UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
	FString GetTaskForAgent(const FString& AgentName);

private:
	void InitializeMilestone1Tasks();
	void LogProgressToConsole();
};