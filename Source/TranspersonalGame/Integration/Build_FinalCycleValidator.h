#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalCycleValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCycleSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalSystemsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 SystemsPassingValidation = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 CriticalErrorsFound = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float CycleExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime CycleTimestamp;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bWorldGenerationHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bCharacterSystemHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bDinosaurAIHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bVFXSystemHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bAudioSystemHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bQASystemHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 FunctionalActorsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float OverallSystemHealth = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_FinalCycleValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    FBuild_CycleValidationResult ValidateCompleteCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    FBuild_SystemHealthMetrics GetSystemHealthMetrics();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    bool ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    TArray<FString> GetCriticalSystemErrors();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    void GenerateFinalCycleReport(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    bool IsGamePlayable();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    float CalculateOverallIntegrationHealth();

protected:
    UPROPERTY()
    FBuild_CycleValidationResult LastValidationResult;

    UPROPERTY()
    FBuild_SystemHealthMetrics CachedHealthMetrics;

    UPROPERTY()
    TArray<FString> CriticalErrors;

    UPROPERTY()
    bool bValidationInProgress = false;

private:
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateDinosaurAI();
    bool ValidateVFXSystems();
    bool ValidateAudioSystems();
    bool ValidateQASystems();
    
    void CollectSystemMetrics();
    void ValidateActorIntegrity();
    void CheckCompilationStatus();
    void AnalyzePerformanceMetrics();
    
    int32 CountFunctionalActors();
    bool CheckCriticalComponentsPresent();
    void LogValidationResults(const FBuild_CycleValidationResult& Result);
};