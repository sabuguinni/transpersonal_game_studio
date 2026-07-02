#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// VFX SYSTEM MANAGER — Agent #17 (VFX Agent)
// Prehistoric survival game — ALL effects are physically real.
// No spiritual/mystical/energy VFX. Only what existed 65Ma ago.
// ============================================================

// --- Enums (global scope, VFX_ prefix) ---

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    HeavyStorm  UMETA(DisplayName = "Heavy Storm"),
    Ash         UMETA(DisplayName = "Volcanic Ash Fall"),
    Snow        UMETA(DisplayName = "Snow"),
};

UENUM(BlueprintType)
enum class EVFX_ImpactSurface : uint8
{
    Dirt    UMETA(DisplayName = "Dirt"),
    Mud     UMETA(DisplayName = "Mud"),
    Rock    UMETA(DisplayName = "Rock"),
    Water   UMETA(DisplayName = "Water"),
    Grass   UMETA(DisplayName = "Grass"),
    Sand    UMETA(DisplayName = "Sand"),
};

UENUM(BlueprintType)
enum class EVFX_CombatImpact : uint8
{
    SpearHit    UMETA(DisplayName = "Spear Hit"),
    StoneHit    UMETA(DisplayName = "Stone Hit"),
    ClawSlash   UMETA(DisplayName = "Claw Slash"),
    BiteCrush   UMETA(DisplayName = "Bite Crush"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
};

// --- Structs (global scope, FVFX_ prefix) ---

USTRUCT(BlueprintType)
struct FVFX_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustRadius = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustLifetime = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    int32 ParticleCount = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    EVFX_ImpactSurface SurfaceType = EVFX_ImpactSurface::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ShockwaveRingScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FVFX_CampfireConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameHeight = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberSpawnRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeRiseSpeed = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightFlickerFrequency = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightFlickerAmplitude = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsWet = false;
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
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float AshFallRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float TransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);
};

USTRUCT(BlueprintType)
struct FVFX_BloodImpactConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    EVFX_CombatImpact ImpactType = EVFX_CombatImpact::SpearHit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    float BloodSprayRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    int32 BloodDropCount = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    float DecalSize = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    float DecalLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    bool bSpawnFleshChunks = false;
};

USTRUCT(BlueprintType)
struct FVFX_LODConfig
{
    GENERATED_BODY()

    // LOD0 — full quality (< 1500 units from camera)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD0MaxDistance = 1500.0f;

    // LOD1 — reduced particles (1500-4000 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD1MaxDistance = 4000.0f;

    // LOD2 — billboard/sprite only (4000-8000 units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD2MaxDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD1ParticleScalar = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD2ParticleScalar = 0.15f;
};

// ============================================================
// MAIN VFX SYSTEM MANAGER CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- WEATHER SYSTEM ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_WeatherConfig WeatherConfig;

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void UpdateWeatherVFX(float DeltaTime);

    // ---- CAMPFIRE SYSTEM ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVFX_CampfireConfig CampfireConfig;

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireVFX(FVector Location, bool bIsWet = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void TickCampfireFlicker(float DeltaTime);

    // ---- FOOTSTEP DUST SYSTEM ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FVFX_FootstepConfig FootstepConfig;

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnFootstepDust(FVector ImpactLocation, EVFX_ImpactSurface Surface, float FootRadius = 80.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnTRexFootstepDust(FVector ImpactLocation, float FootRadius = 200.0f);

    // ---- COMBAT IMPACT SYSTEM ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    FVFX_BloodImpactConfig BloodImpactConfig;

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal, EVFX_CombatImpact ImpactType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnSpearImpactDust(FVector HitLocation, EVFX_ImpactSurface Surface);

    // ---- BREATH VAPOR SYSTEM ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Breath")
    void SpawnBreathVapor(FVector MouthLocation, FVector ForwardVector, float TemperatureCelsius);

    UFUNCTION(BlueprintCallable, Category = "VFX|Breath")
    bool ShouldShowBreath(float AmbientTemperatureCelsius) const;

    // ---- VOLCANIC ASH SYSTEM ----
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SetVolcanicAshIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SpawnDistantEruptionGlow(FVector VolcanoLocation);

    // ---- LOD MANAGEMENT ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    FVFX_LODConfig LODConfig;

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    int32 GetVFXLODLevel(FVector EffectLocation) const;

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    float GetParticleScalarForLOD(int32 LODLevel) const;

    // ---- RUNTIME STATE ----
    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    float CurrentVolcanicAshIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    float WeatherTransitionAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    EVFX_WeatherState PreviousWeatherState = EVFX_WeatherState::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    float CampfireFlickerTimer = 0.0f;

private:
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    bool bWeatherTransitioning = false;
};
