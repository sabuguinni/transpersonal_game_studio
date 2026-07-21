#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype      UMETA(DisplayName = "Prototype"),
    Alpha          UMETA(DisplayName = "Alpha"),
    Beta           UMETA(DisplayName = "Beta"),
    Gold           UMETA(DisplayName = "Gold Master"),
    PostLaunch     UMETA(DisplayName = "Post Launch")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Working        UMETA(DisplayName = "Working"),
    Blocked        UMETA(DisplayName = "Blocked"),
    Complete       UMETA(DisplayName = "Complete"),
    Failed         UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FilesProduced = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastActivity;

    FDir_AgentTaskData()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        FilesProduced = 0;
        UE5CommandsExecuted = 0;
        LastActivity = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TerrainActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 LightActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float PerformanceFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    FDir_ProductionMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        TerrainActors = 0;
        LightActors = 0;
        PerformanceFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

/**
 * Production Manager - Coordinates development workflow between AI agents
 * Tracks production metrics, agent status, and enforces development priorities
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentProgress(int32 AgentID, float CompletionPercentage, int32 FilesProduced, int32 UE5Commands);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTaskData GetAgentData(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTaskData> GetAllAgentData() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Priority Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetHighPriorityAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetAgentPriority(int32 AgentID, int32 Priority);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void EnforceActorCaps();

    // Development Phase Validation
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAdvanceToNextPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetPhaseRequirements() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<int32, FDir_AgentTaskData> AgentRegistry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<int32, int32> AgentPriorities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 MaxActorCount = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 MaxDinosaurCount = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float MinTargetFPS = 30.0f;

private:
    void InitializeAgentRegistry();
    void ValidateWorldState();
    void CalculateProductionProgress();
};

#include "Dir_ProductionManager.generated.h"