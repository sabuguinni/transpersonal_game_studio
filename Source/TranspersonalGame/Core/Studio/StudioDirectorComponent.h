#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "StudioDirectorComponent.generated.h"

/**
 * Studio Director Component - Central coordination system for all production agents
 * Manages the 19-agent production pipeline and ensures Milestone 1 completion
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_StudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_StudioDirectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Production Pipeline Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckMilestone1Status();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAgentTasks();

    // Agent Communication
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SendTaskToAgent(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReceiveAgentReport(int32 AgentID, const FString& Report);

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MonitorBuildStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePlayablePrototype();

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyBuild();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResetProductionPipeline();

protected:
    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bMilestone1Complete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentProductionCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionBudgetLimit;

    // Agent Status Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<bool> AgentActiveStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FString> AgentCurrentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<float> AgentLastReportTime;

    // Milestone 1 Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCharacterMovementWorking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bTerrainExists;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bDinosaursVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bLightingSetup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bSurvivalHUDWorking;

private:
    // Internal coordination methods
    void UpdateAgentStatus();
    void CheckCriticalBlockers();
    void GenerateProductionReport();
    
    // Timer handles
    FTimerHandle ProductionMonitorTimer;
    FTimerHandle AgentCoordinationTimer;
};