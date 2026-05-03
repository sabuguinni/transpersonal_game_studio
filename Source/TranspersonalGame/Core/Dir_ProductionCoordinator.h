#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Estrutura para rastrear o progresso do Milestone 1
 */
USTRUCT(BlueprintType)
struct FDir_Milestone1Progress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    bool bCharacterMovementWorking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    bool bLandscapeExpanded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    bool bDinosaursSpawned = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    bool bLightingConfigured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    bool bBiomesDistributed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    int32 CompletedTasks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    int32 TotalTasks = 5;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Progress")
    float ProgressPercentage = 0.0f;

    FDir_Milestone1Progress()
    {
        bCharacterMovementWorking = false;
        bLandscapeExpanded = false;
        bDinosaursSpawned = false;
        bLightingConfigured = false;
        bBiomesDistributed = false;
        CompletedTasks = 0;
        TotalTasks = 5;
        ProgressPercentage = 0.0f;
    }

    void UpdateProgress()
    {
        CompletedTasks = 0;
        if (bCharacterMovementWorking) CompletedTasks++;
        if (bLandscapeExpanded) CompletedTasks++;
        if (bDinosaursSpawned) CompletedTasks++;
        if (bLightingConfigured) CompletedTasks++;
        if (bBiomesDistributed) CompletedTasks++;
        
        ProgressPercentage = (float)CompletedTasks / (float)TotalTasks * 100.0f;
    }
};

/**
 * Estrutura para coordenar tarefas entre agentes
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    bool bIsBlocked = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    FString BlockedReason;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Task")
    int32 Priority = 0; // 0 = baixa, 10 = crítica

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        bIsCompleted = false;
        bIsBlocked = false;
        BlockedReason = TEXT("");
        Priority = 0;
    }
};

/**
 * Coordenador de produção que monitoriza o progresso do Milestone 1
 * e coordena as tarefas entre os 18 agentes especializados
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Progresso do Milestone 1
    UPROPERTY(BlueprintReadOnly, Category = "Production Coordinator")
    FDir_Milestone1Progress Milestone1Progress;

    // Lista de tarefas por agente
    UPROPERTY(BlueprintReadOnly, Category = "Production Coordinator")
    TArray<FDir_AgentTask> AgentTasks;

    // Funções de coordenação
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UpdateMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void MarkTaskCompleted(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void BlockTask(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void UnblockTask(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    float GetMilestone1ProgressPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool IsMilestone1Complete() const;

    // Verificações específicas do estado do jogo
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool VerifyCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool VerifyLandscapeExpansion();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool VerifyDinosaurSpawns();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool VerifyLightingConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    bool VerifyBiomeDistribution();

    // Logging e debug
    UFUNCTION(BlueprintCallable, Category = "Production Coordinator")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Coordinator")
    void DebugPrintAllTasks();

private:
    // Timer para verificações periódicas
    float VerificationTimer = 0.0f;
    const float VerificationInterval = 5.0f; // Verificar a cada 5 segundos

    // Funções auxiliares
    void CreateAgentTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, int32 Priority);
    FDir_AgentTask* FindTaskByAgent(int32 AgentID);
};