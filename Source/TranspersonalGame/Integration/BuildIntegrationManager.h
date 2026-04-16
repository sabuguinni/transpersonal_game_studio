#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ErrorMessage = TEXT("");
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildTimestamp;

    FBuild_IntegrationReport()
    {
        TotalClasses = 0;
        LoadedClasses = 0;
        FailedClasses = 0;
        bBuildSuccessful = false;
        BuildTimestamp = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport RunFullIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegration(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetLoadedModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateGameAssets();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetMissingAssets();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetCurrentFrameRate();

    // Compilation status
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsProjectCompiled();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetLastCompilationError();

protected:
    // Internal validation functions
    bool ValidateModuleClasses(const FString& ModuleName);
    bool TestSystemInteractions();
    void LogIntegrationResults(const FBuild_IntegrationReport& Report);

private:
    UPROPERTY()
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY()
    TArray<FString> KnownModules;

    UPROPERTY()
    TArray<FString> CriticalAssets;

    // Performance tracking
    UPROPERTY()
    float TargetFrameRate;

    UPROPERTY()
    float MinimumFrameRate;

    // Build state
    UPROPERTY()
    bool bLastBuildSuccessful;

    UPROPERTY()
    FString LastBuildTimestamp;
};