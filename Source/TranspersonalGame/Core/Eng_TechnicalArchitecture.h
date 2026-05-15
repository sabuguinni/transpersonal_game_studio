#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Eng_TechnicalArchitecture.generated.h"

/**
 * Engine Architect Technical Framework
 * Defines the core technical architecture rules and systems for the Transpersonal Game
 * This system enforces compilation rules, module dependencies, and performance standards
 */

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core = 0,
    WorldGeneration = 1,
    CharacterSystems = 2,
    AI_Behavior = 3,
    Combat = 4,
    Audio = 5,
    VFX = 6,
    UI = 7,
    Performance = 8,
    Integration = 9
};

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Critical = 0,    // Must maintain 60fps
    High = 1,        // Should maintain 45fps
    Medium = 2,      // Should maintain 30fps
    Low = 3          // Can drop to 20fps temporarily
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleType ModuleType;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceLevel PerformanceRequirement;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxActorsPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTimeMs;

    FEng_ModuleSpec()
    {
        ModuleName = TEXT("");
        ModuleType = EEng_ModuleType::Core;
        PerformanceRequirement = EEng_PerformanceLevel::Medium;
        MaxActorsPerFrame = 1000;
        MaxFrameTimeMs = 16.67f; // 60fps target
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bIsMandatory;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString ViolationMessage;

    FEng_CompilationRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bIsMandatory = true;
        ViolationMessage = TEXT("");
    }
};

/**
 * Technical Architecture Manager
 * Enforces technical standards and validates system compliance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterModule(const FEng_ModuleSpec& ModuleSpec);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetModuleLoadOrder();

    // Compilation Rules
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateCompilationRules();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    TArray<FEng_CompilationRule> GetFailedRules();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceCompliance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount();

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetArchitectureWarnings();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void RunArchitectureDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogModuleStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    TArray<FEng_ModuleSpec> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    TArray<FEng_CompilationRule> CompilationRules;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxWorldActors;

private:
    void InitializeCompilationRules();
    void InitializeDefaultModules();
    bool ValidateModuleSpec(const FEng_ModuleSpec& ModuleSpec);
    void UpdatePerformanceMetrics();

    // Performance tracking
    float LastFrameTime;
    int32 LastActorCount;
    TArray<FString> ArchitectureWarnings;
};

/**
 * Technical Standards Component
 * Can be attached to any actor to enforce technical compliance
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_TechnicalStandardsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_TechnicalStandardsComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateActorCompliance();

    UFUNCTION(BlueprintCallable, Category = "Standards")
    void EnforcePerformanceStandards();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceLevel RequiredPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsCompliant;

private:
    void CheckActorPerformance();
    void ValidateComponentSetup();

    float AccumulatedTickTime;
    int32 TickCount;
};