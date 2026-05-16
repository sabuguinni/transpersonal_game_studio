#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FVector TaskLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString LastOutput;

    FDir_AgentTaskInfo()
    {
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        TaskLocation = FVector::ZeroVector;
        LastOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CharacterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TMap<FString, int32> BiomeActorCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalActorsInWorld = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        TerrainActors = 0;
        OverallProgress = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTaskInfo> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float MetricsUpdateInterval;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTaskInfo GetAgentTaskInfo(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateTaskMarkers();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateBiomePopulation();

private:
    float LastMetricsUpdate;
    
    void CountActorsByType();
    void CountActorsByBiome();
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bShowDebugInfo;

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnProductionMetricsUpdated(const FDir_ProductionMetrics& Metrics);

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnAgentStatusChanged(const FString& AgentName, EDir_AgentStatus NewStatus);
};