#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime = 0.0f;

    FBuild_FinalValidationResult()
    {
        bIsValid = false;
        ValidationMessage = TEXT("Not validated");
        ErrorCount = 0;
        WarningCount = 0;
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsIntegrated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasDependencies = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> IntegrationIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore = 0.0f;

    FBuild_SystemIntegrationStatus()
    {
        SystemName = TEXT("Unknown");
        bIsIntegrated = false;
        bHasDependencies = false;
        IntegrationScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main validation functions
    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    FBuild_FinalValidationResult ValidateFullSystem();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    FBuild_FinalValidationResult ValidateGameplayIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    FBuild_FinalValidationResult ValidatePerformanceIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    TArray<FBuild_SystemIntegrationStatus> GetSystemIntegrationStatus();

    // Critical system validation
    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateQuestSystem();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool ValidateVFXSystem();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool TestMinPlayableMapIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    bool TestCompilationIntegrity();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    FString GenerateFinalIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Final Integration")
    void SaveIntegrationReport(const FString& ReportPath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Final Integration")
    TArray<FBuild_SystemIntegrationStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Final Integration")
    FBuild_FinalValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Final Integration")
    bool bIsValidationInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "Final Integration")
    float LastValidationTime = 0.0f;

private:
    // Internal validation helpers
    bool ValidateSystemDependencies();
    bool ValidateModuleIntegration();
    bool ValidateAssetIntegration();
    bool ValidateGameplayFlow();
    
    // Performance validation
    bool ValidateMemoryUsage();
    bool ValidateFrameRate();
    bool ValidateLODSystems();
    
    // Critical path validation
    bool ValidatePlayerSpawn();
    bool ValidateBasicMovement();
    bool ValidateWorldLoading();
    bool ValidateUIIntegration();
    
    // Helper functions
    void LogValidationError(const FString& Error);
    void LogValidationWarning(const FString& Warning);
    void ResetValidationState();
    
    // Validation state
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
    double ValidationStartTime = 0.0;
};