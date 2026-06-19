// VFXManager.h
// Agent #17 — VFX Agent | PROD_CYCLE_AUTO_20260619_007
// Manages all Niagara particle VFX for the prehistoric survival game.
// Handles campfire, footstep dust, rain, blood impact, river splash, volcanic ash.
// NO spiritual/mystical effects — all VFX are physically plausible prehistoric world effects.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFXManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — all prefixed EVFX_ to avoid collision with other agents
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    PlayerCombat    UMETA(DisplayName = "Player & Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    World           UMETA(DisplayName = "World")
};

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Overcast        UMETA(DisplayName = "Overcast"),
    Rain            UMETA(DisplayName = "Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Ash             UMETA(DisplayName = "Volcanic Ash")
};

UENUM(BlueprintType)
enum class EVFX_ImpactSurface : uint8
{
    Dirt            UMETA(DisplayName = "Dirt"),
    Rock            UMETA(DisplayName = "Rock"),
    Grass           UMETA(DisplayName = "Grass"),
    Water           UMETA(DisplayName = "Water"),
    Sand            UMETA(DisplayName = "Sand")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — all prefixed FVFX_ to avoid collision
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FVFX_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustRadius = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustLifetime = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ParticleCount = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    EVFX_ImpactSurface Surface = EVFX_ImpactSurface::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ScreenShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ScreenShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FVFX_CampfireConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameHeight = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeRiseSpeed = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberCount = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightIntensity = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bWindAffected = true;
};

USTRUCT(BlueprintType)
struct FVFX_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    EVFX_WeatherState CurrentState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainDensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainAngle = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float TransitionDuration = 10.0f;
};

USTRUCT(BlueprintType)
struct FVFX_BloodImpactConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    float SprayRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    float DropletCount = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    float DecalSize = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    float DecalLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood")
    EVFX_ImpactSurface Surface = EVFX_ImpactSurface::Dirt;
};

// ─────────────────────────────────────────────────────────────────────────────
// UVFX_ManagerComponent — Component that manages VFX spawning for an actor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "VFX", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ManagerComponent();

    // Footstep VFX — called by animation notify or character movement
    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnFootstepDust(FVector ImpactLocation, EVFX_ImpactSurface Surface, float MassScale = 1.0f);

    // Campfire VFX — called when campfire is ignited/extinguished
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireEffect(FVector Location, bool bIgnite);

    // Blood impact — called on damage application
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector ImpactLocation, FVector ImpactNormal, float DamageAmount);

    // River splash — called when actor enters water volume
    UFUNCTION(BlueprintCallable, Category = "VFX|Water")
    void SpawnWaterSplash(FVector Location, float Radius, float Speed);

    // Weather transition — called by weather system
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState, float TransitionTime = 10.0f);

    // Volcanic ash drift — called by world manager when volcano is active
    UFUNCTION(BlueprintCallable, Category = "VFX|World")
    void SetVolcanicAshActive(bool bActive, float Intensity = 1.0f);

    // TRex screen shake — called from AudioZoneManager when DangerLevel == Critical
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void TriggerTRexFootstepShake(FVector FootstepLocation, float PlayerDistance);

    // Damage flash overlay — red vignette when Health < 20%
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SetDamageFlashIntensity(float HealthPercent);

    // Breath vapor — visible breath in cold environments
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetBreathVaporActive(bool bActive, float Temperature);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FVFX_FootstepConfig FootstepConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FVFX_CampfireConfig CampfireConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FVFX_WeatherConfig WeatherConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FVFX_BloodImpactConfig BloodConfig;

    // Niagara system references — assigned in Blueprint or via data asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_FootstepDirt = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_FootstepRock = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_CampfireFire = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_CampfireSmoke = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_BloodImpact = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_WaterSplash = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_Rain = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_VolcanicAsh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_BreathVapor = nullptr;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    UNiagaraComponent* ActiveRainComponent = nullptr;

    UPROPERTY()
    UNiagaraComponent* ActiveAshComponent = nullptr;

    UPROPERTY()
    UNiagaraComponent* ActiveBreathComponent = nullptr;

    EVFX_WeatherState CurrentWeatherState = EVFX_WeatherState::Clear;
    float DamageFlashAlpha = 0.0f;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 10.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AVFX_CampfireActor — Placeable campfire actor with fire + light + sound anchor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_CampfireActor : public AActor
{
    GENERATED_BODY()

public:
    AVFX_CampfireActor();

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void IgniteCampfire();

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishCampfire();

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    bool IsLit() const { return bIsLit; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVFX_CampfireConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    UNiagaraSystem* FireSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    UNiagaraSystem* SmokeSystem = nullptr;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* LogsMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* FireComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* SmokeComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UPointLightComponent* FireLight = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere = nullptr;

private:
    bool bIsLit = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AVFX_WeatherController — World-level weather VFX controller
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void TransitionToWeather(EVFX_WeatherState NewState, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWindDirection(FVector Direction, float Strength);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_WeatherConfig WeatherConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_Rain = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_Storm = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    UNiagaraSystem* NS_VolcanicAsh = nullptr;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    EVFX_WeatherState CurrentWeather = EVFX_WeatherState::Clear;
    EVFX_WeatherState TargetWeather = EVFX_WeatherState::Clear;
    float TransitionAlpha = 1.0f;
    float TransitionDuration = 10.0f;
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);
    float WindStrength = 1.0f;

    UPROPERTY()
    UNiagaraComponent* ActiveWeatherComponent = nullptr;
};
