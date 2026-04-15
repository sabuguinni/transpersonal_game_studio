#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Engine Architecture Performance Metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

/**
 * System Health Status
 */
UENUM(BlueprintType)
enum class EEng_SystemHealth : uint8
{
    Optimal     UMETA(DisplayName = "Optimal"),
    Good        UMETA(DisplayName = "Good"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

/**
 * Architecture Validation Result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_SystemHealth HealthStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString StatusMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FEng_ValidationResult()
    {
        SystemName = TEXT("");
        HealthStatus = EEng_SystemHealth::Optimal;
        StatusMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

/**
 * Engine Architecture Manager - Central system for monitoring and validating
 * the technical architecture of the entire game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AEngineArchitectureManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceOptimal();

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ValidationResult> ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    // Architecture Rules Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateWorldPartitionSetup();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateMemoryBudgets();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceTargets();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void RunFullSystemDiagnostics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogSystemStatus();

protected:
    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoringPerformance;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastMonitoringTime;

    // System registry
    UPROPERTY()
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    // Validation results
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FEng_ValidationResult> LastValidationResults;

    // Architecture rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Rules")
    int32 MaxDrawCalls;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void ValidatePerformanceRules();
    FEng_ValidationResult ValidateWorldGeneration();
    FEng_ValidationResult ValidatePhysicsSystem();
    FEng_ValidationResult ValidateAISystem();
    FEng_ValidationResult ValidateRenderingSystem();
    FEng_ValidationResult ValidateAudioSystem();

    // Performance calculation helpers
    float CalculateFrameRate();
    float CalculateMemoryUsage();
    int32 CountActiveActors();
    int32 CountActiveComponents();
    int32 GetDrawCallCount();
    int32 GetTriangleCount();
};

/**
 * Engine Architecture Subsystem - Game Instance level management
 */
UCLASS()
class TRANSPERSONALGAME_API UEngineArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    AEngineArchitectureManager* GetArchitectureManager();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetArchitectureManager(AEngineArchitectureManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureHealthy();

protected:
    UPROPERTY()
    TWeakObjectPtr<AEngineArchitectureManager> ArchitectureManager;
};