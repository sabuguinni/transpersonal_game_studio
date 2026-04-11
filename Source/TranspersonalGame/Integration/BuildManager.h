#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "BuildManager.generated.h"

UENUM(BlueprintType)
enum class EBuildStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Warning
};

UENUM(BlueprintType)
enum class EModuleType : uint8
{
    Core,
    Physics,
    AI,
    Animation,
    Audio,
    Environment,
    Lighting,
    Characters,
    Combat,
    Crowd,
    Narrative,
    Quest,
    VFX,
    Performance,
    Integration
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EModuleType ModuleType;

    UPROPERTY(BlueprintReadOnly)
    EBuildStatus BuildStatus;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastCompileTime;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> CompileErrors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> CompileWarnings;

    FModuleInfo()
    {
        ModuleName = TEXT("");
        ModuleType = EModuleType::Core;
        BuildStatus = EBuildStatus::Unknown;
        LastCompileTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FDateTime BuildTime;

    UPROPERTY(BlueprintReadOnly)
    EBuildStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly)
    TArray<FModuleInfo> ModuleReports;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalErrors;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalWarnings;

    UPROPERTY(BlueprintReadOnly)
    float CompileTimeSeconds;

    FBuildReport()
    {
        BuildTime = FDateTime::Now();
        OverallStatus = EBuildStatus::Unknown;
        TotalErrors = 0;
        TotalWarnings = 0;
        CompileTimeSeconds = 0.0f;
    }
};

/**
 * BuildManager - Handles compilation, dependency tracking, and build verification
 * for the Transpersonal Game project. This is the core integration system that
 * ensures all modules compile correctly and dependencies are resolved.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildManager : public UObject
{
    GENERATED_BODY()

public:
    UBuildManager();

    // Core build functions
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool StartFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool CompileModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool VerifyDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    FBuildReport GetLastBuildReport() const { return LastBuildReport; }

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    void RegisterModule(const FString& ModuleName, EModuleType ModuleType, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool IsModuleLoaded(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    TArray<FModuleInfo> GetAllModules() const { return RegisteredModules; }

    // Error handling
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    TArray<FString> GetCompileErrors() const;

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    TArray<FString> GetCompileWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    void ClearBuildCache();

    // Build verification
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool VerifyBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool CheckForDuplicateFiles();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    bool ValidateIncludePaths();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    void LogBuildStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    void ExportBuildReport(const FString& FilePath);

protected:
    // Internal build functions
    bool CompileModuleInternal(const FString& ModuleName);
    bool CheckModuleDependencies(const FString& ModuleName);
    void UpdateModuleStatus(const FString& ModuleName, EBuildStatus Status);
    void ScanForSourceFiles();
    void ValidateModuleStructure();

    // Error collection
    void CollectCompileErrors();
    void CollectCompileWarnings();
    void ParseBuildOutput(const FString& BuildOutput);

private:
    UPROPERTY()
    TArray<FModuleInfo> RegisteredModules;

    UPROPERTY()
    FBuildReport LastBuildReport;

    UPROPERTY()
    TArray<FString> SourceFilePaths;

    UPROPERTY()
    TArray<FString> HeaderFilePaths;

    UPROPERTY()
    bool bBuildInProgress;

    UPROPERTY()
    FDateTime BuildStartTime;

    // Module dependency graph
    TMap<FString, TArray<FString>> ModuleDependencies;
    TMap<FString, EBuildStatus> ModuleStatusCache;

    // Build configuration
    FString ProjectPath;
    FString SourcePath;
    FString BuildPath;

    // Error tracking
    TArray<FString> CurrentErrors;
    TArray<FString> CurrentWarnings;
};

/**
 * Build Manager Subsystem - Singleton access to build management
 */
UCLASS()
class TRANSPERSONALGAME_API UBuildManagerSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    static UBuildManagerSubsystem* Get();

    UFUNCTION(BlueprintCallable, Category = "Build Manager")
    UBuildManager* GetBuildManager() const { return BuildManager; }

protected:
    UPROPERTY()
    UBuildManager* BuildManager;
};