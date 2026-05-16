#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFXSystemManager.generated.h"

// VFX Effect Types for prehistoric world
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    DustImpact      UMETA(DisplayName = "Dust Impact"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    BloodSpatter    UMETA(DisplayName = "Blood Spatter"),
    FireSparks      UMETA(DisplayName = "Fire Sparks"),
    BreathSteam     UMETA(DisplayName = "Breath Steam"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    WeaponImpact    UMETA(DisplayName = "Weapon Impact")
};

// VFX Intensity levels based on creature size and impact force
UENUM(BlueprintType)
enum class EVFX_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

// VFX Effect Configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = EVFX_IntensityLevel::Medium;
        Duration = 2.0f;
        Scale = 1.0f;
        SpawnOffset = FVector::ZeroVector;
        bAutoDestroy = true;
    }
};

// Biome-specific VFX settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float DustIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float WaterAvailability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    FLinearColor AmbientParticleColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float WindStrength = 1.0f;

    FVFX_BiomeSettings()
    {
        BiomeType = EBiomeType::Savana;
        DustIntensityMultiplier = 1.0f;
        WaterAvailability = 0.5f;
        AmbientParticleColor = FLinearColor::White;
        WindStrength = 1.0f;
    }
};

/**
 * VFX System Manager - Controls all visual effects in the prehistoric world
 * Handles dinosaur impacts, environmental effects, and biome-specific particles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UParticleSystemComponent* WaterParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UParticleSystemComponent* BloodParticleComponent;

    // VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EffectConfig> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_BiomeSettings> BiomeVFXSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 20;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float CurrentWindStrength = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    int32 ActiveEffectCount = 0;

public:
    // VFX Trigger Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerDustImpact(const FVector& Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerWaterSplash(const FVector& Location, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerBloodSpatter(const FVector& Location, const FVector& Direction, EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerWeaponImpact(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerBreathSteam(const FVector& Location, const FVector& Direction);

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    void UpdateBiomeEffects(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "VFX Biome")
    FVFX_BiomeSettings GetBiomeVFXSettings(EBiomeType BiomeType) const;

    // Effect Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void ClearAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetMaxActiveEffects(int32 NewMax);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const { return ActiveEffectCount; }

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "VFX Events")
    void OnBiomeVFXChanged(EBiomeType NewBiome);

    UFUNCTION(BlueprintImplementableEvent, Category = "VFX Events")
    void OnEffectTriggered(EVFX_EffectType EffectType, const FVector& Location);

private:
    // Internal VFX management
    void InitializeBiomeSettings();
    void UpdateActiveEffects(float DeltaTime);
    void CleanupExpiredEffects();
    bool CanSpawnNewEffect() const;
    
    FTimerHandle EffectUpdateTimer;
    TArray<AActor*> SpawnedEffectActors;
};