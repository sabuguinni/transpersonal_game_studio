#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director Component - Coordinates development workflow and agent communication
 * Manages production cycles, task delegation, and quality assurance
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidatePlayablePrototype();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportAgentProgress(int32 AgentNumber, const FString& Progress);

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 CountOrphanHeaders();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalErrors();

protected:
    // Production tracking
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bProductionActive;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CycleStartTime;

    // Agent status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Agents")
    TMap<int32, FString> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Agents")
    TMap<int32, FString> AgentProgress;

    // Quality metrics
    UPROPERTY(BlueprintReadOnly, Category = "Quality")
    int32 CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Quality")
    int32 OrphanHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Quality")
    TArray<FString> CriticalIssues;

private:
    void UpdateProductionMetrics();
    void ValidateCodebase();
    void CheckPlayabilityRequirements();
};

/**
 * Studio Director Actor - Main coordinator for development workflow
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core studio director component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStudioDirectorComponent* StudioDirectorComponent;

    // Development workflow management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void RunCriticalAssessment();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateProductionReport();

protected:
    // Production state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoManageProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionCycleDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 MaxConcurrentAgents;

private:
    void MonitorAgentChain();
    void EnsurePlayablePrototype();
};