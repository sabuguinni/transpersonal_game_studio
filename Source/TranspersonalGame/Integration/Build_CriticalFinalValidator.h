#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CriticalFinalValidator.generated.h"

/**
 * Critical Final Validator - Integration Agent #19
 * Final validation system for complete project integration
 * Validates all systems are functional and ready for deployment
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalValidationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalClassesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 FailedClassesCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LevelActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 CharacterActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 DinosaurActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LightActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bAllCriticalSystemsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bMinPlayableMapValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> LoadedSystems;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsFunctional = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LoadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DependencyCount = 0;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_CriticalFinalValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_CriticalFinalValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Critical validation functions
    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    FBuild_FinalValidationMetrics RunCompleteValidation();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    bool ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    TArray<FBuild_SystemIntegrationStatus> GetSystemIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    float CalculateOverallValidationScore();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    void GenerateFinalIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Final Validation")
    bool IsProjectReadyForDeployment();

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckCoreSystemsHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckWorldGenerationHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckCharacterSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckAISystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckVFXSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    bool CheckAudioSystemHealth();

    // Integration verification
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool VerifySystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool VerifyAssetIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool VerifyLevelIntegration();

    // Emergency recovery
    UFUNCTION(BlueprintCallable, Category = "Recovery")
    void TriggerEmergencyRecovery();

    UFUNCTION(BlueprintCallable, Category = "Recovery")
    bool AttemptSystemRecovery(const FString& SystemName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FBuild_FinalValidationMetrics LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_SystemIntegrationStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime = 0.0f;

    // Critical system class paths
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TArray<FString> CriticalSystemPaths;

    // Validation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MinValidationScore = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MinRequiredActors = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MinRequiredSystems = 6;

private:
    void InitializeCriticalSystemPaths();
    bool ValidateSystemClass(const FString& ClassPath);
    void UpdateSystemStatus(const FString& SystemName, bool bLoaded, const FString& ErrorMsg = "");
    float CalculateSystemScore(const FBuild_SystemIntegrationStatus& Status);
};