#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning,
    PreProduction,
    Production,
    Alpha,
    Beta,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Error
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdateTime;

    FDir_AgentInfo()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
        LastUpdateTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime CompletionDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        TargetDate = FDateTime::Now();
        CompletionDate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ExpectedDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 CurrentDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated = false;

    FDir_BiomeInfo()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        ExpectedDinosaurCount = 0;
        CurrentDinosaurCount = 0;
        bIsPopulated = false;
    }
};

/**
 * Studio Director System - Manages the entire production pipeline
 * Coordinates 18 AI agents and tracks production milestones
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsAgentBlocked(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetAllAgentInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetBlockedAgents() const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> GetAllMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMilestone GetMilestone(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AreDependenciesMet(const FString& MilestoneName) const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterBiome(const FString& BiomeName, const FVector& Location, float Radius, int32 ExpectedDinosaurs);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomePopulation(const FString& BiomeName, int32 CurrentDinosaurs);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeInfo> GetAllBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetRandomLocationInBiome(const FString& BiomeName) const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateProductionState() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetProductionIssues() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentInfo> AgentList;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeInfo> Biomes;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDateTime ProductionStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    int32 TotalCycles;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    int32 CompletedCycles;

private:
    void SetupAgentList();
    void SetupMilestones();
    void SetupBiomes();
    void ValidateAgentDependencies();
    bool CheckCompilationStatus() const;
    void LogProductionMetrics() const;
};