#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "BuildValidationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FBuild_ValidationResult()
    {
        bIsValid = false;
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;

    FBuild_ModuleStatus()
    {
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, const FBuild_ValidationResult&, Result);

/**
 * Build Validation Subsystem
 * Validates the integrity of the build, modules, and map state
 * Provides comprehensive validation for the Integration Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateModules();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateMapIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> GetModuleStatuses();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsClassLoaded(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    int32 GetActorCountByType(const FString& ActorType);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void CleanupDuplicates();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SetValidationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsValidationEnabled() const { return bValidationEnabled; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Validation")
    FOnValidationComplete OnValidationComplete;

    // Auto-validation
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StartAutoValidation(float IntervalSeconds = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StopAutoValidation();

protected:
    // Internal validation functions
    void ValidateTranspersonalGameModule(FBuild_ValidationResult& Result);
    void ValidateCriticalClasses(FBuild_ValidationResult& Result);
    void ValidateMapActors(FBuild_ValidationResult& Result);
    void ValidateAssetReferences(FBuild_ValidationResult& Result);

    // Timer callback
    void OnAutoValidationTimer();

private:
    UPROPERTY()
    bool bValidationEnabled;

    UPROPERTY()
    bool bAutoValidationActive;

    UPROPERTY()
    float AutoValidationInterval;

    UPROPERTY()
    FTimerHandle AutoValidationTimer;

    UPROPERTY()
    FBuild_ValidationResult LastValidationResult;

    // Critical classes that must be loaded
    UPROPERTY()
    TArray<FString> CriticalClasses;

    // Actor types that should not be duplicated
    UPROPERTY()
    TArray<FString> SingletonActorTypes;
};