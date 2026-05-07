#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Production Coordination System
 * Manages agent task assignments and production tracking
 * Ensures coordination between all 19 specialized agents
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    EDir_Priority Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> TaskList;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedDeliverables;

    UPROPERTY(BlueprintReadWrite, Category = "Agent Task")
    bool bCompleted;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        Priority = EDir_Priority::Medium;
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    FString CycleID;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    TArray<FString> CriticalBlockers;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bPlayablePrototype;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bCharacterMovement;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bDinosaurEncounters;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bSurvivalMechanics;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bBiomeDiversity;

    FDir_ProductionStatus()
    {
        CycleID = TEXT("");
        bPlayablePrototype = false;
        bCharacterMovement = false;
        bDinosaurEncounters = false;
        bSurvivalMechanics = false;
        bBiomeDiversity = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MarkTaskCompleted(const FString& AgentName, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionStatus GetProductionStatus() const;

    // Critical System Checks
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckPlayablePrototypeStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalBlockers();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateSuccessMetrics();

    // Biome Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FVector GetBiomeCenter(EDir_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FVector> GetBiomeSpawnPoints(EDir_BiomeType BiomeType, int32 NumPoints);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionStatus CurrentProductionStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, FDir_AgentTask> AgentTaskMap;

    // Biome coordinate data
    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    TMap<EDir_BiomeType, FVector> BiomeCenters;

private:
    void InitializeBiomeCoordinates();
    void CheckCompilationStatus();
    void AssessWorldState();
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    class UStaticMeshComponent* DirectorConsole;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director")
    class UWidgetComponent* ProductionDisplay;

public:
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DisplayProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerAgentCoordination();
};