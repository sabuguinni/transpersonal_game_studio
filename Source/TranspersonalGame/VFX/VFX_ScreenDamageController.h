#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "VFX_ScreenDamageController.generated.h"

/**
 * VFX Screen Damage Controller - Manages red screen overlay and visual damage feedback
 * Integrates with Audio Agent's damage flash system for synchronized audio-visual effects
 * Provides realistic damage visualization for dinosaur survival gameplay
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ScreenDamageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ScreenDamageController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === DAMAGE OVERLAY SYSTEM ===
    
    /** Trigger red screen damage overlay with specified intensity */
    UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
    void TriggerDamageOverlay(float Intensity = 1.0f, float Duration = 0.5f);
    
    /** Set continuous damage overlay (for bleeding/poison effects) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
    void SetContinuousDamageOverlay(float Intensity, bool bEnabled = true);
    
    /** Clear all damage overlays immediately */
    UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
    void ClearDamageOverlay();
    
    // === SCREEN DISTORTION EFFECTS ===
    
    /** Apply screen distortion effect (for T-Rex proximity/screen shake) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Distortion")
    void ApplyScreenDistortion(float Intensity, float Duration = 1.0f);
    
    /** Set screen edge vignette effect (for fear/stress) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Distortion")
    void SetScreenVignette(float Intensity, bool bEnabled = true);
    
    // === HEALTH STATE VISUALIZATION ===
    
    /** Update screen effects based on player health percentage */
    UFUNCTION(BlueprintCallable, Category = "VFX|Health")
    void UpdateHealthVisualization(float HealthPercentage);
    
    /** Apply low health visual effects (desaturation, blur) */
    UFUNCTION(BlueprintCallable, Category = "VFX|Health")
    void SetLowHealthEffects(bool bEnabled);

protected:
    // === DAMAGE OVERLAY PROPERTIES ===
    
    /** Material for red damage overlay effect */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Materials")
    class UMaterialInterface* DamageOverlayMaterial;
    
    /** Post-process volume for screen effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components")
    class APostProcessVolume* ScreenEffectsVolume;
    
    /** Material parameter collection for screen effects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Materials")
    class UMaterialParameterCollection* ScreenEffectsParameters;
    
    // === DAMAGE OVERLAY SETTINGS ===
    
    /** Maximum damage overlay intensity */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Damage", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float MaxDamageIntensity = 1.0f;
    
    /** Damage overlay fade speed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Damage", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DamageFadeSpeed = 2.0f;
    
    /** Color of damage overlay (red by default) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Damage")
    FLinearColor DamageOverlayColor = FLinearColor::Red;
    
    // === DISTORTION SETTINGS ===
    
    /** Screen distortion strength multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Distortion", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DistortionStrength = 1.0f;
    
    /** Vignette effect radius */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Distortion", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float VignetteRadius = 0.8f;
    
    /** Vignette effect softness */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Distortion", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float VignetteSoftness = 0.5f;

private:
    // === RUNTIME STATE ===
    
    /** Current damage overlay intensity */
    float CurrentDamageIntensity = 0.0f;
    
    /** Target damage overlay intensity */
    float TargetDamageIntensity = 0.0f;
    
    /** Current distortion intensity */
    float CurrentDistortionIntensity = 0.0f;
    
    /** Target distortion intensity */
    float TargetDistortionIntensity = 0.0f;
    
    /** Current vignette intensity */
    float CurrentVignetteIntensity = 0.0f;
    
    /** Damage overlay timer */
    float DamageOverlayTimer = 0.0f;
    
    /** Distortion effect timer */
    float DistortionTimer = 0.0f;
    
    /** Is continuous damage overlay active */
    bool bContinuousDamageActive = false;
    
    /** Is low health effects active */
    bool bLowHealthEffectsActive = false;
    
    // === HELPER FUNCTIONS ===
    
    /** Initialize post-process volume and materials */
    void InitializeScreenEffects();
    
    /** Update material parameters */
    void UpdateMaterialParameters();
    
    /** Interpolate damage overlay intensity */
    void UpdateDamageOverlay(float DeltaTime);
    
    /** Interpolate distortion effects */
    void UpdateDistortionEffects(float DeltaTime);
    
    /** Apply health-based visual effects */
    void ApplyHealthEffects(float HealthPercentage);
};