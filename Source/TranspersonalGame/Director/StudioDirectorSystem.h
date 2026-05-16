#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

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
enum class EDir_BiomePriority : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    NeedsValidation UMETA(DisplayName = "Needs Validation"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked")
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
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        Dependencies = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D Coordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<int32> ResponsibleAgents;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        Coordinates = FVector2D::ZeroVector;
        ActorCount = 0;
        bIsPopulated = false;
        Priority = EDir_BiomePriority::Low;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> BlockingAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Target;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
        Target = TEXT("");
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Agent Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedAgents();

    // Biome Management Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(const FString& BiomeName, int32 ActorCount, bool bPopulated);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_BiomeStatus GetBiomeStatus(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeStatus> GetUnpopulatedBiomes();

    // Production Tracking Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(const FString& MilestoneName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> GetBlockedMilestones();

    // Validation and Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void RefreshAllStatuses();

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultMilestones();
    void ScanWorldForActors();
    
    UPROPERTY()
    float LastUpdateTime;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStudioDirectorComponent* DirectorComponent;

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnAgentStatusChanged(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnMilestoneCompleted(const FString& MilestoneName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnBiomePopulated(const FString& BiomeName);
};