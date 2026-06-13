#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Active      UMETA(DisplayName = "Active"),
    Pending     UMETA(DisplayName = "Pending"),
    Monitoring  UMETA(DisplayName = "Monitoring"),
    Standby     UMETA(DisplayName = "Standby"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

USTRUCT(BlueprintType)
struct FDir_AgentMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 TasksCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    int32 TasksFailed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    float SuccessRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Metrics")
    FDateTime LastUpdate;

    FDir_AgentMetrics()
    {
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Ready;
        TasksCompleted = 0;
        TasksFailed = 0;
        SuccessRate = 100.0f;
        CurrentTask = TEXT("");
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float PrototypeProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString BuildStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> CriticalSystemsOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FString> PendingSystems;

    FDir_ProductionMetrics()
    {
        CurrentPhase = EDir_ProductionPhase::Prototype;
        PrototypeProgress = 65.0f;
        TotalActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        BuildStatus = TEXT("STABLE");
        TargetFPS = 60.0f;
        CriticalSystemsOnline.Add(TEXT("Movement"));
        CriticalSystemsOnline.Add(TEXT("Terrain"));
        CriticalSystemsOnline.Add(TEXT("Basic_Survival"));
        PendingSystems.Add(TEXT("Dinosaur_AI"));
        PendingSystems.Add(TEXT("Combat"));
        PendingSystems.Add(TEXT("Advanced_Survival"));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentMetrics> AgentRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    FDir_ProductionMetrics ProductionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    bool bAutoUpdateMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    float UpdateInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeAgentRegistry();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, const FString& CurrentTask);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void RecordTaskCompletion(const FString& AgentName, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    FDir_AgentMetrics GetAgentMetrics(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    float GetOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    TArray<FString> GetPendingTasks();

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production Management")
    void DebugPrintAllMetrics();

private:
    float LastUpdateTime;
    
    void CountLevelActors();
    FDir_AgentMetrics* FindAgentMetrics(const FString& AgentName);
};