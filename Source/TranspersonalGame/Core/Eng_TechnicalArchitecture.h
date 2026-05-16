#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "../SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core            UMETA(DisplayName = "Core"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    Environment     UMETA(DisplayName = "Environment"),
    Character       UMETA(DisplayName = "Character"),
    AI              UMETA(DisplayName = "AI"),
    Combat          UMETA(DisplayName = "Combat"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX"),
    UI              UMETA(DisplayName = "UI"),
    Performance     UMETA(DisplayName = "Performance")
};

UENUM(BlueprintType)
enum class EEng_MemoryBudget : uint8
{
    Critical        UMETA(DisplayName = "Critical - 512MB"),
    High            UMETA(DisplayName = "High - 256MB"),
    Medium          UMETA(DisplayName = "Medium - 128MB"),
    Low             UMETA(DisplayName = "Low - 64MB"),
    Minimal         UMETA(DisplayName = "Minimal - 32MB")
};

USTRUCT(BlueprintType)
struct FEng_ModuleSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    EEng_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    EEng_MemoryBudget MemoryBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    bool bRequiresWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Spec")
    TArray<FString> AllowedAssetTypes;

    FEng_ModuleSpec()
    {
        ModuleType = EEng_ModuleType::Core;
        ModuleName = TEXT("");
        MemoryBudget = EEng_MemoryBudget::Medium;
        MaxActorsPerBiome = 500;
        TargetFrameTime = 16.67f; // 60 FPS
        bRequiresWorldPartition = false;
    }
};

USTRUCT(BlueprintType)
struct FEng_PerformanceRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    EEng_ModuleType ApplicableModule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    int32 MaxTextureSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    bool bRequiresLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Rule")
    int32 LODLevels;

    FEng_PerformanceRule()
    {
        RuleName = TEXT("");
        ApplicableModule = EEng_ModuleType::Core;
        MaxTriangles = 10000;
        MaxTextureSize = 2048;
        MaxDrawDistance = 5000.0f;
        bRequiresLOD = true;
        LODLevels = 3;
    }
};

USTRUCT(BlueprintType)
struct FEng_BiomeArchitecture
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    FVector WorldCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    int32 MinActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    TArray<FString> AllowedAssetCategories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    EEng_MemoryBudget MemoryAllocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Architecture")
    bool bRequiresStreamingLOD;

    FEng_BiomeArchitecture()
    {
        BiomeType = EBiomeType::Savanna;
        WorldCoordinates = FVector::ZeroVector;
        BiomeRadius = 200000.0f; // 2km radius
        MinActorCount = 500;
        MaxActorCount = 2000;
        MemoryAllocation = EEng_MemoryBudget::Medium;
        bRequiresStreamingLOD = true;
    }
};

/**
 * Technical Architecture Manager - Defines and enforces technical rules for all game systems
 * This is the central authority for performance budgets, memory management, and module dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    // Core Architecture Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    TArray<FEng_ModuleSpec> ModuleSpecifications;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    TArray<FEng_PerformanceRule> PerformanceRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    TArray<FEng_BiomeArchitecture> BiomeArchitectures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    FString ProjectVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    float WorldSizeKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    bool bWorldPartitionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical Architecture")
    int32 MaxConcurrentActors;

public:
    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void RegisterModule(const FEng_ModuleSpec& ModuleSpec);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_ModuleSpec GetModuleSpec(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateModuleDependencies(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetModuleDependencies(EEng_ModuleType ModuleType);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void AddPerformanceRule(const FEng_PerformanceRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_PerformanceRule GetPerformanceRule(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateAssetPerformance(const FString& AssetPath, EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    int32 GetMaxTrianglesForModule(EEng_ModuleType ModuleType);

    // Biome Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetupBiomeArchitecture(EBiomeType BiomeType, FVector Coordinates, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_BiomeArchitecture GetBiomeArchitecture(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool CanSpawnActorInBiome(EBiomeType BiomeType, const FString& AssetCategory);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    int32 GetActorBudgetForBiome(EBiomeType BiomeType);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    float GetMemoryBudgetMB(EEng_MemoryBudget BudgetType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool CheckMemoryUsage(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void OptimizeMemoryForBiome(EBiomeType BiomeType);

    // World Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetWorldSize(float SizeInKm);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ShouldUseWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FVector GetWorldBounds();

    // Agent Communication Rules
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<int32> GetAllowedAgentsForModule(EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool CanAgentModifyModule(int32 AgentNumber, EEng_ModuleType ModuleType);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void RegisterAgentActivity(int32 AgentNumber, EEng_ModuleType ModuleType, const FString& Activity);

    // Validation and Enforcement
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetArchitectureViolations();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FString GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Technical Architecture")
    void RefreshArchitectureRules();

private:
    void InitializeDefaultModules();
    void InitializeDefaultPerformanceRules();
    void InitializeDefaultBiomeArchitectures();
    void ValidateWorldPartitionRequirements();
};

#include "Eng_TechnicalArchitecture.generated.h"