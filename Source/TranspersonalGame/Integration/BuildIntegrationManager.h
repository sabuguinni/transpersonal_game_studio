#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_ModuleStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Empty          UMETA(DisplayName = "Empty"),
    HeadersOnly    UMETA(DisplayName = "Headers Only"),
    Implemented    UMETA(DisplayName = "Implemented"),
    Compiled       UMETA(DisplayName = "Compiled"),
    Failed         UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EBuild_ErrorType : uint8
{
    None           UMETA(DisplayName = "None"),
    Compilation    UMETA(DisplayName = "Compilation"),
    Linking        UMETA(DisplayName = "Linking"),
    Integration    UMETA(DisplayName = "Integration"),
    Dependency     UMETA(DisplayName = "Dependency"),
    Duplicate      UMETA(DisplayName = "Duplicate"),
    MissingImpl    UMETA(DisplayName = "Missing Implementation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EBuild_ModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 CppFileCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 HeaderFileCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasImplementation;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FDateTime LastModified;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    FBuild_ModuleInfo()
    {
        ModuleName = TEXT("");
        Status = EBuild_ModuleStatus::Unknown;
        CppFileCount = 0;
        HeaderFileCount = 0;
        bHasImplementation = false;
        LastModified = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ErrorInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    EBuild_ErrorType ErrorType;

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    FString SourceFile;

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Error")
    FDateTime Timestamp;

    FBuild_ErrorInfo()
    {
        ErrorType = EBuild_ErrorType::None;
        ErrorMessage = TEXT("");
        SourceFile = TEXT("");
        LineNumber = 0;
        ModuleName = TEXT("");
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_Status
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDateTime BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildAgent;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString CycleId;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float ImplementationCoverage;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ImplementedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleInfo> ModuleInfos;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ErrorInfo> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> IntegrationWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> NextActions;

    FBuild_Status()
    {
        bBuildSuccess = false;
        BuildTimestamp = FDateTime::Now();
        BuildAgent = TEXT("Integration & Build Agent #19");
        CycleId = TEXT("");
        ImplementationCoverage = 0.0f;
        TotalModules = 0;
        ImplementedModules = 0;
    }
};

/**
 * Build Integration Manager - Orchestrates compilation and integration of all game modules
 * Tracks module status, compilation errors, and provides build validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Analysis
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void AnalyzeModuleStructure();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_Status GetCurrentBuildStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CheckForDuplicateTypes();

    // Compilation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool AttemptCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ParseCompilationErrors(const FString& CompilerOutput);

    // Integration Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSharedTypes();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateBuildConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateModuleRegistration();

    // Error Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void AddCompilationError(EBuild_ErrorType ErrorType, const FString& Message, const FString& SourceFile = TEXT(""), int32 LineNumber = 0);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void AddIntegrationWarning(const FString& Warning);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ClearErrors();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GenerateBuildReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveBuildStatus(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool LoadBuildStatus(const FString& FilePath);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModule(const FString& ModuleName, EBuild_ModuleStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetModulesWithStatus(EBuild_ModuleStatus Status) const;

    // Properties
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    FBuild_Status CurrentBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    bool bIntegrationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    FString ProjectSourcePath;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> KnownModules;

private:
    // Internal helpers
    void ScanModuleDirectory(const FString& ModulePath, FBuild_ModuleInfo& ModuleInfo);
    bool CheckFileExists(const FString& FilePath) const;
    FString GetProjectSourceDirectory() const;
    void UpdateImplementationCoverage();
    bool IsModuleEmpty(const FString& ModulePath) const;
    void LogBuildStatus() const;
};