#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Error,
    Waiting
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savanna,
    Swamp,
    Forest,
    Desert,
    Mountain
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    FDir_AgentInfo()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        ProgressPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType = EDir_BiomeType::Savanna;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated = false;

    UPROPERTY(BlueprintReadWrite, Category = "Biome")
    TArray<FString> RequiredAssets;

    FDir_BiomeInfo()
    {
        BiomeType = EDir_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        ActorCount = 0;
        bIsPopulated = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 CompletedSystems = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        TotalActorsInWorld = 0;
        CompletedSystems = 0;
        OverallProgress = 0.0f;
        ActiveAgents = 0;
        CurrentMilestone = TEXT("Milestone 1: Walk Around");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Studio Management")
    TArray<FDir_AgentInfo> AgentList;

    UPROPERTY(BlueprintReadWrite, Category = "Studio Management")
    TArray<FDir_BiomeInfo> BiomeList;

    UPROPERTY(BlueprintReadWrite, Category = "Studio Management")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Studio Management")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadWrite, Category = "Studio Management")
    bool bIsProductionActive = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void InitializeAgentSystem();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void UpdateBiomeInfo(EDir_BiomeType BiomeType, int32 NewActorCount, bool bPopulated);

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    FDir_ProductionMetrics CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    TArray<FDir_AgentInfo> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Management", CallInEditor = true)
    void RefreshWorldState();

    UFUNCTION(BlueprintCallable, Category = "Studio Management")
    void SetCurrentCycle(const FString& CycleID);

private:
    void InitializeBiomeData();
    void InitializeAgentData();
    void UpdateWorldMetrics();
    
    float LastUpdateTime = 0.0f;
    const float UpdateInterval = 5.0f;
};