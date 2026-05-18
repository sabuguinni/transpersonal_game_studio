#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction    UMETA(DisplayName = "Pre-Production"),
    CoreSystems      UMETA(DisplayName = "Core Systems"),
    WorldBuilding    UMETA(DisplayName = "World Building"),
    CharacterSystems UMETA(DisplayName = "Character Systems"),
    GameplaySystems  UMETA(DisplayName = "Gameplay Systems"),
    Polish           UMETA(DisplayName = "Polish"),
    Testing          UMETA(DisplayName = "Testing"),
    Release          UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> DependsOnAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString BlockingReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualHours;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        RequiredPhase = EDir_ProductionPhase::PreProduction;
        bIsCompleted = false;
        bIsBlocked = false;
        BlockingReason = TEXT("");
        EstimatedHours = 0.0f;
        ActualHours = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 PropCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bTerrainComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bLightingComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bPopulationComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float CompletionPercentage;

    FDir_BiomeProgress()
    {
        BiomeName = TEXT("");
        BiomeLocation = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        VegetationCount = 0;
        PropCount = 0;
        bTerrainComplete = false;
        bLightingComplete = false;
        bPopulationComplete = false;
        CompletionPercentage = 0.0f;
    }
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_BiomeProgress> BiomeProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalDinosaursSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bMinPlayableMapReady;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void BlockAgentTask(int32 AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateBiomeProgress(const FString& BiomeName, int32 ActorCount, int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ScanWorldActors();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetAgentTask(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetCriticalTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(CallInEditor, Category = "Production")
    void RefreshProductionStatus();

private:
    void SetupDefaultAgentTasks();
    void SetupDefaultBiomes();
    FDir_BiomeProgress* FindBiomeProgress(const FString& BiomeName);
    bool CheckAgentDependencies(int32 AgentNumber) const;
};

#include "ProductionCoordinator.generated.h"