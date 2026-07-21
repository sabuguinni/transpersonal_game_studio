#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalIntegrationOrchestrator_Cycle008.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus_Cycle008 : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Validating      UMETA(DisplayName = "Validating"),
    Integrating     UMETA(DisplayName = "Integrating"),
    Testing         UMETA(DisplayName = "Testing"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport_Cycle008
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus_Cycle008 Status = EBuild_IntegrationStatus_Cycle008::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PassedChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> VFXValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CoreSystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> PerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bQAValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bVFXSystemsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCoreSystemsStable = false;

    FBuild_IntegrationReport_Cycle008()
    {
        Status = EBuild_IntegrationStatus_Cycle008::Unknown;
        IntegrationScore = 0.0f;
        TotalChecks = 0;
        PassedChecks = 0;
        FailedChecks = 0;
        BuildTimestamp = FDateTime::Now().ToString();
        bQAValidationPassed = false;
        bVFXSystemsOperational = false;
        bCoreSystemsStable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult_Cycle008
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime = 0.0f;

    FBuild_SystemValidationResult_Cycle008()
    {
        SystemName = TEXT("Unknown");
        bIsLoaded = false;
        bIsOperational = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

/**
 * Final Integration Orchestrator for Cycle 008
 * Processes QA validation results and performs final build integration
 * Coordinates between VFX systems, core systems, and performance validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator_Cycle008 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator_Cycle008();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration orchestration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ProcessQAValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCoreSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalIntegrationReport();

    // Status queries
    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationStatus_Cycle008 GetIntegrationStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    FBuild_IntegrationReport_Cycle008 GetIntegrationReport() const { return IntegrationReport; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    float GetIntegrationScore() const { return IntegrationReport.IntegrationScore; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsIntegrationComplete() const { return CurrentStatus == EBuild_IntegrationStatus_Cycle008::Complete; }

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult_Cycle008 ValidateSystem(const FString& SystemName, const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult_Cycle008> ValidateAllSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus_Cycle008 CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport_Cycle008 IntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult_Cycle008> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> VFXClassesToValidate;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CoreClassesToValidate;

    // Internal methods
    void InitializeValidationLists();
    void UpdateIntegrationStatus(EBuild_IntegrationStatus_Cycle008 NewStatus);
    void LogIntegrationProgress(const FString& Message);
    float CalculateIntegrationScore() const;
    bool ValidateModuleCompilation() const;
    bool ValidateActorCounts() const;
    void SaveIntegrationState();
};