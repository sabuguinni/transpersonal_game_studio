#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Initializing,
    Stable,
    Unstable,
    Critical,
    Failed
};

USTRUCT(BlueprintType)
struct FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly)
    bool bIsFunctional;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceScore;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("Unknown");
        bIsLoaded = false;
        bIsFunctional = false;
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly)
    int32 DegenerateLabelCount;

    UPROPERTY(BlueprintReadOnly)
    float FrameRate;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_SystemHealth> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly)
    FString LastValidationTime;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        TotalActors = 0;
        DinosaurCount = 0;
        DegenerateLabelCount = 0;
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
        LastValidationTime = TEXT("Never");
    }
};

/**
 * Build Integration Manager - Agent #19 Core System
 * Manages cross-system integration, validation, and build health monitoring
 * Ensures all agent outputs work together cohesively
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupDegenerateLabels();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemCompatibility();

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemHealth CheckSystemHealth(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterSystemForMonitoring(const FString& SystemName, UClass* SystemClass);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetMemoryUsage() const;

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 CountActorsByType(const FString& ActorType) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<AActor*> GetActorsWithDegenerateLabels() const;

    // Integration Status
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_IntegrationStatus GetCurrentIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetIntegrationStatus(EBuild_IntegrationStatus NewStatus);

    // Validation Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationStatusChanged, EBuild_IntegrationStatus, NewStatus);
    UPROPERTY(BlueprintAssignable)
    FOnIntegrationStatusChanged OnIntegrationStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemValidationComplete, const FBuild_IntegrationReport&, Report);
    UPROPERTY(BlueprintAssignable)
    FOnSystemValidationComplete OnSystemValidationComplete;

protected:
    // Internal state
    UPROPERTY()
    EBuild_IntegrationStatus CurrentStatus;

    UPROPERTY()
    FBuild_IntegrationReport LastReport;

    UPROPERTY()
    TMap<FString, UClass*> MonitoredSystems;

    UPROPERTY()
    float LastValidationTime;

    // Internal validation functions
    void ValidateCharacterSystem();
    void ValidateWorldGeneration();
    void ValidateFoliageSystem();
    void ValidateCrowdSimulation();
    void ValidateLightingSystem();
    void ValidateTerrainSystem();
    
    // Utility functions
    bool IsSystemClassLoaded(UClass* SystemClass) const;
    int32 CountDegenerateLabels() const;
    void OptimizePerformance();
    void GenerateIntegrationReport();
};