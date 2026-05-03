#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Coordenador de Produção para Milestone 1 - "Walk Around"
 * Monitoriza o progresso crítico do jogo e coordena tarefas entre agentes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UObject
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Estado do Milestone 1
    UPROPERTY(BlueprintReadOnly, Category = "Milestone 1")
    bool bCharacterMovementReady;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone 1")
    bool bLandscapeReady;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone 1")
    bool bDinosaursSpawned;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone 1")
    bool bLightingConfigured;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone 1")
    int32 TotalActorsInMap;

    // Coordenação de agentes
    UPROPERTY(BlueprintReadOnly, Category = "Agent Coordination")
    TArray<FString> PendingTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Coordination")
    TArray<FString> CompletedTasks;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkTaskCompleted(const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1CompletionPercentage();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CleanupDuplicateActors();

    // Verificações específicas
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterController();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateLandscape();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateLightingSetup();

    // Relatórios
    UFUNCTION(BlueprintCallable, Category = "Reports")
    FString GenerateProgressReport();

    UFUNCTION(BlueprintCallable, Category = "Reports")
    TArray<FString> GetCriticalIssues();

private:
    // Helpers internos
    void UpdateMilestone1Status();
    void LogProductionState();
    int32 CountActorsByClass(UClass* ActorClass);
    void RemoveDuplicateActorsOfType(UClass* ActorClass, int32 MaxAllowed = 1);
};

/**
 * Estrutura para tarefas de produção
 */
USTRUCT(BlueprintType)
struct FDir_ProductionTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    bool bCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Task")
    FDateTime AssignedTime;

    FDir_ProductionTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bCompleted = false;
        AssignedTime = FDateTime::Now();
    }
};

/**
 * Enum para estado do Milestone 1
 */
UENUM(BlueprintType)
enum class EDir_Milestone1State : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    NearCompletion  UMETA(DisplayName = "Near Completion"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};