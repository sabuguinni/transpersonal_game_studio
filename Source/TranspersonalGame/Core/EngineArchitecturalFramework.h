#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitecturalFramework.generated.h"

// Forward declarations
class UBiomeManager;
class UAssetProductionManager;
class UBuildSystemManager;
class UArchitectureValidationSuite;

/**
 * Engine Architectural Framework - Core system registration and validation
 * Defines technical standards, system priorities, and architectural compliance
 * This is the foundation system that all other game systems must register with
 */
UCLASS(BlueprintType)
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
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority = 100);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdateSystemPerformance(const FString& SystemName, float DeltaTime, int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName) const;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RunFullArchitecturalValidation();

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void AddSystemDependency(const FString& SystemName, const FString& DependencyName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool AreSystemDependenciesMet(const FString& SystemName) const;

    // Critical System Access
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UBiomeManager* GetBiomeManager() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UAssetProductionManager* GetAssetProductionManager() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UBuildSystemManager* GetBuildSystemManager() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UArchitectureValidationSuite* GetValidationSuite() const;

    // Engine Standards Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforcePerformanceStandards();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateModuleCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void GenerateArchitecturalReport();

protected:
    // System Registry
    UPROPERTY()
    TMap<FString, UObject*> RegisteredSystems;

    UPROPERTY()
    TMap<FString, int32> SystemPriorities;

    UPROPERTY()
    TMap<FString, float> SystemPerformanceMetrics;

    UPROPERTY()
    TMap<FString, TArray<FString>> SystemDependencies;

    // Core System References
    UPROPERTY()
    UBiomeManager* BiomeManagerRef;

    UPROPERTY()
    UAssetProductionManager* AssetProductionManagerRef;

    UPROPERTY()
    UBuildSystemManager* BuildSystemManagerRef;

    UPROPERTY()
    UArchitectureValidationSuite* ValidationSuiteRef;

    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxAllowedFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActorsPerSystem = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnablePerformanceMonitoring = true;

    // Architectural Standards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bEnforceModuleCompliance = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bValidateSystemDependencies = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bGenerateComplianceReports = true;

private:
    void InitializeCoreSystemReferences();
    void ValidateSystemIntegrity();
    void LogArchitecturalStatus();
    bool CheckSystemPerformance(const FString& SystemName) const;
    void EnforceSystemLimits();
};

/**
 * Engine System Priority Levels
 * Defines the initialization and update order for all game systems
 */
UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    = 0     UMETA(DisplayName = "Critical (0-19)"),
    High        = 20    UMETA(DisplayName = "High (20-39)"),
    Normal      = 40    UMETA(DisplayName = "Normal (40-79)"),
    Low         = 80    UMETA(DisplayName = "Low (80-99)"),
    Background  = 100   UMETA(DisplayName = "Background (100+)")
};

/**
 * Engine Performance Metrics
 * Standardized performance measurement structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PeakFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceWithinLimits = true;

    FEng_PerformanceMetrics()
    {
        AverageFrameTime = 0.0f;
        PeakFrameTime = 0.0f;
        ActiveActorCount = 0;
        MemoryUsageMB = 0.0f;
        bPerformanceWithinLimits = true;
    }
};

/**
 * Engine System Registration Info
 * Contains metadata about registered systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsValidated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FEng_PerformanceMetrics PerformanceData;

    FEng_SystemInfo()
    {
        SystemName = TEXT("");
        Priority = EEng_SystemPriority::Normal;
        bIsActive = true;
        bIsValidated = false;
    }
};