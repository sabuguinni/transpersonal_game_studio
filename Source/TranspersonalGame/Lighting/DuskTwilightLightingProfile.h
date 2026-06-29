#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "DuskTwilightLightingProfile.generated.h"

/**
 * FLight_DuskPalette
 * Colour and intensity parameters for the dusk/twilight lighting palette.
 * All values tuned for Cretaceous prehistoric atmosphere.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DuskPalette
{
    GENERATED_BODY()

    /** Sun (directional light) intensity at dusk — low, warm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float SunIntensity = 1.8f;

    /** Sun colour — deep orange-amber */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    FLinearColor SunColor = FLinearColor(1.0f, 0.52f, 0.18f, 1.0f);

    /** Warm fill light intensity (orange key from low horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float WarmFillIntensity = 8000.0f;

    /** Warm fill colour */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    FLinearColor WarmFillColor = FLinearColor(1.0f, 0.45f, 0.12f, 1.0f);

    /** Cool rim light intensity (blue-violet from opposite horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float CoolRimIntensity = 4500.0f;

    /** Cool rim colour */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    FLinearColor CoolRimColor = FLinearColor(0.18f, 0.22f, 0.85f, 1.0f);

    /** Purple shadow fill intensity (deep ambient bounce) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float PurpleShadowIntensity = 2200.0f;

    /** Purple shadow colour */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    FLinearColor PurpleShadowColor = FLinearColor(0.55f, 0.18f, 0.72f, 1.0f);

    /** Fog density at dusk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    float FogDensity = 0.045f;

    /** Fog inscattering colour — warm orange */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    FLinearColor FogInscatterColor = FLinearColor(0.72f, 0.38f, 0.18f, 1.0f);

    /** Volumetric fog albedo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.85f, 0.62f, 0.42f, 1.0f);

    /** Volumetric fog extinction scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    float VolumetricFogExtinctionScale = 1.4f;

    /** Bloom intensity for dusk glow */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|PostProcess")
    float BloomIntensity = 0.55f;

    /** Auto exposure min brightness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|PostProcess")
    float AutoExposureMin = 0.45f;

    /** Auto exposure max brightness */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|PostProcess")
    float AutoExposureMax = 2.8f;
};

/**
 * ELight_TimeOfDay
 * Enumeration of supported lighting time-of-day presets.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

/**
 * ADuskTwilightLightingProfile
 * 
 * Actor that manages the dusk/twilight lighting palette for the prehistoric world.
 * Applies cinematic three-point fill lighting (warm orange key, cool blue rim,
 * purple shadow bounce) tuned for Cretaceous atmosphere.
 * 
 * Place one instance in the level. Call ApplyDuskPalette() to activate.
 * Integrates with the day/night cycle system via SetTimeOfDay().
 */
UCLASS(ClassGroup = "Lighting", meta = (DisplayName = "Dusk Twilight Lighting Profile"))
class TRANSPERSONALGAME_API ADuskTwilightLightingProfile : public AActor
{
    GENERATED_BODY()

public:
    ADuskTwilightLightingProfile();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── Palette Parameters ───────────────────────────────────────────────

    /** Active dusk palette parameters — editable in editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk Palette")
    FLight_DuskPalette DuskPalette;

    /** Current time of day preset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dusk;

    /** Whether to auto-apply palette on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk Palette")
    bool bAutoApplyOnBeginPlay = true;

    /** Whether Lumen GI is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGIEnabled = true;

    /** Whether Lumen Reflections are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflectionsEnabled = true;

    /** Whether volumetric fog is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFogEnabled = true;

    // ─── References (auto-found in BeginPlay) ────────────────────────────

    /** Cached directional light reference */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ADirectionalLight> DirectionalLightRef;

    /** Cached exponential height fog reference */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AExponentialHeightFog> HeightFogRef;

    /** Cached sky light reference */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ASkyLight> SkyLightRef;

    /** Cached post process volume reference */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<APostProcessVolume> PostProcessRef;

    // ─── Fill Light Actors ────────────────────────────────────────────────

    /** Spawned warm fill point light */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|FillLights", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<APointLight> WarmFillLight;

    /** Spawned cool rim point light */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|FillLights", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<APointLight> CoolRimLight;

    /** Spawned purple shadow fill point light */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|FillLights", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<APointLight> PurpleShadowLight;

    // ─── Public API ───────────────────────────────────────────────────────

    /** Apply the dusk palette to all lighting actors in the world */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Dusk Palette")
    void ApplyDuskPalette();

    /** Set time of day and blend to appropriate palette */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Time")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    /** Find and cache all lighting actors from the world */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Setup")
    void FindLightingActors();

    /** Spawn the three fill lights for dusk palette */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Setup")
    void SpawnFillLights();

    /** Remove all spawned fill lights */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Setup")
    void RemoveFillLights();

    /** Get the current palette blend alpha (0=dawn, 1=midnight) */
    UFUNCTION(BlueprintPure, Category = "Lighting|Time")
    float GetTimeOfDayAlpha() const;

private:
    /** Apply palette to the directional light */
    void ApplyToDirectionalLight();

    /** Apply palette to the height fog */
    void ApplyToHeightFog();

    /** Apply palette to the post process volume */
    void ApplyToPostProcess();

    /** Apply palette to the sky light */
    void ApplyToSkyLight();
};
