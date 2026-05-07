#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordination System
 * Manages agent task assignments, priority tracking, and production milestones
 * Ensures proper coordination between all 19 specialized agents
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString Priority; // CRITICAL, HIGH, MEDIUM, LOW

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    TArray<FString> TaskList;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    TArray<FString> Deliverables;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        Priority = TEXT("MEDIUM");
        bCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "World")
    FString BiomeName;

    UPROPERTY(BlueprintReadWrite, Category = "World")
    FVector CenterLocation;

    UPROPERTY(BlueprintReadWrite, Category = "World")
    FVector MinBounds;

    UPROPERTY(BlueprintReadWrite, Category = "World")
    FVector MaxBounds;

    UPROPERTY(BlueprintReadWrite, Category = "World")
    TArray<FString> TypicalActors;

    FDir_BiomeCoordinates()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        MinBounds = FVector::ZeroVector;
        MaxBounds = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalCppFiles;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 PhantomHeaders; // .h files without .cpp

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalHeaderFiles = 0;
        TotalCppFiles = 0;
        PhantomHeaders = 0;
        CompilationErrors = 0;
        ActiveAgents = 0;
        OverallProgress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(const FString& AgentName, const FString& Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const TArray<FString>& Tasks, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentProgress(const FString& AgentName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void MarkAgentCompleted(const FString& AgentName);

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World")
    void InitializeBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "World")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World")
    FString GetBiomeAtLocation(const FVector& Location);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(const FString& Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCriticalBlocker(const FString& BlockerDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool HasCriticalBlockers() const;

    // Development Tools
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ValidateProjectStructure();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "World")
    TArray<FDir_BiomeCoordinates> BiomeCoordinates;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalBlockers;

private:
    void SetupDefaultAgents();
    void SetupBiomeData();
    int32 CountPhantomHeaders();
    void LogProductionStatus();
};