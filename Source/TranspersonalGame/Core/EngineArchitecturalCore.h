#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "UObject/ObjectMacros.h"
#include "SharedTypes.h"
#include "EngineArchitecturalCore.generated.h"

class UCore_PhysicsManager;
class UCore_CollisionManager;
class UBiomeManager;

/**
 * Architectural compliance levels for system validation
 */
UENUM(BlueprintType)
enum class EEng_ArchitecturalCompliance : uint8
{
    NonCompliant    UMETA(DisplayName = "Non-Compliant"),
    BasicCompliance UMETA(DisplayName = "Basic Compliance"),
    FullCompliance  UMETA(DisplayName = "Full Compliance"),
    ExemplarCompliance UMETA(DisplayName = "Exemplar Compliance")
};

/**
 * Performance tier classifications for system optimization
 */
UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Critical        UMETA(DisplayName = "Critical Performance"),
    High            UMETA(DisplayName = "High Performance"),
    Standard        UMETA(DisplayName = "Standard Performance"),
    Background      UMETA(DisplayName = "Background Performance")
};

/**
 * Module dependency validation results
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependencyStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    bool bHasValidDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    TArray<FString> MissingDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    EEng_ArchitecturalCompliance ComplianceLevel;

    FEng_ModuleDependencyStatus()
        : bIsLoaded(false)
        , bHasValidDependencies(false)
        , ComplianceLevel(EEng_ArchitecturalCompliance::NonCompliant)
    {}
};

/**
 * System performance metrics for architectural monitoring
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceTier PerformanceTier;

    FEng_SystemPerformanceMetrics()
        : AverageFrameTime(0.0f)
        , PeakFrameTime(0.0f)
        , ActiveComponents(0)
        , MemoryUsageMB(0.0f)
        , PerformanceTier(EEng_PerformanceTier::Standard)
    {}
};

/**
 * Central Engine Architectural Core Subsystem
 * 
 * This subsystem serves as the architectural backbone of the entire game,
 * managing module dependencies, performance constraints, and system integration.
 * It enforces architectural standards across all game systems and provides
 * centralized monitoring and validation capabilities.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * Register a system module with the architectural core
     * @param ModuleName Name of the module to register
     * @param Dependencies List of required dependencies
     * @param PerformanceTier Performance classification for this module
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystemModule(const FString& ModuleName, const TArray<FString>& Dependencies, EEng_PerformanceTier PerformanceTier);

    /**
     * Validate all registered modules and their dependencies
     * @return Array of module status information
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleDependencyStatus> ValidateModuleDependencies();

    /**
     * Get current performance metrics for all registered systems
     * @return Array of performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemPerformanceMetrics> GetSystemPerformanceMetrics();

    /**
     * Force architectural compliance check on all systems
     * @return Overall compliance level of the architecture
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ArchitecturalCompliance ValidateArchitecturalCompliance();

    /**
     * Initialize core physics systems with proper architectural integration
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeCorePhysicsSystems();

    /**
     * Setup collision systems for spawned dinosaurs
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetupDinosaurCollisionSystems();

    /**
     * Validate and optimize performance for current world state
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void OptimizeWorldPerformance();

    /**
     * Get architectural health status
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureHealthy() const;

protected:
    /**
     * Registered system modules and their metadata
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_ModuleDependencyStatus> RegisteredModules;

    /**
     * Performance metrics cache
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemPerformanceMetrics> PerformanceMetrics;

    /**
     * Core physics manager reference
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    /**
     * Collision system manager reference
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_CollisionManager> CollisionManager;

    /**
     * Biome management system reference
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBiomeManager> BiomeManager;

    /**
     * Overall architectural health status
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bArchitectureHealthy;

    /**
     * Last validation timestamp
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

private:
    /**
     * Internal validation of module dependencies
     */
    bool ValidateModuleDependency(const FString& ModuleName, const TArray<FString>& Dependencies);

    /**
     * Update performance metrics for a specific system
     */
    void UpdateSystemPerformanceMetrics(const FString& SystemName);

    /**
     * Initialize architectural standards and constraints
     */
    void InitializeArchitecturalStandards();

    /**
     * Validate system integration points
     */
    bool ValidateSystemIntegration();
};