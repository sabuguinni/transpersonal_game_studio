#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_CycleCompletionValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_CycleStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    RequiresReview  UMETA(DisplayName = "Requires Review")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 TotalAgents = 19;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 CompletedAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 FailedAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    float CycleCompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 SourceFilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    bool bAllCriticalSystemsOnline = false;

    FBuild_CycleMetrics()
    {
        TotalAgents = 19;
        CompletedAgents = 0;
        FailedAgents = 0;
        CycleCompletionPercentage = 0.0f;
        SourceFilesCreated = 0;
        UE5CommandsExecuted = 0;
        bAllCriticalSystemsOnline = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CycleCompletionValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CycleCompletionValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cycle validation functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void ValidateCurrentCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    EBuild_CycleStatus GetCycleStatus() const { return CurrentCycleStatus; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    FBuild_CycleMetrics GetCycleMetrics() const { return CycleMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    bool ValidateAgentOutput(int32 AgentNumber, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void MarkCycleComplete();

    UFUNCTION(BlueprintCallable, Category = "Cycle Validation")
    void ReportCycleFailure(const FString& FailureReason);

    // Critical system validation
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "System Validation")
    TArray<FString> GetFailedSystems() const { return FailedSystems; }

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Cycle Status")
    EBuild_CycleStatus CurrentCycleStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FBuild_CycleMetrics CycleMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Failed Systems")
    TArray<FString> FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

private:
    // Internal validation functions
    bool ValidateSourceFiles();
    bool ValidateModuleLoading();
    bool ValidateGameplayElements();
    bool ValidateIntegrationHealth();
    
    void UpdateCycleMetrics();
    void LogValidationResults();
    
    // Timing and performance
    float CycleStartTime;
    float LastValidationTime;
    
    // Critical system references
    TArray<FString> CriticalSystemNames;
    TMap<FString, bool> SystemValidationResults;
};