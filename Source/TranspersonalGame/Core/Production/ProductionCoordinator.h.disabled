#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production Coordinator - Studio Director's central coordination system
 * Tracks agent progress, manages production milestones, and coordinates development cycles
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    CharacterDev    UMETA(DisplayName = "Character Development"),
    GameplayLoop    UMETA(DisplayName = "Gameplay Loop"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Release         UMETA(DisplayName = "Release Ready")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime DeadlineTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("Unknown");
        TaskDescription = TEXT("No task assigned");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototype;

    FDir_ProductionMetrics()
    {
        CycleNumber = 25;
        CompletedTasks = 0;
        TotalTasks = 0;
        OverallProgress = 0.0f;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        bPlayablePrototype = false;
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
    class UTextRenderComponent* StatusDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics Metrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bAutoUpdateDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float UpdateInterval;

private:
    FTimerHandle UpdateTimerHandle;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeCycle025Tasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void RefreshStatusDisplay();

protected:
    UFUNCTION()
    void UpdateDisplayTimer();

    void UpdateStatusText();
    FString GetPhaseDisplayName() const;
    FString GetStatusDisplayName(EDir_AgentStatus Status) const;
};