#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsInitialized = false;
        ComponentCount = 0;
    }
};

/**
 * Integration Validator - Validates system integration and build status
 * Ensures all game systems work together correctly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === VALIDATION METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationResult> RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateLevelIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult ValidateAssetReferences();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemStatus> GetSystemStatusReport();

    // === INTEGRATION TESTING ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestCrossSystemCommunication();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestPerformanceBaseline();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestMemoryUsage();

    // === BUILD VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool ValidateBuildConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetMissingDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build")
    TArray<FString> GetCompilationErrors();

protected:
    // === VALIDATION STATE ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationResult> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bLogValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TArray<FString> CriticalSystems;

private:
    // === INTERNAL VALIDATION ===
    
    FBuild_ValidationResult ValidateSpecificSystem(const FString& SystemName);
    bool CheckSystemDependency(const FString& SystemName, const FString& DependencyName);
    void LogValidationResult(const FBuild_ValidationResult& Result);
    void UpdateSystemStatus(const FString& SystemName);

    // === TIMING ===
    float ValidationTimer;
    double LastValidationStartTime;
};