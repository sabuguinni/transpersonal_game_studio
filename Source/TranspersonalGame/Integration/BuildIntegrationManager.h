#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

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
    FString BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float BuildDurationSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 HeaderCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ImplementationCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> OrphanHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString LastCompileTime;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_CompilationResult CompilationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInMap = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DuplicateLightingActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMapPlayable = false;
};

/**
 * Build Integration Manager - Coordena a integração e validação de todos os sistemas
 * Responsável por verificar compilação, detectar conflitos, e manter a estabilidade do build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool VerifyMapPlayability();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetOrphanHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TriggerCompilationTest();

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RemoveDuplicateLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 CountActorsByClass(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateActorDistribution();

    // Module Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ModuleStatus AnalyzeModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckHeaderImplementationPairs();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogCriticalIssues(const TArray<FString>& Issues);

protected:
    // Internal state
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIntegrationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime = 0.0f;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    float ValidationIntervalSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bAutoCleanupDuplicates = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Integration")
    bool bLogVerboseOutput = false;

private:
    // Internal validation functions
    void PerformPeriodicValidation();
    bool ValidateActorIntegrity();
    bool ValidateSystemDependencies();
    void GenerateCompilationReport(FBuild_CompilationResult& OutResult);
    void ScanForOrphanHeaders(TArray<FString>& OutOrphanHeaders);
    void ValidateBiomeActorDistribution();
};