#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitectCore.generated.h"

/**
 * Engine Architect Core System
 * Defines the technical architecture foundation for the entire game
 * All other systems must follow these architectural rules
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    FEng_SystemValidation()
    {
        SystemName = TEXT("Unknown");
        bIsValid = false;
        ValidationMessage = TEXT("Not validated");
        LastValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMeetsPerformanceTarget;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        ActorCount = 0;
        DinosaurCount = 0;
        bMeetsPerformanceTarget = false;
    }
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Optional    UMETA(DisplayName = "Optional")
};

UENUM(BlueprintType)
enum class EEng_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

/**
 * World Subsystem for Engine Architecture Management
 * Validates and monitors all game systems according to architectural rules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_SystemValidation ValidateSpecificSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemValidation> GetAllSystemValidations();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckPerformanceCompliance();

    // Dinosaur System Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 ValidateDinosaurSpawning();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePlayerCharacterSetup();

    // World Generation Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateBiomeDistribution();

    // Compilation and Module Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    TArray<FString> GetCompilationErrors();

    // Milestone 1 Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetMilestone1Status();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FEng_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FEng_PerformanceMetrics LastPerformanceCheck;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bArchitectureValid;

private:
    void InitializeSystemValidations();
    void UpdatePerformanceMetrics();
    bool CheckSystemDependencies();
    bool ValidateAssetPaths();
    void LogArchitectureStatus();
};

/**
 * Game Instance Subsystem for Engine Architecture Management
 * Persists across level changes and manages global architecture state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectGameInstanceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Global Architecture Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceArchitecturalRules();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemCompliant(const FString& SystemName);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetRegisteredModules();

    // Performance Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceTargets(float TargetFPS, float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforcePerformanceLimits();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, EEng_SystemPriority> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float MaxMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bArchitectureInitialized;

private:
    void LoadArchitectureConfiguration();
    void SaveArchitectureState();
    void ValidateGlobalRules();
};

/**
 * Actor class for Engine Architecture debugging and visualization
 * Can be placed in levels to monitor architecture compliance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineArchitectDebugActor : public AActor
{
    GENERATED_BODY()

public:
    AEngineArchitectDebugActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* DebugMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Settings")
    bool bShowDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Settings")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Debug Info")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Debug Info")
    bool bSystemsValid;

public:
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UpdateDebugDisplay();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugInfo();

private:
    float LastValidationTime;
    void PerformValidationCheck();
    void UpdateDebugMesh();
};