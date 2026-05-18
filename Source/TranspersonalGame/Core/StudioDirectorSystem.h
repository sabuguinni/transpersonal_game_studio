#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    float CompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FDateTime LastUpdate;

    FDir_AgentStatus()
    {
        AgentNumber = 0;
        AgentName = TEXT("Unknown");
        CurrentTask = TEXT("Idle");
        CompletionPercentage = 0.0f;
        bIsActive = false;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 EnvironmentActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 BiomesPopulated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentCycleID;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurCount = 0;
        EnvironmentActors = 0;
        BiomesPopulated = 0;
        MemoryUsagePercent = 0.0f;
        CurrentCycleID = TEXT("UNKNOWN");
    }
};

/**
 * Studio Director System - Coordinates all 18 agents and tracks production metrics
 * Responsible for agent task distribution, progress monitoring, and build validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, const FString& TaskDescription, float CompletionPercent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentStatus GetAgentStatus(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentStatus> GetAllAgentStatuses() const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayablePrototype() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Emergency coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyBuildValidation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugSpawnDinosaurs();

protected:
    // Agent status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Agents")
    TArray<FDir_AgentStatus> AgentStatuses;

    // Production metrics
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    int32 MaxAgentCount = 19;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float MetricsUpdateInterval = 30.0f;

    // Timer handles
    FTimerHandle MetricsUpdateTimer;

private:
    void InitializeDefaultAgentStatuses();
    void CountLevelActors();
    void ValidateBiomePopulation();
    void CheckMemoryUsage();
};

#include "StudioDirectorSystem.generated.h"