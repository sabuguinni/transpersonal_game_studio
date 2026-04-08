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
    Environment     UMETA(DisplayName = "Environment"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Atmosphere      UMETA(DisplayName = "Atmosphere"),
    UI              UMETA(DisplayName = "UI"),
    Destruction     UMETA(DisplayName = "Destruction")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    Background      UMETA(DisplayName = "Background")
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

    // Internal Methods
    void CleanupInactiveVFX();
    void UpdateAllVFXLOD();
    int32 GetVFXLODLevel(float Distance) const;
    bool ShouldCullVFX(UNiagaraComponent* VFXComponent) const;

private:
    float LastLODUpdateTime = 0.0f;
    class APlayerController* CachedPlayerController = nullptr;
};