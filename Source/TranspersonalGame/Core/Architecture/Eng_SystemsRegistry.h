#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemsRegistry.generated.h"

/**
 * Engine Architect - CYCLE 002 Critical Architecture Foundation
 * 
 * Systems Registry - Central coordination hub for all game systems
 * This is the architectural backbone that ensures proper initialization order,
 * dependency management, and system lifecycle control.
 * 
 * DESIGN PRINCIPLES:
 * - Single source of truth for system status
 * - Enforces initialization dependencies  
 * - Provides performance monitoring hooks
 * - Enables graceful shutdown sequences
 * - Supports hot-reloading of systems
 */

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready           UMETA(DisplayName = "Ready"),
    Running         UMETA(DisplayName = "Running"),
    Paused          UMETA(DisplayName = "Paused"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),     // Physics, Core Systems
    High            UMETA(DisplayName = "High"),        // World Gen, Character
    Medium          UMETA(DisplayName = "Medium"),      // AI, Audio
    Low             UMETA(DisplayName = "Low"),         // UI, VFX
    Background      UMETA(DisplayName = "Background")   // Analytics, Debug
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString ErrorMessage;

    FEng_SystemInfo()
    {
        SystemName = "";
        Status = EEng_SystemStatus::Uninitialized;
        Priority = EEng_SystemPriority::Medium;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        ErrorMessage = "";
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemsRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemsRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool UnregisterSystem(const FString& SystemName);

    // System Status Management
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus, const FString& ErrorMessage = "");

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool IsSystemReady(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool AreSystemDependenciesReady(const FString& SystemName) const;

    // System Query Functions
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    TArray<FString> GetSystemsByPriority(EEng_SystemPriority Priority) const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    TArray<FString> GetSystemsByStatus(EEng_SystemStatus Status) const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    // Initialization Control
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void InitializeSystemsInOrder();

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void RestartSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void UpdateSystemPerformance(const FString& SystemName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    float GetSystemAverageUpdateTime(const FString& SystemName) const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    void LogSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    bool ValidateSystemDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Systems Registry")
    TArray<FString> GetSystemsWithErrors() const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemStatusChanged, FString, SystemName, EEng_SystemStatus, NewStatus);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemError, FString, SystemName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSystemsReady);

    UPROPERTY(BlueprintAssignable, Category = "Systems Registry Events")
    FOnSystemStatusChanged OnSystemStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Systems Registry Events")
    FOnSystemError OnSystemError;

    UPROPERTY(BlueprintAssignable, Category = "Systems Registry Events")
    FOnAllSystemsReady OnAllSystemsReady;

protected:
    // Core system registry
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Performance tracking
    UPROPERTY()
    TMap<FString, TArray<float>> SystemUpdateTimes;

    // Internal helper functions
    void SortSystemsByPriority(TArray<FString>& SystemNames) const;
    bool CheckCircularDependencies(const FString& SystemName, TSet<FString>& VisitedSystems) const;
    void InitializeSystem(const FString& SystemName);
    void ShutdownSystem(const FString& SystemName);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    int32 MaxPerformanceSamples;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float SystemTimeoutSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bEnablePerformanceTracking;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bLogSystemEvents;
};