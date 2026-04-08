#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    // Environmental Effects
    Fire                UMETA(DisplayName = "Fire"),
    Smoke               UMETA(DisplayName = "Smoke"),
    Steam               UMETA(DisplayName = "Steam"),
    Dust                UMETA(DisplayName = "Dust"),
    Rain                UMETA(DisplayName = "Rain"),
    Fog                 UMETA(DisplayName = "Fog"),
    
    // Combat Effects
    Blood               UMETA(DisplayName = "Blood"),
    Impact              UMETA(DisplayName = "Impact"),
    Sparks              UMETA(DisplayName = "Sparks"),
    Explosion           UMETA(DisplayName = "Explosion"),
    
    // Dinosaur Effects
    Breath              UMETA(DisplayName = "Breath"),
    Footstep            UMETA(DisplayName = "Footstep"),
    Roar                UMETA(DisplayName = "Roar"),
    
    // Mystical Effects
    GemGlow             UMETA(DisplayName = "Gem Glow"),
    Portal              UMETA(DisplayName = "Portal"),
    TimeDistortion      UMETA(DisplayName = "Time Distortion"),
    
    // Destruction Effects
    RockDebris          UMETA(DisplayName = "Rock Debris"),
    WoodSplinters       UMETA(DisplayName = "Wood Splinters"),
    Collapse            UMETA(DisplayName = "Collapse")
};

UENUM(BlueprintType)
enum class EVFXQuality : uint8
{
    Low                 UMETA(DisplayName = "Low Quality"),
    Medium              UMETA(DisplayName = "Medium Quality"),
    High                UMETA(DisplayName = "High Quality"),
    Ultra               UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXType VFXType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullingDistance = 5000.0f;

    FVFXDefinition()
    {
        VFXType = EVFXType::Fire;
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
        MaxInstances = 10;
        CullingDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FVFXInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* Component;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    EVFXType VFXType;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float Lifetime;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsActive;

    FVFXInstance()
    {
        Component = nullptr;
        VFXType = EVFXType::Fire;
        SpawnTime = 0.0f;
        Lifetime = 5.0f;
        bIsActive = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXSpawned, EVFXType, VFXType, UNiagaraComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXDestroyed, EVFXType, VFXType, UNiagaraComponent*, Component);

/**
 * VFX System Manager - Centralized management of all visual effects in the game
 * Handles Niagara system spawning, pooling, LOD management, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFXSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(EVFXType VFXType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFXType VFXType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator, const FVector& Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFXOfType(EVFXType VFXType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void DestroyAllVFX();

    // Quality and Performance
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(EVFXQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EVFXQuality GetVFXQuality() const { return CurrentQuality; }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetMaxVFXInstances(int32 MaxInstances);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCountOfType(EVFXType VFXType) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXDefinition(const FVFXDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool GetVFXDefinition(EVFXType VFXType, FVFXDefinition& OutDefinition) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "VFX")
    FOnVFXSpawned OnVFXSpawned;

    UPROPERTY(BlueprintAssignable, Category = "VFX")
    FOnVFXDestroyed OnVFXDestroyed;

protected:
    // VFX Definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TMap<EVFXType, FVFXDefinition> VFXDefinitions;

    // Active VFX Instances
    UPROPERTY()
    TArray<FVFXInstance> ActiveVFXInstances;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EVFXQuality CurrentQuality = EVFXQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxGlobalVFXInstances = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VFXCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float VFXUpdateInterval = 0.1f;

    // Timers
    FTimerHandle VFXCleanupTimer;
    FTimerHandle VFXCullingTimer;

private:
    void InitializeDefaultVFXDefinitions();
    void CleanupExpiredVFX();
    void UpdateVFXCulling();
    void ApplyQualitySettings();
    bool ShouldCullVFX(const FVFXInstance& Instance) const;
    UNiagaraComponent* CreateVFXComponent(const FVFXDefinition& Definition);
    void RegisterVFXInstance(UNiagaraComponent* Component, EVFXType VFXType, float Lifetime);
    void UnregisterVFXInstance(UNiagaraComponent* Component);
};