#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Operational UMETA(DisplayName = "Operational"),
    Degraded    UMETA(DisplayName = "Degraded"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime LastValidated;

    FBuild_SystemMetrics()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_SystemStatus::Unknown;
        ActorCount = 0;
        PerformanceScore = 0.0f;
        LastValidated = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemMetrics> SystemMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime GeneratedAt;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        bAllSystemsOperational = false;
        BuildVersion = TEXT("PROD_CYCLE_AUTO_20260516_006");
        GeneratedAt = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Manages complete system integration and build validation
 * Responsible for coordinating all game systems and ensuring stable builds
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

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RefreshSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemMetrics> GetAllSystemMetrics() const { return SystemMetrics; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemMetrics> SystemMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsInitialized;

private:
    void ValidateCharacterSystem();
    void ValidateDinosaurSystem();
    void ValidateEnvironmentSystem();
    void ValidateLightingSystem();
    void ValidateVFXSystem();
    void ValidateAudioSystem();
    void ValidatePhysicsSystem();

    FBuild_SystemMetrics CreateSystemMetric(const FString& SystemName, int32 ActorCount, bool bIsOperational);
    void LogSystemStatus(const FString& SystemName, EBuild_SystemStatus Status);
};