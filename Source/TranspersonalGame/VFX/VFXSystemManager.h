#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "VFXSystemManager.generated.h"

// Forward declarations
class UNiagaraSystem;
class UMaterialInterface;
class UParticleSystem;

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Dino_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Dino_Roar           UMETA(DisplayName = "Dinosaur Roar Wave"),
    Combat_Blood        UMETA(DisplayName = "Blood Splatter"),
    Combat_Impact       UMETA(DisplayName = "Weapon Impact"),
    Environment_Dust    UMETA(DisplayName = "Dust Cloud"),
    Environment_Water   UMETA(DisplayName = "Water Splash"),
    Environment_Volcanic UMETA(DisplayName = "Volcanic Ash"),
    Crafting_Sparks     UMETA(DisplayName = "Crafting Sparks"),
    Crafting_Smoke      UMETA(DisplayName = "Cooking Smoke")
};

UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UParticleSystem> LegacyParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UMaterialInterface> EffectMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LODDistance = 1000.0f;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
        LODDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_SpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = -1.0f; // -1 means use default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    AActor* AttachToActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocketName = NAME_None;

    FVFX_SpawnParams()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector(1.0f);
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = -1.0f;
        AttachToActor = nullptr;
        AttachSocketName = NAME_None;
    }
};

/**
 * VFX System Manager - Handles all visual effects for the prehistoric survival game
 * Manages realistic environmental effects: fire, weather, dust, water, and dinosaur-related VFX
 * NO mystical/spiritual effects - only physically plausible prehistoric world VFX
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, const FVFX_SpawnParams& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateCampfireEffect(FVector Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateDustCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateWaterSplash(FVector Location, float SplashSize = 1.0f);

    // Dinosaur VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateBreathEffect(AActor* DinosaurActor, bool bColdWeather = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateRoarWaveEffect(FVector Location, float RoarIntensity = 1.0f);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    UNiagaraComponent* CreateBloodSplatter(FVector Location, FVector Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    UNiagaraComponent* CreateWeaponImpact(FVector Location, EVFX_EffectType WeaponType);

    // Crafting VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    UNiagaraComponent* CreateCraftingSparks(FVector Location, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    UNiagaraComponent* CreateCookingSmoke(FVector Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    // LOD and Performance
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void UpdateVFXLOD(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    void LoadVFXConfiguration();

    UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    FVFX_EffectData GetEffectData(EVFX_EffectType EffectType) const;

protected:
    // VFX Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Data")
    TMap<EVFX_EffectType, FVFX_EffectData> VFXEffectDatabase;

    // Active Effects Tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX|Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float LODUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float MaxVFXDistance = 2000.0f;

    // Quality Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Quality")
    int32 VFXQualityLevel = 2; // 0=Low, 1=Medium, 2=High, 3=Ultra

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Quality")
    bool bEnableAdvancedLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Quality")
    bool bEnableVolumetricEffects = true;

private:
    // Internal helpers
    UNiagaraComponent* CreateNiagaraEffect(const FVFX_EffectData& EffectData, const FVFX_SpawnParams& SpawnParams);
    void CleanupInactiveEffects();
    void InitializeVFXDatabase();
    float CalculateLODScale(float Distance) const;

    // Timers
    float LODUpdateTimer = 0.0f;
    float CleanupTimer = 0.0f;
    const float CleanupInterval = 5.0f;
};