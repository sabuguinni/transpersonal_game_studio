#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ArchitectureCore.generated.h"

/**
 * Core Architecture System for Transpersonal Game Studio
 * Defines the fundamental architectural patterns and rules that all systems must follow
 * 
 * ARCHITECTURAL PRINCIPLES:
 * 1. Modular Design - Each system is self-contained with clear interfaces
 * 2. Performance First - 60fps PC / 30fps Console mandatory
 * 3. Scalability - Support for 50,000+ concurrent entities
 * 4. World Partition - Mandatory for worlds > 4km²
 * 5. Data-Driven - All gameplay parameters externalized
 */

UENUM(BlueprintType)
enum class ESystemPriority : uint8
{
    Critical = 0,    // Physics, Rendering, Core
    High = 1,        // AI, Animation, Audio
    Medium = 2,      // VFX, UI, Narrative
    Low = 3,         // Analytics, Debug, Tools
    Background = 4   // Streaming, Cleanup, Optimization
};

UENUM(BlueprintType)
enum class EArchitectureLayer : uint8
{
    Engine = 0,      // UE5 Core Systems
    Framework = 1,   // Our Core Architecture
    Gameplay = 2,    // Game-specific Systems
    Content = 3,     // Assets and Data
    UI = 4          // User Interface Layer
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArchitectureLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerformanceBudget; // CPU time in ms per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Dependencies;

    FSystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = ESystemPriority::Medium;
        Layer = EArchitectureLayer::Gameplay;
        bIsActive = true;
        PerformanceBudget = 1.0f;
    }
};

/**
 * Architecture Core Subsystem
 * Manages system registration, dependency resolution, and performance monitoring
 */
UCLASS()
class TRANSPERSONALGAME_API UArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FSystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FSystemInfo GetSystemInfo(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FSystemInfo> GetAllSystems() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetPerformanceBudget(const FString& SystemName, float BudgetMs);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetSystemPerformance(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsPerformanceWithinBudget(const FString& SystemName) const;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecture() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitectureViolations() const;

    // World Partition Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsWorldPartitionRequired() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetWorldSize(float SizeKm);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetWorldSize() const { return WorldSizeKm; }

protected:
    UPROPERTY()
    TMap<FString, FSystemInfo> RegisteredSystems;

    UPROPERTY()
    TMap<FString, float> SystemPerformanceData;

    UPROPERTY()
    float WorldSizeKm;

    // Performance Constants
    static constexpr float TARGET_FRAMETIME_PC = 16.67f; // 60fps = 16.67ms
    static constexpr float TARGET_FRAMETIME_CONSOLE = 33.33f; // 30fps = 33.33ms
    static constexpr float WORLD_PARTITION_THRESHOLD = 4.0f; // 4km²

private:
    void InitializeCoreSystems();
    bool ValidateSystemDependencies(const FString& SystemName) const;
    void UpdatePerformanceMetrics();
};

/**
 * Architecture Validation Component
 * Can be added to actors to validate they follow architectural guidelines
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitectureValidationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitectureValidationComponent();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateActor() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetValidationErrors() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnforcePerformanceLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxRenderComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxTriangleCount;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool ValidateRenderComplexity() const;
    bool ValidateTriangleCount() const;
    bool ValidateComponentStructure() const;
};