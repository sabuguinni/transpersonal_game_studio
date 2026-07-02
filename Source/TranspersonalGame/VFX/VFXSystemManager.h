#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// ENUMS — VFX categories for prehistoric survival game
// ============================================================

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    None            UMETA(DisplayName = "None"),
    Environment     UMETA(DisplayName = "Environment"),    // campfire, rain, fog, ash
    Dinosaur        UMETA(DisplayName = "Dinosaur"),       // footstep dust, breath vapor, blood
    Combat          UMETA(DisplayName = "Combat"),         // weapon impact, wound, sparks
    World           UMETA(DisplayName = "World"),          // god rays, volcanic ash, waterfall
    Weather         UMETA(DisplayName = "Weather")         // rain, snow, storm, wind
};

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Overcast        UMETA(DisplayName = "Overcast"),
    Rain            UMETA(DisplayName = "Rain"),
    HeavyStorm      UMETA(DisplayName = "HeavyStorm"),
    VolcanicAsh     UMETA(DisplayName = "VolcanicAsh"),
    Fog             UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EVFX_ImpactSurface : uint8
{
    Dirt            UMETA(DisplayName = "Dirt"),
    Rock            UMETA(DisplayName = "Rock"),
    Grass           UMETA(DisplayName = "Grass"),
    Water           UMETA(DisplayName = "Water"),
    Mud             UMETA(DisplayName = "Mud"),
    Sand            UMETA(DisplayName = "Sand")
};

// ============================================================
// STRUCTS — VFX configuration data
// ============================================================

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName EffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category = EVFX_EffectCategory::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifetimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;
};

USTRUCT(BlueprintType)
struct FVFX_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    EVFX_ImpactSurface SurfaceType = EVFX_ImpactSurface::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float FootprintRadius = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float GroundShakeRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    bool bIsHeavyCreature = false;
};

USTRUCT(BlueprintType)
struct FVFX_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    EVFX_WeatherState CurrentState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float AshDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float TransitionDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FVFX_CombatImpactConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    FVector ImpactNormal = FVector(0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    EVFX_ImpactSurface Surface = EVFX_ImpactSurface::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    float DamageAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    bool bIsBloodImpact = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    bool bIsBoneImpact = false;
};

// ============================================================
// UVFX_SystemManager — Main VFX component
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent), DisplayName = "VFX System Manager")
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Environment VFX ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnCampfireEffect(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWaterfallSpray(FVector Location, float WaterVolume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnVolcanicAshEmitter(FVector Location, float AshDensity = 1.0f);

    // ---- Dinosaur VFX ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDust(FVector FootLocation, FVFX_FootstepConfig Config);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnBreathVapor(FVector MouthLocation, FRotator Direction, bool bIsColdEnvironment = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnRoarDistortion(FVector RoarOrigin, float RoarIntensity = 1.0f);

    // ---- Combat VFX ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnImpactEffect(FVFX_CombatImpactConfig Config);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodSplatter(FVector Location, FVector Direction, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnCraftingSparks(FVector Location, EVFX_ImpactSurface ToolSurface);

    // ---- Weather VFX ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(FVFX_WeatherConfig NewWeather);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void TransitionWeather(EVFX_WeatherState TargetState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeatherState() const { return CurrentWeather.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    float GetRainIntensity() const { return CurrentWeather.RainIntensity; }

    // ---- Screen VFX ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
    void TriggerDamageFlash(float DamageAmount, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
    void TriggerExhaustionVignette(float StaminaPercent);

    UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
    void TriggerHungerDesaturation(float HungerPercent);

    // ---- Niagara Asset References (assign in Blueprint) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BreathVapor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_RoarDistortion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Rain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_VolcanicAsh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_DirtImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Environment_WaterfallSpray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Crafting_Sparks;

    // ---- State ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State")
    FVFX_WeatherConfig CurrentWeather;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State")
    bool bIsRaining = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State")
    float DamageFlashTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|State")
    float ExhaustionVignetteIntensity = 0.0f;

private:
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateScreenEffects(float DeltaTime);
    UNiagaraSystem* LoadNiagaraAsset(TSoftObjectPtr<UNiagaraSystem>& SoftRef);

    EVFX_WeatherState TargetWeatherState = EVFX_WeatherState::Clear;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bTransitioningWeather = false;
};
