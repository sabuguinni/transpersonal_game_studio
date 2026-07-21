#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "NightAtmosphereController.generated.h"

/**
 * Night / Blue-Hour Atmosphere Controller
 * Manages the deep night lighting palette for the prehistoric survival game.
 * Provides moonlight simulation, bioluminescent ground glow, and campfire warmth.
 * Part of the day/night cycle system — handles the 22:00–04:00 time window.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Night Atmosphere Controller"))
class TRANSPERSONALGAME_API ANightAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ANightAtmosphereController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Moonlight Settings ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Moonlight")
    float MoonIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Moonlight")
    FLinearColor MoonColor = FLinearColor(0.55f, 0.65f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Moonlight")
    float MoonPitchDegrees = -15.0f;

    // ── Bioluminescence Settings ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Bioluminescence")
    FLinearColor BioGlowColor = FLinearColor(0.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Bioluminescence")
    float BioGlowIntensity = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Bioluminescence")
    float BioGlowRadius = 1500.0f;

    // ── Campfire Settings ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Campfire")
    FLinearColor CampfireColor = FLinearColor(1.0f, 0.45f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Campfire")
    float CampfireIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Campfire")
    float CampfireFlickerSpeed = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Campfire")
    float CampfireFlickerAmplitude = 0.15f;

    // ── Fog Settings ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Fog")
    float NightFogDensity = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Fog")
    FLinearColor NightFogColor = FLinearColor(0.02f, 0.04f, 0.18f, 1.0f);

    // ── Star Visibility ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Sky")
    float StarBrightness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night|Sky")
    bool bEnableStarField = true;

    // ── Runtime State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Night|State")
    float CurrentNightTime = 0.0f;  // 0..1 normalized night progress

    UPROPERTY(BlueprintReadOnly, Category = "Night|State")
    bool bIsDeepNight = false;

    // ── Blueprint Events ────────────────────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Night|Events")
    void OnDeepNightBegin();

    UFUNCTION(BlueprintImplementableEvent, Category = "Night|Events")
    void OnDawnApproaching();

    UFUNCTION(BlueprintCallable, Category = "Night|Control")
    void SetNightProgress(float NormalizedProgress);

    UFUNCTION(BlueprintCallable, Category = "Night|Control")
    void ApplyNightPalette();

    UFUNCTION(BlueprintCallable, Category = "Night|Campfire")
    void RegisterCampfireLight(class UPointLightComponent* CampfireComp);

    UFUNCTION(BlueprintCallable, Category = "Night|Bioluminescence")
    void SpawnBioluminescentPatch(FVector WorldLocation, float Radius);

private:
    // Cached light references
    UPROPERTY()
    TArray<class UPointLightComponent*> CampfireLights;

    UPROPERTY()
    TArray<class UPointLightComponent*> BioLights;

    float FlickerAccumulator = 0.0f;

    void TickCampfireFlicker(float DeltaTime);
    void UpdateNightFog();
};
