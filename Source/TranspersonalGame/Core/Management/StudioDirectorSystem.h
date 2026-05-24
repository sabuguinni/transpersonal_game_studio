#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Swamp_SW,
    Forest_NW,
    Savanna_Center,
    Desert_E,
    Mountains_NE
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    EDir_BiomeType AssignedBiome = EDir_BiomeType::Savanna_Center;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    float TaskProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentTaskInfo()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurActorsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TerrainActorsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VegetationActorsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime LastBuildTime;

    FDir_ProductionMetrics()
    {
        LastBuildTime = FDateTime::Now();
    }
};

/**
 * Studio Director System - Central coordination and oversight for all 19 agents
 * Manages task distribution, progress tracking, and production pipeline coordination
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

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(int32 AgentID, const FString& AgentName, EDir_BiomeType AssignedBiome);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& CurrentTask, float Progress = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTaskInfo GetAgentInfo(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTaskInfo> GetAllAgentInfo() const;

    // Production oversight
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    // Biome coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetBiomeCoordinates(EDir_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<AActor*> GetActorsInBiome(EDir_BiomeType BiomeType, float Radius = 2000.0f) const;

    // Task management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_BiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsAgentAvailable(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    int32 GetNextAvailableAgent() const;

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void DebugPrintAgentStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void CreateCoordinationMarkers();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void ValidateProductionPipeline();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TMap<int32, FDir_AgentTaskInfo> AgentRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TMap<EDir_BiomeType, FVector> BiomeCoordinates;

    // Internal methods
    void InitializeBiomeCoordinates();
    void CountActorsByType();
    void LogProductionStatus() const;

private:
    // Timer for periodic updates
    FTimerHandle MetricsUpdateTimer;
    
    void PeriodicMetricsUpdate();
};