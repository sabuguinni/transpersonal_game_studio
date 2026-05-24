#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "EngineArchitectCore.generated.h"

/**
 * Engine Architect Core System - Technical Architecture Foundation
 * Defines the core technical rules and systems that all other agents must follow
 * Implements John Carmack's principle: "Elegant architecture is necessity, not aesthetic"
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Engine Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Metrics")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Metrics")
    int32 ActiveComponents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Metrics")
    bool bPerformanceTarget = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bMandatory = true;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 Priority = 1;
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EEng_ValidationStatus : uint8
{
    Passed      UMETA(DisplayName = "Passed"),
    Warning     UMETA(DisplayName = "Warning"),
    Failed      UMETA(DisplayName = "Failed"),
    Pending     UMETA(DisplayName = "Pending")
};

/**
 * Engine Architect Core Component - Attached to critical game systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitectComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FEng_SystemMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FEng_ArchitectureRule> ActiveRules;

    UPROPERTY(BlueprintReadWrite, Category = "Engine Architecture")
    EEng_SystemPriority SystemPriority = EEng_SystemPriority::Medium;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckArchitectureCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemRule(const FString& RuleName, const FString& Description, bool bMandatory = true);

private:
    void UpdateMetrics();
    void EnforcePerformanceTargets();
};

/**
 * Engine Architect World Subsystem - Global architecture enforcement
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_ArchitectWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Global Architecture")
    TArray<FEng_ArchitectureRule> GlobalRules;

    UPROPERTY(BlueprintReadOnly, Category = "Global Architecture")
    FEng_SystemMetrics GlobalMetrics;

    UFUNCTION(BlueprintCallable, Category = "Global Architecture")
    void EnforceGlobalRules();

    UFUNCTION(BlueprintCallable, Category = "Global Architecture")
    EEng_ValidationStatus ValidateWorldArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Global Architecture")
    void RegisterCriticalSystem(AActor* SystemActor, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Global Architecture")
    TArray<AActor*> GetRegisteredSystems() const;

private:
    UPROPERTY()
    TArray<AActor*> RegisteredSystems;

    void InitializeArchitectureRules();
    void MonitorSystemPerformance();
};

/**
 * Engine Architect Manager Actor - Central architecture coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_ArchitectManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_ArchitectManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UEng_ArchitectComponent* ArchitectComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture Management")
    bool bEnforcePerformanceTargets = true;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture Management")
    float TargetFrameRate = 60.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture Management")
    float MaxMemoryUsageMB = 4096.0f;

    UFUNCTION(BlueprintCallable, Category = "Architecture Management")
    void InitializeEngineArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture Management")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture Management")
    void EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture Management")
    void RunArchitectureAudit();

private:
    void SetupCoreRules();
    void MonitorSystemHealth();
    bool ValidateModuleDependencies();
};