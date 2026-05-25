#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngArchitect_TechnicalSpecs.generated.h"

/**
 * Engine Architect Technical Specifications
 * Defines the core technical architecture and constraints for the entire game
 * All agents must follow these specifications - violations will cause compilation errors
 */

UENUM(BlueprintType)
enum class EEng_PerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS")
};

UENUM(BlueprintType)
enum class EEng_WorldPartitionMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Standard        UMETA(DisplayName = "Standard 4km²"),
    Large           UMETA(DisplayName = "Large 16km²"),
    Massive         UMETA(DisplayName = "Massive 64km²")
};

UENUM(BlueprintType)
enum class EEng_LODLevel : uint8
{
    LOD0_High       UMETA(DisplayName = "LOD0 High Quality"),
    LOD1_Medium     UMETA(DisplayName = "LOD1 Medium Quality"),
    LOD2_Low        UMETA(DisplayName = "LOD2 Low Quality"),
    LOD3_Culled     UMETA(DisplayName = "LOD3 Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceConstraints
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTrianglesPerFrame = 2000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTimeMS = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCrowdAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f;

    FEng_PerformanceConstraints()
    {
        MaxActorsPerFrame = 10000;
        MaxTrianglesPerFrame = 2000000;
        MaxFrameTimeMS = 16.67f;
        MaxCrowdAgents = 50000;
        CullingDistance = 10000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleArchitecture
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsCore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRequiresWorldPartition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_PerformanceTarget TargetPerformance = EEng_PerformanceTarget::PC_60FPS;

    FEng_ModuleArchitecture()
    {
        ModuleName = TEXT("");
        bIsCore = false;
        bRequiresWorldPartition = false;
        TargetPerformance = EEng_PerformanceTarget::PC_60FPS;
    }
};

/**
 * Engine Architect Technical Specifications Subsystem
 * Enforces technical constraints and architecture rules across all game systems
 */
UCLASS()
class TRANSPERSONALGAME_API UEngArchitect_TechnicalSpecs : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleArchitecture(const FEng_ModuleArchitecture& Module);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforcePerformanceConstraints(const FEng_PerformanceConstraints& Constraints);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetWorldPartitionMode(EEng_WorldPartitionMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_WorldPartitionMode GetWorldPartitionMode() const { return CurrentWorldPartitionMode; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    EEng_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistances(float LOD1Distance, float LOD2Distance, float CullDistance);

    // Module Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModule(const FEng_ModuleArchitecture& Module);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ModuleArchitecture> GetRegisteredModules() const { return RegisteredModules; }

    // Critical Architecture Rules (MANDATORY)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void EnforceArchitectureRules();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ValidateAllModules();

protected:
    UPROPERTY()
    TArray<FEng_ModuleArchitecture> RegisteredModules;

    UPROPERTY()
    FEng_PerformanceConstraints CurrentConstraints;

    UPROPERTY()
    EEng_WorldPartitionMode CurrentWorldPartitionMode = EEng_WorldPartitionMode::Standard;

    UPROPERTY()
    float LOD1Distance = 2000.0f;

    UPROPERTY()
    float LOD2Distance = 5000.0f;

    UPROPERTY()
    float CullDistance = 10000.0f;

private:
    void InitializeDefaultConstraints();
    void ValidateSystemRequirements();
    bool CheckModuleDependencies(const FEng_ModuleArchitecture& Module);
};