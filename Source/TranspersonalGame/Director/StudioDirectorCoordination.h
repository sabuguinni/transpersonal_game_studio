#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "StudioDirectorCoordination.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    LOW         UMETA(DisplayName = "Low Priority"),
    MEDIUM      UMETA(DisplayName = "Medium Priority"),
    HIGH        UMETA(DisplayName = "High Priority"),
    CRITICAL    UMETA(DisplayName = "Critical Priority")
};

UENUM(BlueprintType)
enum class EDir_BiomeStatus : uint8
{
    EMPTY       UMETA(DisplayName = "Empty"),
    PENDING     UMETA(DisplayName = "Pending Population"),
    POPULATED   UMETA(DisplayName = "Populated"),
    COMPLETE    UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    NOT_STARTED UMETA(DisplayName = "Not Started"),
    IN_PROGRESS UMETA(DisplayName = "In Progress"),
    COMPLETED   UMETA(DisplayName = "Completed"),
    BLOCKED     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentPriority Priority = EDir_AgentPriority::MEDIUM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_TaskStatus Status = EDir_TaskStatus::NOT_STARTED;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CycleID;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::MEDIUM;
        Status = EDir_TaskStatus::NOT_STARTED;
        CompletionPercentage = 0.0f;
        CycleID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Coordinates = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomeStatus Status = EDir_BiomeStatus::EMPTY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> RequiredAssetTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float PopulationProgress = 0.0f;

    FDir_BiomeData()
    {
        BiomeName = TEXT("");
        Coordinates = FVector::ZeroVector;
        ActorCount = 0;
        Status = EDir_BiomeStatus::EMPTY;
        PopulationProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_PrototypeValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bPlayerMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bCameraSystem = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bTerrainExists = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bDinosaursSpawned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bLightingSetup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bSurvivalHUD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    float OverallCompletion = 0.0f;

    FDir_PrototypeValidation()
    {
        bPlayerMovement = false;
        bCameraSystem = false;
        bTerrainExists = false;
        bDinosaursSpawned = false;
        bLightingSetup = false;
        bSurvivalHUD = false;
        OverallCompletion = 0.0f;
    }
};

/**
 * Studio Director Coordination System
 * Manages task assignment and progress tracking for all 18 agents
 * Ensures coherent execution of the playable prototype development
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_StudioDirectorCoordination : public UObject
{
    GENERATED_BODY()

public:
    UDir_StudioDirectorCoordination();

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentProgress(int32 AgentID, EDir_TaskStatus Status, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAgentTasks() const { return AgentTasks; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(const FString& BiomeName, int32 ActorCount, EDir_BiomeStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeData> GetBiomeStatus() const { return BiomeData; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_BiomeData GetBiomeData(const FString& BiomeName) const;

    // Prototype Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_PrototypeValidation ValidatePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPrototypePlayable() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress() const;

    // Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateStatusReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeData> BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_PrototypeValidation PrototypeStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FString> CriticalTasks;

private:
    void InitializeAgentTasks();
    void CalculateOverallProgress();
    int32 CountActorsInBiome(const FVector& BiomeCenter, float Radius = 10000.0f);
};