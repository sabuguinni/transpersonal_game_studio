#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

// Forward declarations
class UEng_SystemManager;
class UEng_PerformanceMonitor;

/**
 * Core Architecture Status for Engine Systems
 */
UENUM(BlueprintType)
enum class EEng_ArchitectureStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

/**
 * Architecture Performance Metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
        : FrameTime(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , DrawCalls(0)
        , ActiveActors(0)
        , MemoryUsageMB(0.0f)
    {
    }
};

/**
 * System Health Information
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    EEng_ArchitectureStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    int32 ErrorCount;

    FEng_SystemHealth()
        : SystemName(TEXT("Unknown"))
        , Status(EEng_ArchitectureStatus::Uninitialized)
        , LastUpdateTime(0.0f)
        , LastError(TEXT(""))
        , ErrorCount(0)
    {
    }
};

/**
 * Core Architecture Manager - Central hub for all engine architecture systems
 * Manages system initialization, health monitoring, and performance tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ArchitectureStatus GetArchitectureStatus() const;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System Management")
    void RegisterSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "System Management")
    TArray<FEng_SystemHealth> GetSystemHealthReport() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    // Error Handling
    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    void ReportSystemError(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    void ClearSystemErrors(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Error Handling")
    TArray<FString> GetSystemErrors(const FString& SystemName) const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateArchitectureIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RunArchitectureDiagnostics();

protected:
    // Core status
    UPROPERTY(BlueprintReadOnly, Category = "Architecture Status")
    EEng_ArchitectureStatus ArchitectureStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Status")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture Status")
    float InitializationTime;

    // System registry
    UPROPERTY()
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    TArray<FEng_SystemHealth> SystemHealthData;

    // Performance data
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastPerformanceUpdate;

    // Error tracking
    UPROPERTY()
    TMap<FString, TArray<FString>> SystemErrors;

    // Performance limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    float MaxMemoryUsageMB;

private:
    // Internal helpers
    void UpdateSystemHealth();
    void ValidateSystemIntegrity();
    FEng_SystemHealth* FindSystemHealth(const FString& SystemName);
    void CleanupInvalidSystems();
};

/**
 * Architecture Component - Attach to actors that need architecture monitoring
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitectureComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitectureComponent();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Component Management
    UFUNCTION(BlueprintCallable, Category = "Architecture Component")
    void RegisterWithArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture Component")
    void UnregisterFromArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture Component")
    bool IsRegisteredWithArchitecture() const;

    // Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Health Monitoring")
    void ReportComponentHealth(EEng_ArchitectureStatus Status, const FString& Message = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Health Monitoring")
    EEng_ArchitectureStatus GetComponentStatus() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Settings")
    FString ComponentSystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Component Status")
    EEng_ArchitectureStatus ComponentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Component Status")
    bool bIsRegistered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring Settings")
    bool bEnableHealthMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring Settings")
    float HealthCheckInterval;

private:
    float LastHealthCheck;
    TWeakObjectPtr<UEng_CoreArchitecture> ArchitectureSubsystem;
};