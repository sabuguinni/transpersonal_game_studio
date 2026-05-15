#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

// VFX Categories for prehistoric environments
UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    DinosaurImpact  UMETA(DisplayName = "Dinosaur Impact"),
    WeatherEffect   UMETA(DisplayName = "Weather Effect"),
    CombatEffect    UMETA(DisplayName = "Combat Effect"),
    AmbientEffect   UMETA(DisplayName = "Ambient Effect")
};

// VFX intensity levels for performance scaling
UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Ultra   UMETA(DisplayName = "Ultra")
};

// Dinosaur-specific VFX profile for footsteps and impacts
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_DinosaurProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    float FootstepIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    float DustCloudSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    FLinearColor ParticleColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    bool bCreateGroundCrack = false;

    FVFX_DinosaurProfile()
    {
        DinosaurName = TEXT("DefaultDinosaur");
        FootstepIntensity = 1.0f;
        DustCloudSize = 100.0f;
        ParticleColor = FLinearColor::Brown;
        bCreateGroundCrack = false;
    }
};

// Environment-specific VFX settings for each biome
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment VFX")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment VFX")
    float AmbientParticleDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment VFX")
    FLinearColor AmbientParticleColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment VFX")
    float WindIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment VFX")
    bool bHasVolcanicAsh = false;

    FVFX_EnvironmentSettings()
    {
        BiomeType = EBiomeType::Savanna;
        AmbientParticleDensity = 0.5f;
        AmbientParticleColor = FLinearColor::White;
        WindIntensity = 1.0f;
        bHasVolcanicAsh = false;
    }
};

/**
 * VFX Niagara Manager - Handles all visual effects for prehistoric environments
 * Manages dinosaur footsteps, environmental particles, weather effects, and combat VFX
 * Integrates with Audio system for synchronized audio-visual effects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === DINOSAUR IMPACT VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnDinosaurFootstep(const FVector& Location, const FString& DinosaurType, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnDinosaurRoarEffect(const FVector& Location, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "VFX Dinosaur")
    void SpawnDinosaurBreathEffect(const FVector& Location, const FRotator& Direction, const FString& DinosaurType);

    // === ENVIRONMENTAL VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SetEnvironmentVFX(EBiomeType BiomeType, EVFX_IntensityLevel Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnCampfireEffect(const FVector& Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnWaterfallSpray(const FVector& Location, const FVector& Direction, float WaterVolume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void SpawnVolcanicAsh(const FVector& Location, float AshDensity = 1.0f);

    // === WEATHER VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StartRainEffect(float RainIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void StartWindEffect(const FVector& WindDirection, float WindStrength = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Weather")
    void SpawnLightningStrike(const FVector& Location);

    // === COMBAT VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnWeaponImpact(const FVector& Location, const FString& WeaponType, const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnBloodEffect(const FVector& Location, const FVector& Direction, float BloodAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void SpawnSpearThrow(const FVector& StartLocation, const FVector& EndLocation);

    // === CRAFTING VFX ===
    UFUNCTION(BlueprintCallable, Category = "VFX Crafting")
    void SpawnStoneKnapping(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Crafting")
    void SpawnFireMaking(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Crafting")
    void SpawnCookingSmoke(const FVector& Location, float CookingIntensity = 1.0f);

    // === VFX MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetGlobalVFXIntensity(EVFX_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void EnableVFXCategory(EVFX_EffectCategory Category, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

protected:
    // === NIAGARA SYSTEM REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> FootstepEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> CampfireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> RainEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> DustCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TSoftObjectPtr<UNiagaraSystem> SparksEffect;

    // === VFX CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_IntensityLevel GlobalIntensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<FString, FVFX_DinosaurProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EBiomeType, FVFX_EnvironmentSettings> EnvironmentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFX_EffectCategory, bool> EnabledCategories;

    // === ACTIVE EFFECTS TRACKING ===
    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float EffectCleanupInterval = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    float LastCleanupTime = 0.0f;

private:
    // === HELPER METHODS ===
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    void InitializeDinosaurProfiles();
    void InitializeEnvironmentSettings();
    void InitializeEnabledCategories();
    bool IsEffectCategoryEnabled(EVFX_EffectCategory Category) const;
    float GetIntensityMultiplier() const;
};