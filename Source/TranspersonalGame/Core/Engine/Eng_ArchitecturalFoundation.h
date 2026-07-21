#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Eng_ArchitecturalFoundation.generated.h"

/**
 * ENGINE ARCHITECT - CYCLE 010 ARCHITECTURAL FOUNDATION
 * 
 * This is the central architectural foundation that defines the core structure
 * and rules for the entire TranspersonalGame project. All other systems must
 * conform to these architectural principles.
 * 
 * CRITICAL ARCHITECTURAL PRINCIPLES:
 * 1. MODULAR DESIGN - Every system is self-contained with clear interfaces
 * 2. DEPENDENCY INJECTION - Systems register with central registry
 * 3. EVENT-DRIVEN COMMUNICATION - No direct coupling between systems
 * 4. PERFORMANCE FIRST - 60fps PC / 30fps console mandatory
 * 5. SCALABILITY - Support for 50,000+ actors via Mass Entity
 * 6. REALISTIC SURVIVAL - No spiritual/mystical content, pure dinosaur survival
 */

// Core System Status Tracking
UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized   UMETA(DisplayName = "Uninitialized"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Ready           UMETA(DisplayName = "Ready"),
    Running         UMETA(DisplayName = "Running"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

// System Priority Levels
UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),      // Core engine systems
    High            UMETA(DisplayName = "High"),          // Gameplay systems
    Medium          UMETA(DisplayName = "Medium"),        // World systems
    Low             UMETA(DisplayName = "Low"),           // Cosmetic systems
    Background      UMETA(DisplayName = "Background")     // Analytics/telemetry
};

// Performance Budget Categories
UENUM(BlueprintType)
enum class EEng_PerformanceBudget : uint8
{
    CPU_GameThread      UMETA(DisplayName = "CPU Game Thread"),
    CPU_RenderThread    UMETA(DisplayName = "CPU Render Thread"),
    GPU_Rendering       UMETA(DisplayName = "GPU Rendering"),
    Memory_Heap         UMETA(DisplayName = "Memory Heap"),
    Memory_VRAM         UMETA(DisplayName = "Memory VRAM"),
    Network_Bandwidth   UMETA(DisplayName = "Network Bandwidth")
};

// System Registration Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 AgentNumber;

    FEng_SystemRegistration()
    {
        SystemName = "Unknown";
        Priority = EEng_SystemPriority::Medium;
        Status = EEng_SystemStatus::Uninitialized;
        InitializationTime = 0.0f;
        AgentNumber = 0;
    }
};

// Performance Metrics
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
    float GPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameTime = 16.67f; // 60fps target
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        ActiveActors = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * ARCHITECTURAL FOUNDATION SUBSYSTEM
 * 
 * This is the central coordination hub for all game systems.
 * Every agent must register their systems here to ensure proper
 * initialization order and dependency management.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFoundation : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFoundation();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FEng_SystemRegistration& SystemData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName);

    // Dependency Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetInitializationOrder();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsPerformanceTargetMet();

    // System Control
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ShutdownAllSystems();

    // Editor Tools
    UFUNCTION(BlueprintCallable, CallInEditor = true, Category = "Architecture")
    void ValidateArchitecture();

    UFUNCTION(BlueprintCallable, CallInEditor = true, Category = "Architecture")
    void GenerateSystemReport();

protected:
    // Registered Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FEng_SystemRegistration> RegisteredSystems;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    // Architectural Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxMemoryUsageMB;

private:
    // Internal system management
    void UpdatePerformanceMetrics();
    bool CheckSystemDependencies(const FString& SystemName);
    void LogArchitecturalViolation(const FString& Violation);
};

/**
 * WORLD-LEVEL ARCHITECTURAL COORDINATOR
 * 
 * Manages world-specific architectural concerns like streaming,
 * level management, and world partition coordination.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitecturalCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitecturalCoordinator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateWorldConfiguration();

    // Streaming Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ConfigureWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void OptimizeStreamingSettings();

    // Level Management
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterLevelSystems();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
    bool bWorldSystemsInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
    bool bStreamingConfigured;

private:
    void SetupPerformanceBudgets();
    void ConfigureActorCulling();
};