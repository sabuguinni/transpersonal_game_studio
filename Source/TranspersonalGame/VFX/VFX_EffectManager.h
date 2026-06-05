#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UMaterial;

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    Fire_Campfire = 1,
    Dino_Footstep = 2,
    Weather_Rain = 3,
    Combat_BloodImpact = 4,
    Environment_Dust = 5,
    Water_Splash = 6,
    Volcanic_Ash = 7,
    Insect_Swarm = 8
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UMaterial> EffectMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        DefaultScale = FVector(1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_SpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    AActor* AttachToActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FName AttachSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bAutoPlay = true;

    FVFX_SpawnParameters()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector(1.0f);
        AttachToActor = nullptr;
        AttachSocketName = NAME_None;
        bAutoPlay = true;
    }
};

/**
 * VFX Effect Manager - Handles all visual effects for prehistoric survival gameplay
 * Manages Niagara particle systems for fire, dust, weather, combat, and environmental effects
 * Provides centralized spawning, pooling, and lifecycle management for VFX
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EffectManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UVFX_EffectManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Effect spawning and management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, const FVFX_SpawnParameters& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAttached(EVFX_EffectType EffectType, AActor* AttachToActor, FName SocketName = NAME_None, FVector RelativeLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffectsOfType(EVFX_EffectType EffectType);

    // Prehistoric-specific effect spawners
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    UNiagaraComponent* SpawnCampfire(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* SpawnDinosaurFootstep(FVector Location, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    UNiagaraComponent* SpawnRainEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    UNiagaraComponent* SpawnBloodImpact(FVector Location, FVector ImpactDirection);

    // Effect data management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void LoadEffectData();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    FVFX_EffectData GetEffectData(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectTypeValid(EVFX_EffectType EffectType) const;

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupInactiveEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Effect data storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Data")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDataMap;

    // Active effect tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    float EffectCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX Performance")
    int32 VFXQualityLevel = 3;

private:
    // Internal helper methods
    UNiagaraComponent* CreateEffectComponent(const FVFX_EffectData& EffectData, const FVFX_SpawnParameters& SpawnParams);
    void RegisterActiveEffect(UNiagaraComponent* EffectComponent);
    void UnregisterActiveEffect(UNiagaraComponent* EffectComponent);
    void InitializeDefaultEffectData();

    // Cleanup timer
    FTimerHandle CleanupTimerHandle;
    void PerformCleanupTick();
};