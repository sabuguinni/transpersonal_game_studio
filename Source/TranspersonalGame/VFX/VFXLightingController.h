#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "VFXLightingController.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EVFX_LightZoneType — identifies the ecological/gameplay zone for VFX lighting
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EVFX_LightZoneType : uint8
{
    Campfire    UMETA(DisplayName = "Campfire"),
    Volcanic    UMETA(DisplayName = "Volcanic Glow"),
    ColdZone    UMETA(DisplayName = "Cold Zone"),
    DangerZone  UMETA(DisplayName = "Danger Zone"),
    WaterReflect UMETA(DisplayName = "Water Reflection"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FVFX_LightFlickerParams — parameters for procedural light flicker simulation
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FVFX_LightFlickerParams
{
    GENERATED_BODY()

    /** Base intensity in lux */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    float BaseIntensity = 2500.0f;

    /** Maximum deviation from base intensity (±) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    float FlickerAmplitude = 400.0f;

    /** Flicker speed — higher = more chaotic (campfire ~8.0, torch ~5.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    float FlickerFrequency = 8.0f;

    /** Secondary noise frequency for organic feel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    float SecondaryFrequency = 3.2f;

    /** Colour shift on flicker peak (warm orange for fire) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    FLinearColor PeakColor = FLinearColor(1.0f, 0.55f, 0.1f, 1.0f);

    /** Colour at flicker trough (cooler, dimmer) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    FLinearColor TroughColor = FLinearColor(1.0f, 0.28f, 0.04f, 1.0f);
};

// ─────────────────────────────────────────────────────────────────────────────
// AVFX_LightingController — manages dynamic VFX lighting for prehistoric world
// Handles campfire flicker, volcanic pulsing, danger zone alerts
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = "VFX", meta = (DisplayName = "VFX Lighting Controller"))
class TRANSPERSONALGAME_API AVFX_LightingController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_LightingController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Zone type determines default flicker behaviour ──────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Zone")
    EVFX_LightZoneType ZoneType = EVFX_LightZoneType::Campfire;

    // ── Flicker parameters (editable per instance) ──────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
    FVFX_LightFlickerParams FlickerParams;

    // ── Whether this light is currently active ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|State")
    bool bLightActive = true;

    // ── Danger level input (0.0-1.0) from AudioZoneManager ──────────────────
    // Higher danger = more aggressive flicker + redder tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Danger")
    float ExternalDangerLevel = 0.0f;

    // ── Radius for player proximity check (0 = always active) ───────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float ActiveRadius = 1500.0f;

    // ── LOD: skip flicker update when player is far ──────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    bool bUseLOD = true;

    // ── The point light component ────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components",
              meta = (AllowPrivateAccess = "true"))
    UPointLightComponent* PointLight = nullptr;

    // ── Blueprint-callable: set danger level from gameplay code ─────────────
    UFUNCTION(BlueprintCallable, Category = "VFX|Danger")
    void SetDangerLevel(float NewDangerLevel);

    // ── Blueprint-callable: force light on/off ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "VFX|State")
    void SetLightActive(bool bActive);

    // ── Blueprint-callable: apply preset for zone type ──────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX|Zone")
    void ApplyZonePreset();

private:
    float ElapsedTime = 0.0f;
    float LODUpdateAccumulator = 0.0f;
    bool bPlayerInRange = false;

    void UpdateFlicker(float DeltaTime);
    void UpdatePlayerProximity();
    float ComputeFlickerIntensity() const;
    FLinearColor ComputeFlickerColor() const;
};
