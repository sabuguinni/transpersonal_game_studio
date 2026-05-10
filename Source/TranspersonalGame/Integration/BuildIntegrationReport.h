#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float CompilationTimeSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildConfiguration;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime CompilationTimestamp;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasValidClasses = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasDependencyIssues = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> MissingDependencies;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_CompilationResult CompilationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleIntegrationStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsIntegrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalModulesChecked = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SuccessfulModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Recommendations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ReportGeneratedAt;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationReportSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemIntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegration(const FString& ModuleName, FBuild_ModuleIntegrationStatus& OutStatus);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckCompilationStatus(FBuild_CompilationResult& OutResult);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveReportToFile(const FBuild_SystemIntegrationReport& Report, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetAvailableModules() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RefreshModuleStatus();

protected:
    UPROPERTY()
    FBuild_SystemIntegrationReport LastGeneratedReport;

    UPROPERTY()
    TMap<FString, FBuild_ModuleIntegrationStatus> CachedModuleStatuses;

    UPROPERTY()
    bool bReportCacheValid = false;

private:
    void ValidateAllModules();
    void CheckClassAvailability(const FString& ModuleName, FBuild_ModuleIntegrationStatus& Status);
    void AnalyzeDependencies(const FString& ModuleName, FBuild_ModuleIntegrationStatus& Status);
    FString GetModuleStatusSummary(const FBuild_ModuleIntegrationStatus& Status) const;
};