#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

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
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ErrorMessage = TEXT("");
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float ValidationTime;

    FBuild_ValidationResult()
    {
        bSuccess = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ValidationResult ValidateModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatusList();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateClassRegistration(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateActorSpawning(const FString& ActorClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ValidationResult RunIntegrationTests();

    // Cross-system validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAISystems();

    // Build reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetBuildVersion();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetLastBuildTime();

protected:
    // Internal validation helpers
    bool ValidateModuleClasses(const FString& ModuleName);
    bool ValidateComponentRegistration();
    bool ValidateSubsystemInitialization();
    bool ValidateCrossModuleDependencies();

    // Error tracking
    void LogValidationError(const FString& ErrorMessage);
    void LogValidationWarning(const FString& WarningMessage);

private:
    UPROPERTY()
    TArray<FBuild_ModuleStatus> CachedModuleStatus;

    UPROPERTY()
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY()
    FString BuildVersion;

    UPROPERTY()
    FDateTime LastBuildTime;

    // Validation state
    bool bValidationInProgress;
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
};