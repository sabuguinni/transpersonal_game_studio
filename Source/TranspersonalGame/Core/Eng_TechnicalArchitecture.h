#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

// Forward declarations
class UBiomeManager;
class UCore_PhysicsManager;
class UArchitectureValidationSuite;

/**
 * Engine Architect's Technical Architecture Framework
 * Defines the core technical standards and validation systems for the entire project
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High           UMETA(DisplayName = "High"),
    Medium         UMETA(DisplayName = "Medium"),
    Low            UMETA(DisplayName = "Low"),
    Background     UMETA(DisplayName = "Background")
};

UENUM(BlueprintType)
enum class EEng_ValidationResult : uint8
{
    Pass           UMETA(DisplayName = "Pass"),
    Warning        UMETA(DisplayName = "Warning"),
    Fail           UMETA(DisplayName = "Fail"),
    Critical       UMETA(DisplayName = "Critical"),
    NotTested      UMETA(DisplayName = "Not Tested")
};

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core           UMETA(DisplayName = "Core Systems"),
    Gameplay       UMETA(DisplayName = "Gameplay"),
    Rendering      UMETA(DisplayName = "Rendering"),
    Audio          UMETA(DisplayName = "Audio"),
    Physics        UMETA(DisplayName = "Physics"),
    AI             UMETA(DisplayName = "AI"),
    Networking     UMETA(DisplayName = "Networking"),
    UI             UMETA(DisplayName = "User Interface")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveActors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PhysicsBodies = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CPUUsage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float GPUUsage = 0.0f;

    FEng_SystemMetrics()
    {
        FrameTime = 16.67f; // Target 60fps
        MemoryUsage = 0.0f;
        ActiveActors = 0;
        PhysicsBodies = 0;
        CPUUsage = 0.0f;
        GPUUsage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalStandard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    FString StandardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    EEng_SystemPriority Priority = EEng_SystemPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    bool bMandatory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    float MaxFrameTime = 16.67f; // 60fps target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standard")
    float MaxMemoryMB = 8192.0f; // 8GB limit

    FEng_ArchitecturalStandard()
    {
        StandardName = TEXT("Default Standard");
        Description = TEXT("Default architectural standard");
        Priority = EEng_SystemPriority::Medium;
        bMandatory = true;
        MaxFrameTime = 16.67f;
        MaxMemoryMB = 8192.0f;
    }
};

/**
 * Technical Architecture Subsystem - Core engine management
 * Manages all technical standards, validation, and cross-system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateSystemCompliance();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_SystemMetrics GetCurrentSystemMetrics();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void RegisterSubsystem(const FString& SubsystemName, EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void UnregisterSubsystem(const FString& SubsystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(float TargetFrameTime, float MaxMemoryMB);

    // Validation and Compliance
    UFUNCTION(BlueprintCallable, Category = "Validation")
    EEng_ValidationResult ValidateModule(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetValidationErrors();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RunFullSystemValidation();

    // Cross-System Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegratePhysicsWithBiomes();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCrossSystemDependencies();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void PrintArchitecturalStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void GenerateSystemReport();

protected:
    // Core architectural standards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ArchitecturalStandard> ArchitecturalStandards;

    // System metrics tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    FEng_SystemMetrics CurrentMetrics;

    // Registered subsystems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    TMap<FString, EEng_ModuleType> RegisteredSubsystems;

    // Validation results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f; // 8GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors = 10000;

private:
    // Internal validation methods
    void ValidatePhysicsIntegration();
    void ValidateBiomeSystem();
    void ValidateCharacterSystems();
    void ValidateWorldGeneration();
    
    // Performance monitoring
    void CollectFrameTimeMetrics();
    void CollectMemoryMetrics();
    void CollectActorMetrics();
    
    // System initialization
    void InitializeStandardsDatabase();
    void SetupPerformanceMonitoring();
    void RegisterCoreSubsystems();
};

/**
 * World-level Technical Architecture Manager
 * Handles world-specific technical validation and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World-specific validation
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldSetup();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldCompliant();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void IntegrateWorldSystems();

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomePhysics();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
    bool bWorldValidated = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World")
    TArray<FString> WorldValidationErrors;

private:
    void ValidateTerrainSetup();
    void ValidateLightingSetup();
    void ValidatePhysicsWorld();
    void ValidateNavMesh();
};

#include "Eng_TechnicalArchitecture.generated.h"