#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "VFX_ScreenDamageOverlay.generated.h"

UENUM(BlueprintType)
enum class EVFX_DamageIntensity : uint8
{
    None        UMETA(DisplayName = "No Damage"),
    Light       UMETA(DisplayName = "Light Damage"),
    Medium      UMETA(DisplayName = "Medium Damage"),
    Heavy       UMETA(DisplayName = "Heavy Damage"),
    Critical    UMETA(DisplayName = "Critical Damage")
};

USTRUCT(BlueprintType)
struct FVFX_DamageOverlaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Overlay")
    float RedTintIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Overlay")
    float VignetteStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Overlay")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Overlay")
    float FadeOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Overlay")
    bool bEnableParticleOverlay = true;

    FVFX_DamageOverlaySettings()
    {
        RedTintIntensity = 0.0f;
        VignetteStrength = 0.0f;
        FlashDuration = 0.3f;
        FadeOutTime = 1.0f;
        bEnableParticleOverlay = true;
    }
};

/**
 * VFX Screen Damage Overlay Component
 * Integrates with Audio Agent's screen shake system to provide visual damage feedback
 * Creates red tint, vignette effects, and particle overlays for damage events
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ScreenDamageOverlay : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ScreenDamageOverlay();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === DAMAGE OVERLAY INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void TriggerDamageFlash(EVFX_DamageIntensity Intensity, float CustomDuration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void SetDamageIntensity(EVFX_DamageIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void ClearDamageOverlay();

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void UpdateHealthPercentage(float HealthPercent);

    // === INTEGRATION WITH AUDIO SHAKE SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void OnScreenShakeTriggered(float ShakeIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void SyncWithAudioShake(bool bEnable);

protected:
    // === DAMAGE OVERLAY SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    TMap<EVFX_DamageIntensity, FVFX_DamageOverlaySettings> DamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    float HealthBasedTintMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    float LowHealthThreshold = 0.25f;

    // === POST PROCESS INTEGRATION ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPostProcessComponent* PostProcessComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* DamageParameterCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* DamageOverlayMaterial;

    // === RUNTIME STATE ===
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EVFX_DamageIntensity CurrentDamageLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentHealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsFlashing;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float FlashTimer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float FadeTimer;

    // === AUDIO INTEGRATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    bool bSyncWithScreenShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    float ShakeToFlashMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Integration")
    class UAudio_ScreenShakeManager* LinkedShakeManager;

private:
    // === INTERNAL METHODS ===
    
    void InitializeDamageSettings();
    void UpdatePostProcessParameters();
    void ProcessDamageFlash(float DeltaTime);
    void ApplyDamageOverlay(const FVFX_DamageOverlaySettings& Settings);
    void FindAndLinkShakeManager();

    // === CACHED REFERENCES ===
    
    UPROPERTY()
    class APlayerController* CachedPlayerController;

    UPROPERTY()
    class UMaterialParameterCollectionInstance* ParameterCollectionInstance;
};