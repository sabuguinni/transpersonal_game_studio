#pragma once

// VFXManager.h — VFX Agent #17
// Manages all Niagara particle systems and VFX for the prehistoric survival game.
// Handles campfire, footstep dust, blood impact, weather particles, and environmental VFX.
// All effects are physically plausible in a prehistoric world — no magical/spiritual content.

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "VFXManager.generated.h"

// ─── VFX Zone Type ───────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EVFX_ZoneType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    CampSite        UMETA(DisplayName = "Camp Site"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    Swamp           UMETA(DisplayName = "Swamp")
};

// ─── VFX Impact Type ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinoFootstepDirt    UMETA(DisplayName = "Dino Footstep Dirt"),
    DinoFootstepMud     UMETA(DisplayName = "Dino Footstep Mud"),
    DinoFootstepRock    UMETA(DisplayName = "Dino Footstep Rock"),
    WeaponHitFlesh      UMETA(DisplayName = "Weapon Hit Flesh"),
    WeaponHitRock       UMETA(DisplayName = "Weapon Hit Rock"),
    WeaponHitWood       UMETA(DisplayName = "Weapon Hit Wood"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    CraftingSparkFlint  UMETA(DisplayName = "Crafting Spark Flint"),
    WaterSplash         UMETA(DisplayName = "Water Splash")
};

// ─── VFX Weather Type ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    Ash         UMETA(DisplayName = "Volcanic Ash"),
    Dust        UMETA(DisplayName = "Dust Storm")
};

// ─── Campfire VFX Data ───────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FVFX_CampfireData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameIntensity = 1.0f;       // 0.0 = dying, 1.0 = normal, 2.0 = large

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberSpawnRate = 15.0f;      // embers per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightRadius = 600.0f;        // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsActive = true;
};

// ─── Impact VFX Data ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    FVector HitNormal = FVector(0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinoFootstepDirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    float ImpactForce = 1.0f;          // scales particle count and spread

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    float DustRadius = 80.0f;          // cm, for footstep dust cloud
};

// ─── AVFXManager Actor ───────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "VFX Manager"))
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Campfire VFX ──────────────────────────────────────────────────────────

    /** Spawn or update a campfire VFX at the given location */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireVFX(const FVFX_CampfireData& CampfireData);

    /** Extinguish a campfire VFX (rain or player action) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishCampfire(FVector Location, bool bPlaySteamEffect = true);

    /** Update campfire intensity (fuel level drives this) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SetCampfireIntensity(FVector Location, float NewIntensity);

    // ── Impact VFX ────────────────────────────────────────────────────────────

    /** Spawn an impact VFX at a hit location (footstep, weapon, etc.) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnImpactVFX(const FVFX_ImpactData& ImpactData);

    /** Spawn dinosaur footstep dust — called from Animation Notify */
    UFUNCTION(BlueprintCallable, Category = "VFX|Dino")
    void SpawnDinoFootstepDust(FVector FootLocation, FVector FootNormal, float DinoMass);

    /** Spawn blood splatter on hit */
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodSplatter(FVector HitLocation, FVector HitDirection, float DamageAmount);

    // ── Weather VFX ───────────────────────────────────────────────────────────

    /** Set current weather VFX state */
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherVFX(EVFX_WeatherType WeatherType, float TransitionTime = 3.0f);

    /** Get current weather type */
    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    EVFX_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    // ── Environmental VFX ─────────────────────────────────────────────────────

    /** Spawn ambient insects/pollen particles for a zone */
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetZoneAmbientVFX(EVFX_ZoneType ZoneType, FVector ZoneCenter, float ZoneRadius);

    /** Trigger volcanic ash particle burst */
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void TriggerVolcanicAshBurst(FVector EruptionDirection, float Intensity);

    // ── Campfire Light (runtime) ───────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Campfire",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPointLightComponent> CampfireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Campfire",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPointLightComponent> CampfireFlickerLight;

    // ── State ─────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|State")
    EVFX_WeatherType CurrentWeather = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|State")
    float WeatherTransitionAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|State")
    EVFX_ZoneType ActiveZoneType = EVFX_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    TArray<FVFX_CampfireData> ActiveCampfires;

private:
    // Campfire flicker simulation
    float FlickerTimer = 0.0f;
    float FlickerPhase = 0.0f;
    bool bFlickerActive = false;

    void UpdateCampfireFlicker(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
};
