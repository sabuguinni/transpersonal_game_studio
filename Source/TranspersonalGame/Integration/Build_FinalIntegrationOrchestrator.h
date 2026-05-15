#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Validated       UMETA(DisplayName = "Validated")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime LastValidation;

    FBuild_SystemIntegrationData()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ActorCount = 0;
        ValidationScore = 0.0f;
        LastValidation = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CrossSystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsCompatible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float CompatibilityScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> ValidationErrors;

    FBuild_CrossSystemValidation()
    {
        SystemA = TEXT("");
        SystemB = TEXT("");
        bIsCompatible = false;
        CompatibilityScore = 0.0f;
    }
};

/**
 * Final Integration Orchestrator - Manages the complete integration of all game systems
 * Ensures all agent outputs work together cohesively in the final build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeIntegrationOrchestrator();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformCrossSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateFinalBuildReport();

    // System registration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FString& SystemName, int32 ActorCount, float ValidationScore);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystem(const FString& SystemName);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateOverallIntegrationScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetSystemValidationErrors();

    // Cross-system compatibility
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemCompatibility(const FString& SystemA, const FString& SystemB);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool AreSystemsCompatible(const FString& SystemA, const FString& SystemB);

    // Build orchestration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OrchestrateFinalBuild();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationMetrics();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_SystemIntegrationData> GetRegisteredSystems() const { return RegisteredSystems; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationStatus GetOverallIntegrationStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    float GetIntegrationProgress() const { return IntegrationProgress; }

protected:
    // System tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_SystemIntegrationData> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_CrossSystemValidation> CrossSystemValidations;

    // Integration status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float IntegrationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float OverallValidationScore;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float MinimumValidationScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxValidationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnableAutomaticValidation;

    // Integration metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ValidatedSystemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime LastFullValidation;

private:
    // Internal validation helpers
    void ValidateSystemDependencies(const FString& SystemName);
    void UpdateIntegrationProgress();
    void LogIntegrationStatus();
    FBuild_SystemIntegrationData* FindSystemData(const FString& SystemName);
};