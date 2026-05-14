#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Coordinates all game systems and agent outputs
 * Manages the integration of 19 specialized agent outputs into cohesive gameplay
 */

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Status")
    int32 CompletedCycles;

    FDir_AgentStatus()
    {
        AgentName = TEXT("Unknown");
        bIsActive = false;
        LastUpdateTime = 0.0f;
        CurrentTask = TEXT("Idle");
        CompletedCycles = 0;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeCoordination
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    float Radius;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<FString> AssignedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsReady;

    FDir_BiomeCoordination()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 5000.0f;
        bIsReady = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeCoordination> BiomeCoordinations;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentCoordination();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, const FString& Task, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterBiome(const FString& BiomeName, FVector Location, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateGameplayReadiness();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAgentTasks();

    // Asset pipeline coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckBiomeReadiness();

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void DebugPrintAgentStatuses();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void ValidateMinPlayableMap();

private:
    // Internal coordination state
    float LastCoordinationUpdate;
    bool bSystemInitialized;

    // Helper functions
    void SetupDefaultAgents();
    void SetupDefaultBiomes();
    FDir_AgentStatus* FindAgentStatus(const FString& AgentName);
    FDir_BiomeCoordination* FindBiome(const FString& BiomeName);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    UStudioDirectorComponent* DirectorComponent;

    // Command and control interface
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ExecuteAgentCoordination();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MonitorGameplayState();
};

#include "StudioDirectorSystem.generated.h"