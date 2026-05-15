#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FDateTime LastValidation;

    FBuild_SystemHealthData()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_SystemStatus::Unknown;
        ActorCount = 0;
        PerformanceScore = 0.0f;
        LastError = TEXT("");
        LastValidation = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    int32 RequiredActors;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    int32 CurrentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    bool bIsPopulated;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    TArray<FString> MissingAssetTypes;

    FBuild_BiomeValidationData()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        RequiredActors = 500;
        CurrentActors = 0;
        bIsPopulated = false;
    }
};

/**
 * Final Integration Orchestrator - Manages complete system integration and build validation
 * Coordinates all subsystems, validates biome population, and ensures stable build state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "System Health")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Health")
    TArray<FBuild_SystemHealthData> GetAllSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    float GetOverallSystemHealth();

    // Biome Validation
    UFUNCTION(BlueprintCallable, Category = "Biome Validation")
    TArray<FBuild_BiomeValidationData> GetBiomeValidationData();

    UFUNCTION(BlueprintCallable, Category = "Biome Validation")
    bool AreBiomesProperlyPopulated();

    UFUNCTION(BlueprintCallable, Category = "Biome Validation")
    int32 GetTotalActorCount();

    // Asset Pipeline Validation
    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    bool ValidateFBXImportPipeline();

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    TArray<FString> GetAvailableAssets();

    UFUNCTION(BlueprintCallable, Category = "Asset Pipeline")
    bool TestAssetSpawning();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits();

    // Build Validation
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetCompilationErrors();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsReadyForProduction();

protected:
    // System Health Data
    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    TMap<FString, FBuild_SystemHealthData> SystemHealthMap;

    // Biome Validation Data
    UPROPERTY(BlueprintReadOnly, Category = "Biome Validation")
    TArray<FBuild_BiomeValidationData> BiomeValidationArray;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaximumMemoryUsage = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaximumActorCount = 20000;

    // Validation Flags
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bSystemsValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bBiomesValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bAssetPipelineValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bPerformanceValidated = false;

private:
    // Internal validation functions
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidatePhysicsSystems();
    void ValidateVFXSystems();
    void ValidateAudioSystems();
    void ValidateNPCSystems();
    void ValidateCrowdSystems();

    // Biome validation helpers
    void ValidateSavanaBiome();
    void ValidatePantanoBiome();
    void ValidateFlorestaBiome();
    void ValidateDesertoBiome();
    void ValidateMontanhaBiome();

    // Performance monitoring helpers
    void UpdatePerformanceMetrics();
    void CheckMemoryLeaks();
    void ValidateActorCounts();

    // Asset pipeline helpers
    void TestFBXImport();
    void ValidateAssetRegistry();
    void CheckAssetIntegrity();
};