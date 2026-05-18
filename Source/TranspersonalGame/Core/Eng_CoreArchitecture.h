#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

/**
 * Engine Architect Core Architecture System
 * Defines the fundamental architectural patterns and rules for the entire game
 * This system enforces consistency across all modules and agents
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArchitecturalViolation, const FString&, ViolationMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleRegistered, const FString&, ModuleName, bool, bSuccess);

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Gameplay        UMETA(DisplayName = "Gameplay Systems"), 
    Content         UMETA(DisplayName = "Content Systems"),
    Presentation    UMETA(DisplayName = "Presentation Layer"),
    Platform        UMETA(DisplayName = "Platform Layer")
};

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Physics         UMETA(DisplayName = "Physics Module"),
    WorldGen        UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI Systems"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    VFX             UMETA(DisplayName = "Visual Effects"),
    UI              UMETA(DisplayName = "User Interface"),
    Narrative       UMETA(DisplayName = "Narrative Systems")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float PerformanceWeight;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("");
        ModuleType = EEng_ModuleType::Physics;
        Layer = EEng_ArchitecturalLayer::Core;
        bIsActive = false;
        PerformanceWeight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PhysicsBodies;

    FEng_PerformanceMetrics()
    {
        FrameTime = 16.67f; // 60 FPS target
        CPUUsage = 0.0f;
        MemoryUsage = 0.0f;
        ActiveActors = 0;
        PhysicsBodies = 0;
    }
};

/**
 * Core Architecture Subsystem
 * Manages the overall architectural integrity of the game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CoreArchitectureSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforcePerformanceLimits();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ModuleInfo> GetRegisteredModules();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateArchitecturalCompliance();

    UPROPERTY(BlueprintAssignable, Category = "Architecture")
    FOnArchitecturalViolation OnArchitecturalViolation;

    UPROPERTY(BlueprintAssignable, Category = "Architecture")
    FOnModuleRegistered OnModuleRegistered;

protected:
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> RegisteredModules;

    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime = 33.33f; // 30 FPS minimum

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCPUUsage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage = 4096.0f; // 4GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors = 10000;

private:
    void UpdatePerformanceMetrics();
    void CheckArchitecturalViolations();
    bool ValidateLayerDependencies(EEng_ArchitecturalLayer Layer);
};

/**
 * Architectural Compliance Component
 * Attached to actors to ensure they follow architectural standards
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitecturalComplianceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalComplianceComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Compliance")
    void ValidateActorCompliance();

    UFUNCTION(BlueprintCallable, Category = "Compliance")
    bool CheckPerformanceCompliance();

    UFUNCTION(BlueprintCallable, Category = "Compliance")
    void ReportViolation(const FString& ViolationType, const FString& Description);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    EEng_ModuleType AssignedModule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    float PerformanceBudget = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    bool bEnforceStrictCompliance = true;

    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    TArray<FString> ViolationHistory;

private:
    void ValidateNamingConventions();
    void ValidateComponentStructure();
    void ValidatePerformanceImpact();
};