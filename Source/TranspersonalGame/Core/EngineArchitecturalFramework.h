#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitecturalFramework.generated.h"

/**
 * Engine Architectural Framework - Core system that defines and enforces
 * the technical architecture across all game systems.
 * 
 * This is the foundational system that ensures all other systems follow
 * the established architectural patterns and performance requirements.
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical System"),
    High            UMETA(DisplayName = "High Priority"),
    Normal          UMETA(DisplayName = "Normal Priority"),
    Low             UMETA(DisplayName = "Low Priority"),
    Background      UMETA(DisplayName = "Background System")
};

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Engine          UMETA(DisplayName = "Engine Layer"),
    Core            UMETA(DisplayName = "Core Systems"),
    Gameplay        UMETA(DisplayName = "Gameplay Systems"),
    Content         UMETA(DisplayName = "Content Layer"),
    UI              UMETA(DisplayName = "User Interface"),
    Debug           UMETA(DisplayName = "Debug Systems")
};

UENUM(BlueprintType)
enum class EEng_ValidationStatus : uint8
{
    Valid           UMETA(DisplayName = "Valid"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical Error"),
    Unknown         UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemPriority Priority = EEng_SystemPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_ArchitecturalLayer Layer = EEng_ArchitecturalLayer::Gameplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float PerformanceBudget = 16.67f; // 60fps target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    TArray<FString> Dependencies;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown System");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage = 0.0f;

    FEng_PerformanceMetrics()
    {
        // Initialize to safe defaults
    }
};

/**
 * Main Engine Architectural Framework Subsystem
 * Manages all architectural compliance and system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecturalFramework();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FEng_SystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetRegisteredSystems() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdatePerformanceMetrics(const FEng_PerformanceMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ValidationStatus ValidateSystemArchitecture(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceArchitecturalCompliance();

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemDependencies(const FString& SystemName);

    // Performance Budget Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool SetSystemPerformanceBudget(const FString& SystemName, float BudgetMs);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceBudget(const FString& SystemName) const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogArchitecturalStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalWarnings();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalErrors();

protected:
    // Core system registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architecture")
    FEng_PerformanceMetrics CurrentMetrics;

    // Validation results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> ArchitecturalWarnings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> ArchitecturalErrors;

    // Internal validation methods
    bool ValidateSystemLayer(const FEng_SystemInfo& SystemInfo);
    bool ValidateSystemPriority(const FEng_SystemInfo& SystemInfo);
    bool ValidatePerformanceBudget(const FEng_SystemInfo& SystemInfo);
    void UpdateValidationStatus();

private:
    // Internal state tracking
    bool bIsInitialized = false;
    float TotalPerformanceBudget = 16.67f; // 60fps target
    int32 ValidationFrameCounter = 0;
};