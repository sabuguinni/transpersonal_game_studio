#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Combat          UMETA(DisplayName = "Combat"),
    Creature        UMETA(DisplayName = "Creature"),
    Environment     UMETA(DisplayName = "Environment"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Survival        UMETA(DisplayName = "Survival"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical"),
    High = 1        UMETA(DisplayName = "High"),
    Medium = 2      UMETA(DisplayName = "Medium"),
    Low = 3         UMETA(DisplayName = "Low"),
    Background = 4  UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct FVFXPoolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category = EVFXCategory::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority = EVFXPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRecycle = true;

    FVFXPoolData()
    {
        PoolSize = 10;
        Category = EVFXCategory::Environment;
        Priority = EVFXPriority::Medium;
        MaxDistance = 5000.0f;
        bAutoRecycle = true;
    }
};

/**
 * VFX System Manager - Centralized VFX management with LOD chain and performance optimization
 * Implements 3-tier LOD system: High (0-1000m), Medium (1000-3000m), Low (3000m+)
 * 
 * CONSOLIDATION NOTE: This is the canonical VFXSystemManager - replaces duplicate in Core/
 */
UCLASS()
class TRANSPERSONALGAME_API UVFXSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // VFX Spawning
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAtLocation(
        UNiagaraSystem* System,
        const FVector& Location,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVector& Scale = FVector::OneVector,
        bool bAutoDestroy = true,
        EVFXPriority Priority = EVFXPriority::Medium
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAttached(
        UNiagaraSystem* System,
        USceneComponent* AttachToComponent,
        FName AttachPointName = NAME_None,
        const FVector& Location = FVector::ZeroVector,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVector& Scale = FVector::OneVector,
        bool bAutoDestroy = true,
        EVFXPriority Priority = EVFXPriority::Medium
    );

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetMaxActiveVFX(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PauseAllVFXOfCategory(EVFXCategory Category);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ResumeAllVFXOfCategory(EVFXCategory Category);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateVFXLOD(UNiagaraComponent* VFXComponent, float DistanceToPlayer);

    // Pool Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeVFXPool(const FString& PoolName, const FVFXPoolData& PoolData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* GetPooledVFX(const FString& PoolName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ReturnPooledVFX(const FString& PoolName, UNiagaraComponent* VFXComponent);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const { return ActiveVFXComponents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    float GetVFXBudgetUsage() const;

protected:
    // VFX Tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveVFXComponents;

    UPROPERTY()
    TMap<FString, TArray<UNiagaraComponent*>> VFXPools;

    UPROPERTY()
    TMap<FString, FVFXPoolData> PoolConfigurations;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFX = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentVFXQuality = 2; // 0=Low, 1=Medium, 2=High

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VFXBudgetMS = 2.0f; // Maximum 2ms per frame for VFX

    // Internal Methods
    void CleanupInactiveVFX();
    void UpdateAllVFXLOD();
    int32 GetVFXLODLevel(float Distance) const;
    bool ShouldCullVFX(UNiagaraComponent* VFXComponent) const;
    bool CanSpawnNewVFX(EVFXPriority Priority) const;
    void AdaptiveQualityAdjustment();

private:
    float LastLODUpdateTime = 0.0f;
    class APlayerController* CachedPlayerController = nullptr;
    float CurrentFrameVFXTime = 0.0f;
};