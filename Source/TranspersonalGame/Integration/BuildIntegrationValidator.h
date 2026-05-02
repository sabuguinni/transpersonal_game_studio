#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationValidator.generated.h"

/**
 * Build Integration Validator - Validates cross-module integration and build consistency
 * Used by Integration Agent #19 to ensure all agent outputs work together
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationValidator : public UObject
{
    GENERATED_BODY()

public:
    UBuildIntegrationValidator();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateMapIntegrity(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateLightingSetup(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateGameplayActors(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CleanDuplicateActors(UWorld* World);

    // Validation results
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    TArray<FString> ValidationWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    int32 DuplicatesRemoved;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    bool bMapIsValid;

    // Actor counting and analysis
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TMap<FString, int32> GetActorCountsByClass(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<AActor*> FindDuplicateLightingActors(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidatePlayerStart(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateTranspersonalGameClasses();

    // Build system validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCrossModuleDependencies();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogValidationResults();

private:
    // Internal validation helpers
    bool ValidateActorClass(const FString& ClassName);
    bool CheckForDuplicateTypes(UWorld* World, const FString& ActorType);
    void AddValidationError(const FString& Error);
    void AddValidationWarning(const FString& Warning);
    void ClearValidationResults();
};