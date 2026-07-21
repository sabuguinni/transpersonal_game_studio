#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Initializing UMETA(DisplayName = "Initializing"),
    SystemsLoading UMETA(DisplayName = "Systems Loading"),
    ModulesValidating UMETA(DisplayName = "Modules Validating"),
    CrossSystemTesting UMETA(DisplayName = "Cross System Testing"),
    BuildComplete UMETA(DisplayName = "Build Complete"),
    BuildFailed UMETA(DisplayName = "Build Failed"),
    BridgeFailure UMETA(DisplayName = "Bridge Failure")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInWorld;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime BuildTimestamp;

    FBuild_IntegrationReport()
    {
        Status = EBuild_IntegrationStatus::Unknown;
        TotalActorsInWorld = 0;
        ActiveSystemCount = 0;
        TotalIntegrationTime = 0.0f;
        BuildVersion = TEXT("PROD_CYCLE_AUTO_20260515_002");
        BuildTimestamp = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Coordinates all systems and validates complete build integrity
 * Handles bridge failures and emergency recovery protocols
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void HandleBridgeFailure();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EmergencyRecoveryProtocol();

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateEnvironmentSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateQASystems();

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ArchiveBuildResults();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ReportToStudioDirector();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBridgeFailureDetected;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalSystemNames;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationStartTime;

private:
    void InitializeCriticalSystems();
    void LogIntegrationStep(const FString& StepName, bool bSuccess);
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName, TFunction<bool()> ValidationFunction);
};