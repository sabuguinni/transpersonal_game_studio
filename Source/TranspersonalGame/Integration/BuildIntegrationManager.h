#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Initializing,
    SystemsValidated,
    CrossSystemTested,
    PerformanceOptimized,
    Ready,
    Failed
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 StaticMeshCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 SkeletalMeshCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LightCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DegenerateLabelCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bEssentialSystemsPresent = false;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float ValidationTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FBuild_SystemMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> WarningMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bPerformanceWithinLimits = true;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCrossSystemCompatible = true;
};

/**
 * Build Integration Manager - Orchestrates validation and integration of all game systems
 * Ensures compatibility between different agent outputs and maintains build stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ValidationResult ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCoreGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCharacterWorldIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAIEnvironmentIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAudioVisualSynchronization();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemMetrics GatherSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool EnforcePerformanceLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CleanupDegenerateLabels();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SaveValidatedMap();

    // Status and monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_IntegrationStatus GetCurrentIntegrationStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ValidationResult GetLastValidationResult() const { return LastValidationResult; }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxDinosaurCount = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxLabelLength = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxLabelUnderscores = 5;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    EBuild_IntegrationStatus CurrentStatus = EBuild_IntegrationStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float LastValidationTime = 0.0f;

private:
    // Internal validation helpers
    bool ValidateEssentialSystems();
    bool ValidateSpawnLocations();
    bool ValidateNavigationMesh();
    bool CheckPerformanceMetrics(const FBuild_SystemMetrics& Metrics);
    void LogValidationResults(const FBuild_ValidationResult& Result);
    TArray<AActor*> GetActorsByLabelPattern(const FString& Pattern);
    void CleanupActorsByType(const TArray<AActor*>& Actors, int32 MaxCount);
};