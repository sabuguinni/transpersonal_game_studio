#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director's Production Coordinator
 * Manages critical path execution and agent task distribution
 * Ensures playable prototype milestones are met
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float EstimatedHours;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bIsCompleted = false;
        EstimatedHours = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsOverLimit;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        bIsOverLimit = false;
    }
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    MinimumViablePrototype,
    CoreGameplayLoop,
    ContentExpansion,
    Polish,
    Release
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    // Critical Path Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bPlayerCharacterExists;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bTerrainGenerated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bDinosaursSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bSurvivalHUDActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bBasicLightingSetup;

    // Actor Limits (from Hugo's directives)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxPropsPerBiome;

public:
    // Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDesc, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    // Critical Path Functions
    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    void ValidateCriticalPath();

    UFUNCTION(BlueprintCallable, Category = "Critical Path")
    bool IsMinimumViablePrototypeReady() const;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateBiomeStatuses();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector GetOptimalSpawnLocationForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CanSpawnInBiome(const FString& BiomeName) const;

    // Actor Limit Enforcement
    UFUNCTION(BlueprintCallable, Category = "Limits")
    bool IsWithinActorLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Limits")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Limits")
    int32 GetCurrentActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Limits")
    int32 GetCurrentDinosaurCount() const;

private:
    void InitializeBiomes();
    void InitializeProductionTasks();
    FDir_BiomeStatus* FindBiomeStatus(const FString& BiomeName);
};