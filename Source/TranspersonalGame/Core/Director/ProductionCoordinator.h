#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production Coordinator - Studio Director's central coordination system
 * Manages agent task distribution and milestone tracking for Milestone 1
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core coordination components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    USceneComponent* RootSceneComponent;

    // Milestone 1 tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bTerrainGenerated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCharacterMovementWorking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bDinosaursSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bLightingSetup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bSurvivalHUDActive;

    // Agent task assignments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Tasks")
    TArray<FString> PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Tasks")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Tasks")
    int32 CurrentCycle;

public:
    // Milestone verification functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignAgentTask(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Completion();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    // Critical system checks
    UFUNCTION(BlueprintCallable, Category = "Production")
    void VerifyCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void VerifyDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void VerifyTerrainQuality();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void VerifyLightingSetup();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetNextAgentPriorities();

protected:
    // Internal tracking
    UPROPERTY()
    float LastVerificationTime;

    UPROPERTY()
    bool bInitialized;
};