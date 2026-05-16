#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "ProductionPipeline.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionStage : uint8
{
    EngineArchitecture      UMETA(DisplayName = "Engine Architecture"),
    CoreSystems            UMETA(DisplayName = "Core Systems"),
    WorldGeneration        UMETA(DisplayName = "World Generation"),
    EnvironmentArt         UMETA(DisplayName = "Environment Art"),
    CharacterCreation      UMETA(DisplayName = "Character Creation"),
    AnimationSystem        UMETA(DisplayName = "Animation System"),
    NPCBehavior           UMETA(DisplayName = "NPC Behavior"),
    CombatAI              UMETA(DisplayName = "Combat AI"),
    QuestSystem           UMETA(DisplayName = "Quest System"),
    AudioIntegration      UMETA(DisplayName = "Audio Integration"),
    VFXIntegration        UMETA(DisplayName = "VFX Integration"),
    QATesting             UMETA(DisplayName = "QA Testing"),
    FinalIntegration      UMETA(DisplayName = "Final Integration")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle                  UMETA(DisplayName = "Idle"),
    Active                UMETA(DisplayName = "Active"),
    Waiting               UMETA(DisplayName = "Waiting for Dependencies"),
    Blocked               UMETA(DisplayName = "Blocked"),
    Completed             UMETA(DisplayName = "Completed"),
    Error                 UMETA(DisplayName = "Error")
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
    EDir_ProductionStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        CurrentStage = EDir_ProductionStage::EngineArchitecture;
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    int32 TargetActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    bool bIsPopulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Status")
    TArray<FString> AssetTypes;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        Location = FVector::ZeroVector;
        ActorCount = 0;
        TargetActorCount = 500;
        bIsPopulated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionPipeline : public AActor
{
    GENERATED_BODY()

public:
    AProductionPipeline();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    bool bPipelineActive;

    // Core Pipeline Functions
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void InitializePipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void UpdateBiomeStatus(const FString& BiomeName, int32 NewActorCount);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool CanAgentProceed(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void AdvanceToNextStage(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    float CalculateOverallProgress();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void RegisterBiome(const FString& BiomeName, FVector Location, int32 TargetActors);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool IsBiomePopulated(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    TArray<FString> GetUnpopulatedBiomes();

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    void LogPipelineStatus();

    UFUNCTION(BlueprintCallable, Category = "Production Pipeline")
    bool ValidateMinimumViablePrototype();

private:
    void SetupAgentDependencies();
    void CheckDependencies();
    void UpdateWorldMetrics();
    
    UPROPERTY()
    float LastUpdateTime;
};

#include "ProductionPipeline.generated.h"