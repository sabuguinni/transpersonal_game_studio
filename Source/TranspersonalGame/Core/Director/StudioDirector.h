#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/NoExportTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Central coordination system for game development and production management
 * Manages agent task distribution, compilation monitoring, and production milestones
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"), 
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Error           UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    Integration     UMETA(DisplayName = "Integration"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector MinBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector MaxBounds;

    FDir_BiomeCoordinates()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        MinBounds = FVector::ZeroVector;
        MaxBounds = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirector : public UObject
{
    GENERATED_BODY()

public:
    UStudioDirector();

    // Core Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetRandomSpawnLocationInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_BiomeCoordinates GetBiomeByName(const FString& BiomeName) const;

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayablePrototype() const;

    // Development Tools
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void SpawnTestActorsInAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void CleanOrphanedHeaders();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void VerifyAgentProgress();

protected:
    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentCycleID;

    // Biome Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeCoordinates> BiomeCoordinates;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 OrphanedHeaderCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CompilationErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float ProductionEfficiency;

    // Internal Functions
    void SetupDefaultBiomes();
    void LogProductionMetrics() const;
    bool IsValidSpawnLocation(const FVector& Location) const;
};