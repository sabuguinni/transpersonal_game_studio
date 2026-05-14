#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Operational UMETA(DisplayName = "Operational"),
    Failed      UMETA(DisplayName = "Failed"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;

    FBuild_SystemIntegrationData()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Pending;
        ActorCount = 0;
        PerformanceScore = 0.0f;
        LastValidationTime = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_SystemIntegrationData> SystemsData;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildStable;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        OverallHealthScore = 0.0f;
        bBuildStable = false;
        BuildTimestamp = TEXT("");
    }
};

/**
 * Final Integration Orchestrator - Coordinates all game systems and ensures stable build
 * Responsible for system validation, performance monitoring, and build stability
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
    void PerformFullSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemStability();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OrchestrateBuildFinalization();

    // System monitoring
    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void RegisterSystemForMonitoring(const FString& SystemName, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    float CalculateOverallHealthScore();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemIntegration();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool VerifyBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void FinalizeProductionBuild();

protected:
    // System tracking
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_SystemIntegrationData> TrackedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationStartTime;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinHealthScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime;

private:
    // Internal validation
    void ValidateVFXSystems();
    void ValidateAudioSystems();
    void ValidateCharacterSystems();
    void ValidateWorldSystems();
    void ValidateAISystems();
    void ValidateQuestSystems();
    void ValidateBuildSystems();

    // Performance monitoring
    void MonitorSystemPerformance();
    void UpdateSystemMetrics();
    void CheckPerformanceThresholds();

    // Integration coordination
    void CoordinateSystemIntegration();
    void ResolveSystemConflicts();
    void OptimizeSystemInteractions();

    // Build finalization
    void PrepareProductionBuild();
    void ValidateFinalBuild();
    void CreateBuildDocumentation();
};