#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Build_IntegrationCycleReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_CycleStatus : uint8
{
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Skipped         UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EBuild_AgentType : uint8
{
    StudioDirector          UMETA(DisplayName = "Studio Director"),
    EngineArchitect         UMETA(DisplayName = "Engine Architect"),
    CoreSystems             UMETA(DisplayName = "Core Systems"),
    PerformanceOptimizer    UMETA(DisplayName = "Performance Optimizer"),
    ProceduralWorld         UMETA(DisplayName = "Procedural World"),
    EnvironmentArtist       UMETA(DisplayName = "Environment Artist"),
    Architecture            UMETA(DisplayName = "Architecture"),
    LightingAtmosphere      UMETA(DisplayName = "Lighting & Atmosphere"),
    CharacterArtist         UMETA(DisplayName = "Character Artist"),
    Animation               UMETA(DisplayName = "Animation"),
    NPCBehavior             UMETA(DisplayName = "NPC Behavior"),
    CombatEnemyAI           UMETA(DisplayName = "Combat & Enemy AI"),
    CrowdTraffic            UMETA(DisplayName = "Crowd & Traffic"),
    QuestMission            UMETA(DisplayName = "Quest & Mission"),
    NarrativeDialogue       UMETA(DisplayName = "Narrative & Dialogue"),
    Audio                   UMETA(DisplayName = "Audio"),
    VFX                     UMETA(DisplayName = "VFX"),
    QATesting               UMETA(DisplayName = "QA & Testing"),
    IntegrationBuild        UMETA(DisplayName = "Integration & Build")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_AgentCycleData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    EBuild_AgentType AgentType;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    EBuild_CycleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    float ExecutionTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    TArray<FString> CreatedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Agent Data")
    FString CycleNotes;

    FBuild_AgentCycleData()
    {
        AgentType = EBuild_AgentType::StudioDirector;
        Status = EBuild_CycleStatus::InProgress;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        ExecutionTimeSeconds = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalAgentsCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalUE5Commands;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float TotalExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActorsInScene;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalDinosaursInScene;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bAllValidationsPassed;

    FBuild_CycleMetrics()
    {
        TotalAgentsCompleted = 0;
        TotalFilesCreated = 0;
        TotalUE5Commands = 0;
        TotalExecutionTime = 0.0f;
        TotalActorsInScene = 0;
        TotalDinosaursInScene = 0;
        bCompilationSuccessful = false;
        bAllValidationsPassed = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_IntegrationCycleReport : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cycle Info")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cycle Info")
    FDateTime CycleStartTime;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cycle Info")
    FDateTime CycleEndTime;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cycle Info")
    EBuild_CycleStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Agent Data")
    TArray<FBuild_AgentCycleData> AgentData;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Metrics")
    FBuild_CycleMetrics CycleMetrics;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Integration")
    TArray<FString> IntegrationIssues;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Integration")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Integration")
    TArray<FString> ValidationFailures;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Integration")
    FString FinalNotes;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void AddAgentData(const FBuild_AgentCycleData& Data);

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void UpdateCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void FinalizeCycle(EBuild_CycleStatus FinalStatus);

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    float GetCycleDurationSeconds() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    bool IsSuccessfulCycle() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    FString GenerateSummaryReport() const;
};