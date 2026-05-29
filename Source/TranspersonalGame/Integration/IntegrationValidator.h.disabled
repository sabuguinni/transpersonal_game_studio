#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "IntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Details;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasErrors = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_LevelStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    FString LevelName;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    int32 DuplicateActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bHasPlayerStart = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bHasCharacter = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    bool bHasLighting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Level")
    TMap<FString, int32> ActorCounts;
};

/**
 * Integration Validator Subsystem
 * Validates system integration, module loading, and level consistency
 */
UCLASS()
class TRANSPERSONALGAME_API UIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_ValidationResult ValidateFullSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_ModuleStatus ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FInteg_LevelStatus ValidateLevel(const FString& LevelPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CleanupDuplicateActors(const FString& LevelPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetLoadedModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFramerate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable();

protected:
    // Internal validation helpers
    bool ValidateModuleClasses(const FString& ModuleName, FInteg_ModuleStatus& OutStatus);
    bool ValidateLevelActors(UWorld* World, FInteg_LevelStatus& OutStatus);
    void CleanupLightingDuplicates(UWorld* World);
    void LogValidationResults(const FInteg_ValidationResult& Results);

private:
    UPROPERTY()
    TArray<FString> KnownModules;

    UPROPERTY()
    TArray<FString> CriticalClasses;

    UPROPERTY()
    float LastValidationTime = 0.0f;

    UPROPERTY()
    bool bValidationInProgress = false;
};