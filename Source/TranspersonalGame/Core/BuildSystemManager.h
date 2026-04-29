#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildSystemManager.generated.h"

/**
 * Build System Manager - Manages compilation, linking and build processes
 * Enforces build rules, tracks dependencies, and ensures clean builds
 */

UENUM(BlueprintType)
enum class EEng_BuildStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

UENUM(BlueprintType)
enum class EEng_BuildTarget : uint8
{
    Development     UMETA(DisplayName = "Development"),
    Debug           UMETA(DisplayName = "Debug"),
    Shipping        UMETA(DisplayName = "Shipping"),
    Test            UMETA(DisplayName = "Test")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BuildInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EEng_BuildTarget Target;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float Duration;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> CompiledFiles;

    FEng_BuildInfo()
    {
        BuildName = TEXT("");
        Status = EEng_BuildStatus::NotStarted;
        Target = EEng_BuildTarget::Development;
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        Duration = 0.0f;
        ErrorCount = 0;
        WarningCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    bool bIsCore;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    int32 BuildOrder;

    FEng_ModuleDependency()
    {
        ModuleName = TEXT("");
        bIsCore = false;
        BuildOrder = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build System")
    void StartBuild(const FString& BuildName, EEng_BuildTarget Target = EEng_BuildTarget::Development);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void CancelBuild(const FString& BuildName);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void CompleteBuild(const FString& BuildName, EEng_BuildStatus FinalStatus);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    FEng_BuildInfo GetBuildInfo(const FString& BuildName) const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    TArray<FEng_BuildInfo> GetAllBuilds() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    TArray<FEng_BuildInfo> GetBuildsByStatus(EEng_BuildStatus Status) const;

    // Error and Warning Management
    UFUNCTION(BlueprintCallable, Category = "Build System")
    void AddBuildError(const FString& BuildName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void AddBuildWarning(const FString& BuildName, const FString& WarningMessage);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void AddCompiledFile(const FString& BuildName, const FString& FilePath);

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Build System")
    void RegisterModuleDependency(const FString& ModuleName, const TArray<FString>& Dependencies, bool bIsCore = false);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    TArray<FEng_ModuleDependency> GetModuleDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    TArray<FString> GetBuildOrder() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    bool ValidateDependencies(TArray<FString>& OutCircularDeps) const;

    // Build Rules and Validation
    UFUNCTION(BlueprintCallable, Category = "Build System")
    bool ValidateBuildRules(const FString& BuildName, TArray<FString>& OutViolations) const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    void SetBuildRule(const FString& RuleName, const FString& RuleValue);

    UFUNCTION(BlueprintCallable, Category = "Build System")
    FString GetBuildRule(const FString& RuleName) const;

    // Statistics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Build System")
    int32 GetTotalBuilds() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    int32 GetSuccessfulBuilds() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    int32 GetFailedBuilds() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    float GetAverageBuildTime() const;

    UFUNCTION(BlueprintCallable, Category = "Build System")
    FString GenerateBuildReport() const;

    // Debug and Utilities
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build System")
    void CleanBuildHistory();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build System")
    void ValidateAllDependencies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build System")
    void LogBuildStatistics();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    TMap<FString, FEng_BuildInfo> BuildHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    TArray<FEng_ModuleDependency> ModuleDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    TMap<FString, FString> BuildRules;

    UPROPERTY(BlueprintReadOnly, Category = "Build System")
    bool bInitialized;

private:
    void SetupDefaultBuildRules();
    void SetupDefaultModuleDependencies();
    bool HasCircularDependency(const FString& ModuleName, const TArray<FString>& VisitedModules) const;
    TArray<FString> CalculateBuildOrder() const;
};