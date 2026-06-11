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
    Pending     UMETA(DisplayName = "Pending"),
    Running     UMETA(DisplayName = "Running"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        ClassCount = 0;
    }
};

/**
 * Integration Validator Component - Validates build integrity and module loading
 * Used by Integration Agent #19 to ensure all systems integrate correctly
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    // === VALIDATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateClassRegistration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLevelIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // === GETTERS ===
    
    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_ValidationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    float GetValidationProgress() const { return ValidationProgress; }

protected:
    // === VALIDATION DATA ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_ValidationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedTests;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> RequiredClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bVerboseLogging;

private:
    // === INTERNAL VALIDATION METHODS ===
    
    FBuild_ValidationResult RunSingleTest(const FString& TestName, TFunction<bool()> TestFunction);
    void LogValidationResult(const FBuild_ValidationResult& Result);
    void UpdateValidationProgress();
    void ResetValidationState();
};

/**
 * Integration Test Actor - Spawnable actor for integration testing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationTestActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationTestActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    class UBuild_IntegrationValidator* IntegrationValidator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    class UStaticMeshComponent* TestMesh;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintImplementableEvent, Category = "Integration")
    void OnValidationComplete(EBuild_ValidationStatus Status);
};