#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Dir_StudioDirector.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D XRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D YRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    FDir_BiomeData()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        XRange = FVector2D::ZeroVector;
        YRange = FVector2D::ZeroVector;
        Description = TEXT("");
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 OrphanHeaders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCompilationSuccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        OrphanHeaders = 0;
        bCompilationSuccess = false;
        CompilationErrors = 0;
        OverallProgress = 0.0f;
    }
};

/**
 * Studio Director Subsystem
 * Coordinates all 19 AI agents and manages production pipeline
 * Tracks biome distribution, agent tasks, and compilation status
 */
UCLASS()
class TRANSPERSONALGAME_API UDir_StudioDirector : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentTask(const FString& AgentName, float CompletionPercentage, bool bCompleted = false);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAgentTasks() const { return AgentTasks; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(const FString& AgentName) const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetRandomBiomeLocation(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeData> GetBiomeData() const { return BiomeData; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeActorCount(const FString& BiomeName, int32 NewCount);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const { return ProductionMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress() const;

    // Compilation Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerCompilationCheck();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsCompilationHealthy() const { return ProductionMetrics.bCompilationSuccess; }

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetHighPriorityAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetAgentPriority(const FString& AgentName, int32 NewPriority);

protected:
    UPROPERTY()
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY()
    TArray<FDir_BiomeData> BiomeData;

    UPROPERTY()
    FDir_ProductionMetrics ProductionMetrics;

private:
    void SetupDefaultAgentTasks();
    void SetupBiomeCoordinates();
    void CalculateOverallProgress();
};