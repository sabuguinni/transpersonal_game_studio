#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/NoExportTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitect, Log, All);

/**
 * Engine Architect Framework - Core architectural patterns and validation
 * Defines the foundational rules that all other systems must follow
 * Created by Engine Architect Agent #02
 */

// Core architectural validation levels
UENUM(BlueprintType)
enum class EEng_ValidationLevel : uint8
{
    None = 0        UMETA(DisplayName = "No Validation"),
    Basic = 1       UMETA(DisplayName = "Basic Checks"),
    Standard = 2    UMETA(DisplayName = "Standard Validation"),
    Strict = 3      UMETA(DisplayName = "Strict Enforcement"),
    Critical = 4    UMETA(DisplayName = "Critical Systems")
};

// System dependency priorities
UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical Infrastructure"),
    Core = 1        UMETA(DisplayName = "Core Systems"),
    Gameplay = 2    UMETA(DisplayName = "Gameplay Systems"),
    Content = 3     UMETA(DisplayName = "Content Systems"),
    Polish = 4      UMETA(DisplayName = "Polish & Effects")
};

// Module loading states
UENUM(BlueprintType)
enum class EEng_ModuleState : uint8
{
    Unloaded = 0    UMETA(DisplayName = "Not Loaded"),
    Loading = 1     UMETA(DisplayName = "Loading"),
    Loaded = 2      UMETA(DisplayName = "Loaded"),
    Error = 3       UMETA(DisplayName = "Load Error"),
    Deprecated = 4  UMETA(DisplayName = "Deprecated")
};

// Architectural compliance result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ComplianceResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compliance")
    bool bIsCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Compliance")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Compliance")
    EEng_ValidationLevel ValidationLevel = EEng_ValidationLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Compliance")
    float ComplianceScore = 0.0f;

    FEng_ComplianceResult()
    {
        bIsCompliant = false;
        ErrorMessage = TEXT("Not validated");
        ValidationLevel = EEng_ValidationLevel::None;
        ComplianceScore = 0.0f;
    }
};

// System dependency definition
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    EEng_SystemPriority Priority = EEng_SystemPriority::Gameplay;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    TArray<FString> RequiredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    bool bIsOptional = false;

    FEng_SystemDependency()
    {
        SystemName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Gameplay;
        bIsOptional = false;
    }
};

/**
 * Core Architectural Framework Subsystem
 * Validates system compliance and enforces architectural rules
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ComplianceResult ValidateSystemCompliance(const FString& SystemName, EEng_ValidationLevel Level = EEng_ValidationLevel::Standard);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystemDependency(const FEng_SystemDependency& Dependency);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemLoadOrder();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ModuleState GetModuleState(const FString& ModuleName);

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceRequirements(const FString& SystemName, float MaxFrameTime = 16.67f);

    // Memory management validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateMemoryUsage(const FString& SystemName, int32 MaxMemoryMB = 512);

    // Thread safety validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateThreadSafety(const FString& SystemName);

protected:
    // Internal validation logic
    bool ValidateClassHierarchy(UClass* ClassToValidate);
    bool ValidateModuleDependencies(const FString& ModuleName);
    bool CheckCircularDependencies(const TArray<FEng_SystemDependency>& Dependencies);

private:
    // Registered system dependencies
    UPROPERTY()
    TArray<FEng_SystemDependency> RegisteredSystems;

    // Module state tracking
    UPROPERTY()
    TMap<FString, EEng_ModuleState> ModuleStates;

    // Performance metrics
    UPROPERTY()
    TMap<FString, float> SystemPerformanceMetrics;

    // Validation cache
    UPROPERTY()
    TMap<FString, FEng_ComplianceResult> ValidationCache;

    // Critical system list (cannot be disabled)
    TArray<FString> CriticalSystems;
};

/**
 * World-level architectural validator
 * Validates world-specific architectural compliance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldArchitectValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitectValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World validation functions
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateWorldConfiguration();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateActorHierarchy();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateComponentDependencies();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    TArray<FString> GetArchitecturalWarnings();

private:
    // Validation state
    bool bWorldValidated = false;
    TArray<FString> ValidationWarnings;
};

// Global architectural constants
namespace EngArchitecturalConstants
{
    // Performance limits
    constexpr float MAX_FRAME_TIME_MS = 16.67f;  // 60 FPS target
    constexpr int32 MAX_ACTORS_PER_LEVEL = 50000;
    constexpr int32 MAX_COMPONENTS_PER_ACTOR = 20;
    
    // Memory limits
    constexpr int32 MAX_SYSTEM_MEMORY_MB = 512;
    constexpr int32 MAX_TEXTURE_MEMORY_MB = 1024;
    constexpr int32 MAX_AUDIO_MEMORY_MB = 256;
    
    // System priorities (load order)
    const TArray<FString> CRITICAL_SYSTEMS = {
        TEXT("Core"),
        TEXT("Physics"), 
        TEXT("Rendering"),
        TEXT("Audio"),
        TEXT("Input")
    };
    
    const TArray<FString> CORE_SYSTEMS = {
        TEXT("WorldGeneration"),
        TEXT("CharacterSystem"),
        TEXT("GameMode"),
        TEXT("PlayerController")
    };
}