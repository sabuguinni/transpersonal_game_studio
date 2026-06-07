#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsCompiled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllModulesLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedModules;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationDuration = 0.0f;
};

/**
 * Build Integration Manager - Handles integration of all game systems
 * Validates compilation, module loading, and cross-system compatibility
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationStatus GetIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckModuleCompatibility(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RefreshSystemStatus();

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool TriggerBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetLoadedModuleList() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetFailedModuleList() const;

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Actors")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Actors")
    bool ValidateActorIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Actors")
    void CleanupInvalidActors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> SystemModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime = 0.0f;

private:
    void InitializeSystemModules();
    bool ValidateModule(const FString& ModuleName);
    void UpdateIntegrationStatus();
    void LogValidationResults(const FBuild_ValidationResult& Result);
};