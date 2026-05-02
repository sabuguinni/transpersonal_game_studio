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
    TArray<FString> DetailedErrors;

    FInteg_ValidationResult()
    {
        bIsValid = false;
        ErrorMessage = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
    }
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
    bool bIsCompiled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;

    FInteg_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        ClassCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FInteg_ValidationResult ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FInteg_ValidationResult ValidateMapIntegrity(const FString& MapPath = TEXT("/Game/Maps/MinPlayableMap"));

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    TArray<FInteg_ModuleStatus> ValidateModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FInteg_ValidationResult ValidateActorDuplicates();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FInteg_ValidationResult ValidatePerformance();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogValidationResults(const FInteg_ValidationResult& Results);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveValidationReport(const FString& FilePath);

protected:
    // Internal validation helpers
    bool ValidateClassLoading(const FString& ClassName, FString& OutError);
    bool ValidateActorSpawning(UClass* ActorClass, FString& OutError);
    int32 CountActorsOfType(UClass* ActorClass);
    void CleanupActorsOfType(UClass* ActorClass, int32 MaxCount = 1);

private:
    // Validation state
    UPROPERTY()
    TArray<FInteg_ValidationResult> ValidationHistory;

    UPROPERTY()
    float LastValidationTime = 0.0f;

    // Critical classes to validate
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };

    // Lighting classes that should have max 1 instance
    TArray<UClass*> SingletonLightingClasses;
};