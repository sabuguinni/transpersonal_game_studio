#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastCheckTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ActorCount = 0;
        LastError = TEXT("");
        LastCheckTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SkeletalMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ParticleSystemActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float EstimatedFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceWarning;

    FBuild_PerformanceMetrics()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        ParticleSystemActors = 0;
        LightActors = 0;
        EstimatedFrameTime = 0.0f;
        bPerformanceWarning = false;
    }
};

/**
 * Build Integration Manager - Coordinates integration between all game systems
 * Validates cross-system dependencies, monitors performance, and ensures build coherence
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckCrossSystemDependencies();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FBuild_PerformanceMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable();

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build")
    bool CheckModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void GenerateIntegrationReport();

    // System status
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateSystemStatus(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& ErrorMessage = TEXT(""));

    // Map persistence
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveMapSafely();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void BackupCurrentState();

protected:
    // System tracking
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FBuild_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bIntegrationValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    // Critical system names
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalSystems;

private:
    // Internal validation methods
    void ValidateCharacterSystem();
    void ValidateDinosaurSystem();
    void ValidateWorldGenSystem();
    void ValidateAudioSystem();
    void ValidateVFXSystem();
    void ValidatePhysicsSystem();

    // Performance helpers
    void CalculatePerformanceMetrics();
    void CheckActorCounts();
    void ValidateMemoryUsage();

    // Integration helpers
    void CheckSystemDependencies(const FString& SystemName);
    void ValidateComponentIntegration();
    void CheckSubsystemHealth();

    // Timers
    FTimerHandle ValidationTimerHandle;
    FTimerHandle PerformanceTimerHandle;
};