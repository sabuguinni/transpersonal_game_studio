#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "DirectorCoordinationSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Swamp_SW        UMETA(DisplayName = "Swamp Southwest"),
    Forest_NW       UMETA(DisplayName = "Forest Northwest"),
    Savanna_Center  UMETA(DisplayName = "Savanna Center"),
    Desert_E        UMETA(DisplayName = "Desert East"),
    Mountains_NE    UMETA(DisplayName = "Mountains Northeast"),
    Unknown         UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Error           UMETA(DisplayName = "Error"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_BiomeCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType = EDir_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 AssignedAgentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = false;

    FDir_BiomeCoordination()
    {
        BiomeType = EDir_BiomeType::Unknown;
        Location = FVector::ZeroVector;
        Radius = 2000.0f;
        AssignedAgentCount = 0;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_BiomeType AssignedBiome = EDir_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float TaskProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentCoordination()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        AssignedBiome = EDir_BiomeType::Unknown;
        CurrentTask = TEXT("");
        TaskProgress = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Director Coordination System - Central command and control for all 19 agents
 * Manages biome assignments, agent status tracking, and production pipeline coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADirectorCoordinationSystem : public AActor
{
    GENERATED_BODY()

public:
    ADirectorCoordinationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* CommandPostLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CommandPostMesh;

    // Biome Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FDir_BiomeCoordination> BiomeCoordinationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EDir_BiomeType, AActor*> BiomeMarkers;

    // Agent Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentCoordination> AgentCoordinationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    int32 TotalAgents = 19;

    // Production Pipeline
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bPipelineActive = false;

public:
    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_BiomeCoordination GetBiomeData(EDir_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateBiomeStatus(EDir_BiomeType BiomeType, bool bActive, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Director")
    EDir_BiomeType GetOptimalBiomeForAgent(int32 AgentID) const;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeAgents();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& Task, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_AgentCoordination GetAgentData(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentCoordination> GetAgentsByStatus(EDir_AgentStatus Status) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignAgentToBiome(int32 AgentID, EDir_BiomeType BiomeType);

    // Production Pipeline
    UFUNCTION(BlueprintCallable, Category = "Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateCycleProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsCycleActive() const { return bPipelineActive; }

    // Coordination Utilities
    UFUNCTION(BlueprintCallable, Category = "Director")
    void CreateBiomeMarker(EDir_BiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateMarkerVisuals();

    UFUNCTION(BlueprintCallable, Category = "Director")
    FString GenerateStatusReport() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Director")
    void DebugPrintStatus();

private:
    // Internal coordination logic
    void UpdateCoordinationTick(float DeltaTime);
    void ValidateAgentAssignments();
    void OptimizeBiomeDistribution();
    
    // Utility functions
    FString BiomeTypeToString(EDir_BiomeType BiomeType) const;
    FString AgentStatusToString(EDir_AgentStatus Status) const;
    FLinearColor GetStatusColor(EDir_AgentStatus Status) const;
};