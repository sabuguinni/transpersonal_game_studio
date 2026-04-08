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
    Environment     UMETA(DisplayName = "Environment"),
    Survival        UMETA(DisplayName = "Survival"),
    Creatures       UMETA(DisplayName = "Creatures"),
    Atmosphere      UMETA(DisplayName = "Atmosphere"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Temporal        UMETA(DisplayName = "Temporal")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High           UMETA(DisplayName = "High"),
    Medium         UMETA(DisplayName = "Medium"),
    Low            UMETA(DisplayName = "Low"),
    Background     UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeTime = 10.0f;

    FVFXDefinition()
    {
        EffectName = TEXT("");
        Category = EVFXCategory::Environment;
        Priority = EVFXPriority::Medium;
    }
};

/**
 * VFX System Manager
 * Centralizes all visual effects management with LOD system and performance optimization
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

    // VFX Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<FString, FVFXDefinition> VFXRegistry;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 1000.0f; // High quality

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 2500.0f; // Medium quality

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 5000.0f; // Low quality

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TMap<EVFXPriority, int32> PriorityLimits;

public:
    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXByCategory(EVFXCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFX(const FString& EffectName, const FVFXDefinition& Definition);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVFXLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetLODLevel(float Distance) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectsCount() const { return ActiveEffects.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetVFXBudgetUsage() const;

private:
    void InitializePriorityLimits();
    void CleanupFinishedEffects();
    bool CanSpawnEffect(EVFXPriority Priority) const;
    void ForceCleanupLowPriorityEffects();
};