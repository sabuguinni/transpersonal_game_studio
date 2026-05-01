#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

/**
 * Core Architecture System - Engine Architect Agent #02
 * Manages the fundamental technical architecture of the game
 * Ensures all systems follow established patterns and performance requirements
 */

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    Poor            UMETA(DisplayName = "Poor"),
    Acceptable      UMETA(DisplayName = "Acceptable"),
    Good            UMETA(DisplayName = "Good"),
    Excellent       UMETA(DisplayName = "Excellent")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    TArray<FString> Dependencies;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Status = EEng_SystemStatus::Uninitialized;
        InitializationTime = 0.0f;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceLevel OverallLevel;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        OverallLevel = EEng_PerformanceLevel::Acceptable;
    }
};

/**
 * Core Architecture Manager - Central system for technical architecture
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

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void RegisterSystem(const FString& SystemName, int32 Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void SetSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    bool IsPerformanceAcceptable() const;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    TArray<FString> GetSystemInitializationOrder() const;

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void InitializeAllSystems();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void LogSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

protected:
    // Registered systems
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Performance tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Architecture", meta = (AllowPrivateAccess = "true"))
    FEng_PerformanceMetrics CurrentMetrics;

    // Architecture settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Architecture")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Architecture")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Architecture")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Architecture")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Architecture")
    float PerformanceUpdateInterval;

private:
    // Internal methods
    void InitializeCoreArchitecture();
    void RegisterCoreGameSystems();
    bool CheckSystemDependencies(const FString& SystemName, TArray<FString>& VisitedSystems) const;
    void CalculatePerformanceLevel();

    // Timer for performance monitoring
    FTimerHandle PerformanceTimerHandle;
};