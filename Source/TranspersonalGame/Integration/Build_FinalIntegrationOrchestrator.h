#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Initializing,
    Validating,
    Stable,
    Degraded,
    Critical
};

USTRUCT(BlueprintType)
struct FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly)
    float ValidationTime = 0.0f;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("Unknown");
        ValidationMessage = TEXT("Not validated");
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnvironmentActors = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Unknown;
};

/**
 * Final Integration Orchestrator - Agent #19
 * Manages the integration and validation of all game systems
 * Ensures stable build state and coordinates between all subsystems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationMetrics GetIntegrationMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetSystemValidationResults() const;

    // System-specific validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAudioSystem();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SaveBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus CurrentStatus;

    // Internal validation functions
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName, TFunction<bool()> ValidationFunction);
    void UpdateIntegrationStatus();
    void LogValidationResult(const FBuild_SystemValidationResult& Result);

private:
    FDateTime LastValidationTime;
    bool bIsValidating = false;
};