#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalCycleIntegrationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystemsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SystemsLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 VFXSystemsIntegrated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CoreSystemsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCountInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString IntegrationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> LoadedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedSystems;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleCompletionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 AgentNumber = 19;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FDateTime CompletionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    bool bAllSystemsIntegrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FBuild_FinalIntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    TArray<FString> SuccessfulIntegrations;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleIntegrationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleIntegrationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ExecuteFinalCycleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_FinalIntegrationMetrics ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCoreGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLevelIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalCycleReport();

    // System Status Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetLoadedSystemsList();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetFailedSystemsList();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetIntegrationPercentage();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsIntegrationSuccessful();

    // Cycle Completion Functions
    UFUNCTION(BlueprintCallable, Category = "Cycle")
    void CompleteCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle")
    FBuild_CycleCompletionData GetCycleCompletionData();

    UFUNCTION(BlueprintCallable, Category = "Cycle")
    void LogCycleCompletion();

protected:
    // Internal validation functions
    bool ValidateSystemClass(const FString& ClassPath);
    void LogIntegrationResults();
    void UpdateIntegrationMetrics();

private:
    UPROPERTY()
    FBuild_FinalIntegrationMetrics CurrentMetrics;

    UPROPERTY()
    FBuild_CycleCompletionData CycleData;

    UPROPERTY()
    TArray<FString> CoreSystemPaths;

    UPROPERTY()
    TArray<FString> VFXSystemPaths;

    UPROPERTY()
    bool bIntegrationInitialized = false;

    UPROPERTY()
    bool bCycleCompleted = false;
};