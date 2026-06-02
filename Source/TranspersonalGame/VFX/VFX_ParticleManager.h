#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

// VFX effect types for prehistoric survival gameplay
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    CampfireFire,
    CampfireSmoke,
    FootstepDust,
    BloodImpact,
    WaterSplash,
    CookingSmoke,
    RainDroplets,
    WindParticles,
    CraftingSparks,
    BreathVapor
};

// VFX intensity levels
UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low = 0,
    Medium,
    High,
    Extreme
};

// VFX effect configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = EVFX_IntensityLevel::Medium;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Duration = 5.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
    }
};

// VFX particle data for tracking active effects
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    int32 EffectID = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    class UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    float StartTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    float Duration = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Data")
    bool bIsActive = false;

    FVFX_ParticleData()
    {
        EffectID = 0;
        EffectType = EVFX_EffectType::None;
        NiagaraComponent = nullptr;
        StartTime = 0.0f;
        Duration = 0.0f;
        bIsActive = false;
    }
};

/**
 * VFX Particle Manager - Handles all Niagara particle effects for prehistoric survival gameplay
 * Manages campfires, footstep dust, blood impacts, water splashes, and environmental effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class USceneComponent* RootSceneComponent;

    // Active particle effects tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Tracking")
    TArray<FVFX_ParticleData> ActiveEffects;

    // Effect ID counter
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Tracking")
    int32 NextEffectID = 1;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullDistance = 5000.0f;

public:
    // Primary VFX creation functions
    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateCampfireEffect(const FVector& Location, float Scale = 1.0f, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateFootstepDustEffect(const FVector& Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateBloodImpactEffect(const FVector& Location, const FVector& ImpactDirection, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateWaterSplashEffect(const FVector& Location, float SplashRadius = 100.0f, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateCookingSmokeEffect(const FVector& Location, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateCraftingSparksEffect(const FVector& Location, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateBreathVaporEffect(const FVector& Location, const FRotator& Direction);

    // Generic effect creation
    UFUNCTION(BlueprintCallable, Category = "VFX Effects")
    int32 CreateEffect(const FVFX_EffectConfig& Config);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    bool StopEffect(int32 EffectID);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void StopEffectsByType(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    bool IsEffectActive(int32 EffectID) const;

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    TArray<int32> GetActiveEffectIDs() const;

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void CullDistantEffects(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetMaxActiveEffects(int32 NewMaxEffects);

    UFUNCTION(BlueprintCallable, Category = "VFX Performance")
    void SetEffectCullDistance(float NewCullDistance);

protected:
    // Internal helper functions
    UNiagaraComponent* CreateNiagaraComponent(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation, float Scale);
    void ConfigureEffectIntensity(UNiagaraComponent* Component, EVFX_EffectType EffectType, EVFX_IntensityLevel Intensity);
    void CleanupExpiredEffects();
    void RemoveOldestEffect();
    FVFX_ParticleData* FindEffectData(int32 EffectID);
    const FVFX_ParticleData* FindEffectData(int32 EffectID) const;
}