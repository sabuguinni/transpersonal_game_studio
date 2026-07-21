#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VFX_NiagaraSystemsConfig.generated.h"

/**
 * VFX_NiagaraSystemsConfig
 * Configuration data for all Niagara particle systems in the prehistoric survival game.
 * Covers: fire/smoke, weather, dinosaur impacts, combat, environment.
 * All effects are physically plausible in a real prehistoric world.
 */

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    DinosaurImpact  UMETA(DisplayName = "Dinosaur Impact"),
    CombatPlayer    UMETA(DisplayName = "Combat Player"),
    Weather         UMETA(DisplayName = "Weather"),
    WorldAmbient    UMETA(DisplayName = "World Ambient"),
};

UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High    UMETA(DisplayName = "High (< 20m)"),
    Medium  UMETA(DisplayName = "Medium (20-60m)"),
    Low     UMETA(DisplayName = "Low (60-150m)"),
    Culled  UMETA(DisplayName = "Culled (> 150m)"),
};

USTRUCT(BlueprintType)
struct FVFX_NiagaraEntry
{
    GENERATED_BODY()

    /** Unique system name used for asset lookup: NS_[Category]_[Effect] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName SystemName;

    /** Content browser path to the Niagara System asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FSoftObjectPath AssetPath;

    /** Effect category for budget grouping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category = EVFX_EffectCategory::Environment;

    /** Max simultaneous active instances of this effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxInstances = 8;

    /** Distance at which effect is fully culled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullDistanceMeters = 150.0f;

    /** Whether this effect loops (campfire=true, impact=false) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    FVFX_NiagaraEntry()
        : SystemName(NAME_None)
        , MaxInstances(8)
        , CullDistanceMeters(150.0f)
        , bLooping(false)
    {}
};

USTRUCT(BlueprintType)
struct FVFX_WeatherState
{
    GENERATED_BODY()

    /** 0.0 = clear, 1.0 = heavy storm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainIntensity = 0.0f;

    /** 0.0 = calm, 1.0 = gale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindStrength = 0.0f;

    /** Fog density multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FogDensity = 1.0f;

    /** Is volcanic ash falling? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    bool bVolcanicAsh = false;

    FVFX_WeatherState()
        : RainIntensity(0.0f)
        , WindStrength(0.2f)
        , FogDensity(1.0f)
        , bVolcanicAsh(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraSystemsConfig : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraSystemsConfig();

    /** All registered Niagara effect entries */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Registry")
    TArray<FVFX_NiagaraEntry> RegisteredEffects;

    /** Current weather state driving weather VFX */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_WeatherState CurrentWeather;

    /** Global VFX budget: max total active Niagara components */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 GlobalMaxActiveEffects = 64;

    /** Get entry by system name */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool GetEffectEntry(FName SystemName, FVFX_NiagaraEntry& OutEntry) const;

    /** Register a new effect at runtime */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffect(const FVFX_NiagaraEntry& Entry);

    /** Get all effects in a category */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<FVFX_NiagaraEntry> GetEffectsByCategory(EVFX_EffectCategory Category) const;

    /** Update weather state (drives rain/fog/ash VFX) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(const FVFX_WeatherState& NewWeather);

private:
    void InitDefaultEffects();
};
