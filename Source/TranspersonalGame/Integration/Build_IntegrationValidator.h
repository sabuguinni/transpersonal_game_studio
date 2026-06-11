#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Validating      UMETA(DisplayName = "Validating"),
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassesFailed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        ClassesLoaded = 0;
        ClassesFailed = 0;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CustomActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 BinaryFilesFound;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalValidationTime;

    FBuild_IntegrationReport()
    {
        TotalActorsInLevel = 0;
        CustomActorsInLevel = 0;
        BinaryFilesFound = 0;
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        ValidationTimestamp = FDateTime::Now();
        TotalValidationTime = 0.0f;
    }
};

/**
 * Integration & Build Validation System
 * Validates module compilation, class loading, and system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UObject
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    // === VALIDATION METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport ValidateFullIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName, const TArray<FString>& ClassNames);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateClassLoading(const FString& ClassName, FString& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    int32 CountLevelActors(bool bCustomActorsOnly = false);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    int32 CountCompiledBinaries();

    // === DIAGNOSTIC METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    TArray<FString> GetFailedClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    TArray<FString> GetLoadedClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FString GenerateIntegrationReport(const FBuild_IntegrationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool FixCommonIntegrationIssues();

    // === TESTING METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool TestActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool TestSystemInteractions();

protected:
    // === INTERNAL VALIDATION ===
    
    bool ValidateBinaryFiles();
    bool ValidateProjectStructure();
    bool ValidateDependencies();
    
    EBuild_IntegrationStatus DetermineOverallStatus(const TArray<FBuild_ModuleStatus>& ModuleStatuses);
    
    // === CACHED DATA ===
    
    UPROPERTY()
    FBuild_IntegrationReport LastValidationReport;

    UPROPERTY()
    TArray<FString> KnownModules;

    UPROPERTY()
    TArray<FString> CriticalClasses;

private:
    void InitializeKnownModules();
    void InitializeCriticalClasses();
    
    float ValidationStartTime;
};