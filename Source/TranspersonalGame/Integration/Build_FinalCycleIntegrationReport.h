#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Build_FinalCycleIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Critical        UMETA(DisplayName = "Critical Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleIntegrationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasValidation;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

    FBuild_ModuleIntegrationData()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasValidation = false;
        IntegrationScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float OverallIntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageEstimate;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bWithinActorLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bWithinDinosaurLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EBuild_IntegrationStatus Status;

    FBuild_CycleIntegrationMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        LoadedModules = 0;
        OverallIntegrationScore = 0.0f;
        MemoryUsageEstimate = 0.0f;
        bWithinActorLimits = true;
        bWithinDinosaurLimits = true;
        Status = EBuild_IntegrationStatus::Unknown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleIntegrationReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleIntegrationReport();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FDateTime ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FBuild_CycleIntegrationMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_ModuleIntegrationData> ModuleData;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> Recommendations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bReadyForHandoff;

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ValidateModuleIntegration(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void CalculateIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void CheckSystemLimits();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void AddCriticalIssue(const FString& Issue);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void AddRecommendation(const FString& Recommendation);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    bool IsIntegrationSuccessful() const;

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    FString GetReportSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ExportReportToFile(const FString& FilePath);

private:
    void InitializeReport();
    void ValidateWorldState();
    void CheckModuleDependencies();
    void AnalyzePerformanceMetrics();
    void GenerateRecommendations();
};