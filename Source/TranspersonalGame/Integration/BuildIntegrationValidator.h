#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "BuildIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Severity;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString FilePath;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LineNumber;

    FBuild_ValidationIssue()
    {
        Severity = EBuild_ValidationResult::Success;
        Category = TEXT("");
        Description = TEXT("");
        FilePath = TEXT("");
        LineNumber = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationIssue> Issues;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalHeaderFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalCppFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 OrphanHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 DuplicateActors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime Timestamp;

    FBuild_ValidationReport()
    {
        TotalHeaderFiles = 0;
        TotalCppFiles = 0;
        OrphanHeaders = 0;
        DuplicateActors = 0;
        ValidationTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

/**
 * Build Integration Validator - Validates project structure and compilation readiness
 * Detects orphan headers, duplicate actors, missing implementations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    FBuild_ValidationReport LastValidationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    bool bCleanupDuplicatesAutomatically;

private:
    FTimerHandle ValidationTimerHandle;

public:
    // Validation Methods
    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    FBuild_ValidationReport ValidateProjectStructure();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    int32 DetectOrphanHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    int32 DetectDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    bool CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    TArray<FString> GetMisplacedFiles();

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    bool ValidateModuleDependencies();

    // Report Methods
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationReport GetLastValidationReport() const { return LastValidationReport; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    void ExportValidationReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation", CallInEditor)
    void LogValidationSummary();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetAutoValidation(bool bEnabled, float IntervalSeconds = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetCleanupMode(bool bAutoCleanup) { bCleanupDuplicatesAutomatically = bAutoCleanup; }

private:
    void PerformPeriodicValidation();
    void AddValidationIssue(EBuild_ValidationResult Severity, const FString& Category, 
                           const FString& Description, const FString& FilePath = TEXT(""), 
                           int32 LineNumber = 0);
    void ClearValidationIssues();
    FString GetProjectSourcePath() const;
    TArray<FString> FindFilesRecursive(const FString& Directory, const FString& Extension) const;
};

#include "BuildIntegrationValidator.generated.h"