#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environment,    // Atmospheric, weather, ambient
    Creature,      // Dinosaur interactions, behaviors
    Combat,        // Damage, impacts, destruction
    Survival,      // Crafting, building, resource gathering
    Narrative      // Story moments, discoveries
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical,      // Always render, gameplay essential
    High,          // Important for immersion
    Medium,        // Nice to have, can be culled
    Low            // First to be removed under performance pressure
};

USTRUCT(BlueprintType)
struct FVFXSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDrawDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLOD = true;

    FVFXSystemData()
    {
        Category = EVFXCategory::Environment;
        Priority = EVFXPriority::Medium;
    }
};

/**
 * VFX System Manager
 * Manages all Niagara VFX systems with performance-aware LOD and culling
 * Implements 3-tier LOD system for optimal performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX System Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<FString, FVFXSystemData> RegisteredSystems;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudgetMS = 2.0f; // Max 2ms per frame for VFX

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceHigh = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMedium = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceLow = 5000.0f;

private:
    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance monitoring
    float CurrentFrameVFXTime;
    int32 CurrentActiveCount;

public:
    // System Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXSystem(const FString& SystemName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXSystem(const FString& SystemName, const FVFXSystemData& SystemData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetPerformanceMode(bool bHighPerformance);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateLODForAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetLODLevelForDistance(float Distance) const;

    // Performance Monitoring
    UFUNCTION(BlueprintPure, Category = "VFX")
    float GetCurrentVFXBudgetUsage() const { return CurrentFrameVFXTime; }

    UFUNCTION(BlueprintPure, Category = "VFX")
    int32 GetActiveEffectCount() const { return CurrentActiveCount; }

private:
    void CullLowPriorityEffects();
    void UpdatePerformanceMetrics();
    APawn* GetLocalPlayerPawn() const;
};