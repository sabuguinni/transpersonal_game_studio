#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngArch_CompilationManager.generated.h"

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown = 0,
    Compiling = 1,
    Success = 2,
    Failed = 3,
    Warning = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    FString ModuleName;

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    EEng_CompilationStatus Status;

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    float CompileTime;

    UPROPERTY(BlueprintReadWrite, Category = "Compilation")
    TArray<FString> ErrorMessages;

    FEng_CompilationResult()
    {
        ModuleName = TEXT("");
        Status = EEng_CompilationStatus::Unknown;
        ErrorCount = 0;
        WarningCount = 0;
        CompileTime = 0.0f;
    }
};

/**
 * Engine Architect Compilation Manager
 * Monitors compilation status, tracks errors, and ensures code quality standards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_CompilationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_CompilationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_CompilationResult GetModuleCompilationResult(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_CompilationResult> GetAllCompilationResults();

    // Error Tracking
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetTotalErrorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetTotalWarningCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetCriticalErrors() const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCodeQuality();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateHeaderIncludes();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateUPropertyMacros();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsModuleLoaded(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetLoadedModules() const;

    // Debug and Reporting
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void ValidateProjectIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor)
    void CleanupDisabledFiles();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_CompilationResult> CompilationResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    TArray<FString> LoadedModules;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    TArray<FString> CriticalErrors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bLastCompilationSuccessful;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    float LastCompilationTime;

private:
    void ScanForCompilationErrors();
    void ValidateFileStructure();
    void CheckForDisabledFiles();
    bool ValidateClassDefinition(const FString& ClassName);
    void LogCompilationIssue(const FString& Issue, bool bIsCritical = false);
};