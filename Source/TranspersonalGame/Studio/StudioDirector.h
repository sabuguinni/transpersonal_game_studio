#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Error       UMETA(DisplayName = "Error"),
    Waiting     UMETA(DisplayName = "Waiting")
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountains   UMETA(DisplayName = "Mountains")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_BiomeType AssignedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CreatedTime;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        AssignedBiome = EDir_BiomeType::Savanna;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        CreatedTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    EDir_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    TArray<int32> AssignedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    bool bIsPopulated;

    FDir_BiomeStatus()
    {
        BiomeType = EDir_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        CompletionPercentage = 0.0f;
        bIsPopulated = false;
    }
};

/**
 * Studio Director - Central coordination system for the 19-agent production pipeline
 * Manages task distribution, biome population tracking, and agent status monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CommandCenterMesh;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float ProductionProgress;

public:
    virtual void Tick(float DeltaTime) override;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentTask(int32 AgentNumber, const FString& AgentName, const FString& TaskDescription, 
                        EDir_BiomeType AssignedBiome, FVector TargetLocation, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByAgent(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByBiome(EDir_BiomeType BiomeType);

    // Biome Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeActorCount(EDir_BiomeType BiomeType, int32 NewActorCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_BiomeStatus GetBiomeStatus(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsBiomePopulated(EDir_BiomeType BiomeType, int32 MinimumActors = 500);

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionProgress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogAgentActivity(int32 AgentNumber, const FString& Activity);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetBiomeCoordinates(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetAgentsForBiome(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void RefreshWorldStatus();

private:
    void SetupCommandCenterMesh();
    void CreateInitialTasks();
    float CalculateOverallProgress();
};

#include "StudioDirector.generated.h"