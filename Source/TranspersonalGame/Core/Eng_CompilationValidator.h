#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Eng_CompilationValidator.generated.h"

/**
 * Engine Architect's Compilation Validation System
 * Monitors C++ compilation status, module loading, and critical system functionality
 * Provides real-time feedback on build health and identifies compilation issues
 */

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    PartialSuccess  UMETA(DisplayName = "Partial Success"),
    ModuleError     UMETA(DisplayName = "Module Error")
};

UENUM(BlueprintType)
enum class EEng_ModuleStatus : uint8
{
    NotLoaded       UMETA(DisplayName = "Not Loaded"),
    Loading         UMETA(DisplayName = "Loading"),
    Loaded          UMETA(DisplayName = "Loaded"),
    Failed          UMETA(DisplayName = "Failed"),
    Unloading       UMETA(DisplayName = "Unloading")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ClassValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ClassName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasCppFile;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasHeaderFile;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    FEng_ClassValidationResult()
    {
        ClassName = TEXT("");
        bIsLoaded = false;
        bHasCppFile = false;
        bHasHeaderFile = false;
        ErrorMessage = TEXT("");
        LastValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 LoadedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 FailedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> DependencyModules;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LoadTime;

    FEng_ModuleValidationResult()
    {
        ModuleName = TEXT("");
        Status = EEng_ModuleStatus::NotLoaded;
        LoadedClassCount = 0;
        FailedClassCount = 0;
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    EEng_CompilationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 MissingCppFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_ClassValidationResult> ClassResults;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FEng_ModuleValidationResult> ModuleResults;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString LastCompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float ValidationDuration;

    FEng_CompilationReport()
    {
        OverallStatus = EEng_CompilationStatus::Unknown;
        TotalClasses = 0;
        LoadedClasses = 0;
        FailedClasses = 0;
        MissingCppFiles = 0;
        LastCompilationTime = TEXT("");
        ValidationDuration = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_CompilationValidator : public AActor
{
    GENERATED_BODY()

public:
    AEng_CompilationValidator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ValidatorMesh;

    // Validation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    TArray<FString> CriticalClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    TArray<FString> RequiredModules;

    // Current Status
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FEng_CompilationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsValidating;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 ValidationCount;

public:
    // Main Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void ValidateClasses();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void ValidateModules();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void ValidateCriticalSystems();

    // Class Validation
    UFUNCTION(BlueprintCallable, Category = "Class Validation")
    FEng_ClassValidationResult ValidateClass(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Class Validation")
    bool IsClassLoaded(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Class Validation")
    TArray<FString> GetLoadedClasses();

    UFUNCTION(BlueprintCallable, Category = "Class Validation")
    TArray<FString> GetFailedClasses();

    // Module Validation
    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    FEng_ModuleValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    bool IsModuleLoaded(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Validation")
    void RefreshModule(const FString& ModuleName);

    // Compilation Health
    UFUNCTION(BlueprintCallable, Category = "Health Check")
    EEng_CompilationStatus GetCompilationHealth();

    UFUNCTION(BlueprintCallable, Category = "Health Check")
    float GetCompilationScore();

    UFUNCTION(BlueprintCallable, Category = "Health Check")
    bool HasCriticalErrors();

    UFUNCTION(BlueprintCallable, Category = "Health Check")
    TArray<FString> GetCriticalErrors();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void SaveValidationReport(const FString& FilePath);

    // Auto-Validation
    UFUNCTION(BlueprintCallable, Category = "Auto Validation")
    void StartAutoValidation();

    UFUNCTION(BlueprintCallable, Category = "Auto Validation")
    void StopAutoValidation();

    UFUNCTION(BlueprintCallable, Category = "Auto Validation")
    void SetValidationInterval(float NewInterval);

    // Cleanup and Repair
    UFUNCTION(BlueprintCallable, Category = "Cleanup", CallInEditor)
    void CleanupOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Cleanup", CallInEditor)
    void IdentifyMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Cleanup")
    void RepairBrokenDependencies();

private:
    // Internal validation logic
    void PerformValidationTick();
    void UpdateValidationStatus();
    void ProcessValidationResults();
    
    // Helper functions
    bool CheckFileExists(const FString& FilePath);
    FString GetClassPath(const FString& ClassName);
    void LogValidationError(const FString& Message);
    void LogValidationWarning(const FString& Message);
    
    // Timer handle for auto-validation
    FTimerHandle ValidationTimerHandle;
    
    // Internal state
    bool bValidationInProgress;
    float ValidationStartTime;
    TArray<FString> PendingValidations;
};