#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuildValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime ValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = true;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LightingActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 EnvironmentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 CharacterActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 AIActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 AudioVFXActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 OtherActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationMessages;

    FBuildValidationReport()
    {
        ValidationTime = FDateTime::Now();
        bIsValid = true;
        TotalActorCount = 0;
        LightingActorCount = 0;
        EnvironmentActorCount = 0;
        CharacterActorCount = 0;
        AIActorCount = 0;
        AudioVFXActorCount = 0;
        OtherActorCount = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDuplicatesCleaned, int32, DestroyedCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidationComplete, const FBuildValidationReport&, Report);

/**
 * BuildIntegrationManager - Manages build integration, duplicate cleanup, and system validation
 * 
 * This component automatically:
 * - Cleans duplicate lighting actors (DirectionalLight, SkyLight, ExponentialHeightFog)
 * - Validates system integration across all agent outputs
 * - Provides detailed validation reports
 * - Monitors actor counts and categories
 * 
 * Critical for preventing the duplicate actor issues that cause memory problems.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bAutoCleanDuplicates = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bValidateSystemIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bLogDetailedReports = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    int32 MaxAllowedDuplicates = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    float ValidationCheckInterval = 10.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Integration Events")
    FOnDuplicatesCleaned OnDuplicatesCleanedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Integration Events")
    FOnValidationComplete OnValidationCompleteDelegate;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuildValidationReport GetLatestValidationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuildValidationReport> GetValidationHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ForceValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ForceDuplicateCleanup();

private:
    // Internal validation functions
    void ValidateLightingSystem(FBuildValidationReport& Report);
    void ValidateCharacterSystem(FBuildValidationReport& Report);
    void ValidateWorldGeneration(FBuildValidationReport& Report);
    void ValidateAISystems(FBuildValidationReport& Report);
    
    void LogValidationReport(const FBuildValidationReport& Report);

    // State tracking
    float LastValidationTime = 0.0f;
    
    UPROPERTY()
    TArray<FBuildValidationReport> ValidationReports;
};