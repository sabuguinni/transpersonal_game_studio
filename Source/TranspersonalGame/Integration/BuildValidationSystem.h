#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildValidationSystem.generated.h"

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
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float ValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bOverallSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> CriticalErrors;

    FBuild_ValidationReport()
    {
        ValidationTimestamp = 0.0f;
        bOverallSuccess = false;
        TotalModules = 0;
        LoadedModules = 0;
        TotalActors = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateMapState(const FString& MapPath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetLoadedModules();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    int32 GetActorCountInLevel();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestClassLoading(const FString& ClassName);

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestCrossModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateSharedTypes();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool CheckCompilationStatus();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateValidationReport(const FBuild_ValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GetLastValidationSummary();

protected:
    // Internal validation helpers
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName);
    bool ValidateActorSpawning();
    bool ValidateComponentSystems();
    void LogValidationResults(const FBuild_ValidationReport& Report);

private:
    UPROPERTY()
    FBuild_ValidationReport LastValidationReport;

    UPROPERTY()
    TArray<FString> KnownModules;

    UPROPERTY()
    TArray<FString> CriticalClasses;

    // Validation state
    bool bValidationInProgress;
    float LastValidationTime;
};