#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "Particles/ParticleSystem.h"
#include "Components/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Audio_ImmersiveFeedbackController.generated.h"

/**
 * Audio_ImmersiveFeedbackController
 * 
 * Manages immersive audio-visual feedback systems including:
 * - Screen shake effects when T-Rex approaches
 * - Damage flash overlay effects with audio
 * - Footstep particle systems for player and creatures
 * - Dynamic audio feedback based on gameplay events
 * 
 * This controller enhances game feel through synchronized audio-visual responses
 * that make the prehistoric world feel more alive and reactive.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ImmersiveFeedbackController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ImmersiveFeedbackController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === SCREEN SHAKE SYSTEM ===
    
    /** Audio component for screen shake sound effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ScreenShakeAudioComponent;
    
    /** Camera shake class to use for T-Rex proximity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TSubclassOf<UCameraShakeBase> TRexScreenShakeClass;
    
    /** Sound to play during screen shake events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    USoundBase* ScreenShakeSound;
    
    /** Distance at which T-Rex triggers screen shake */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (ClampMin = "500", ClampMax = "5000"))
    float TRexProximityDistance;
    
    /** Intensity multiplier for screen shake effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float ScreenShakeIntensity;
    
    /** Cooldown between screen shake triggers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TRexShakeCooldown;
    
    /** Whether screen shake effects are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bScreenShakeEnabled;

    // === DAMAGE FLASH SYSTEM ===
    
    /** Audio component for damage flash sound effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* DamageFlashAudioComponent;
    
    /** Sound to play during damage flash */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    USoundBase* DamageFlashSound;
    
    /** Material parameter collection for damage flash effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    UMaterialParameterCollection* DamageFlashMPC;
    
    /** Duration of damage flash effect in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float DamageFlashDuration;
    
    /** Maximum intensity of damage flash effect */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float DamageFlashIntensity;
    
    /** Whether damage flash effects are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    bool bDamageFlashEnabled;

    // === FOOTSTEP PARTICLE SYSTEM ===
    
    /** Particle component for footstep effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UParticleSystemComponent* FootstepParticleComponent;
    
    /** Particle system for player footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles")
    UParticleSystem* PlayerFootstepParticles;
    
    /** Particle system for large creature footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles")
    UParticleSystem* LargeFootstepParticles;
    
    /** Sound for player footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles")
    USoundBase* PlayerFootstepSound;
    
    /** Sound for large creature footsteps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles")
    USoundBase* LargeFootstepSound;
    
    /** Scale multiplier for footstep particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float FootstepParticleScale;
    
    /** Whether footstep particle effects are enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Particles")
    bool bFootstepParticlesEnabled;

private:
    // === CACHED REFERENCES ===
    
    /** Cached player controller reference */
    UPROPERTY()
    APlayerController* CachedPlayerController;
    
    /** Cached player character reference */
    UPROPERTY()
    ACharacter* CachedPlayerCharacter;

    // === INTERNAL STATE ===
    
    /** Last time T-Rex screen shake was triggered */
    float LastTRexShakeTime;
    
    /** Time when current damage flash started */
    float DamageFlashStartTime;
    
    /** Current damage flash intensity */
    float CurrentDamageFlashIntensity;
    
    /** Whether damage flash is currently active */
    bool bDamageFlashActive;
    
    /** Last time player footstep was triggered */
    float LastPlayerFootstepTime;

    // === INTERNAL METHODS ===
    
    /** Update T-Rex proximity screen shake effects */
    void UpdateTRexProximityShake(float DeltaTime);
    
    /** Update damage flash visual effect */
    void UpdateDamageFlashEffect(float DeltaTime);
    
    /** Update footstep particle effects */
    void UpdateFootstepParticles(float DeltaTime);

public:
    // === PUBLIC INTERFACE ===
    
    /** Trigger screen shake effect based on T-Rex distance */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void TriggerTRexScreenShake(float Distance);
    
    /** Trigger damage flash effect with specified damage amount */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void TriggerDamageFlash(float Damage);
    
    /** Trigger footstep particles at specified location */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void TriggerFootstepParticles(const FVector& Location, bool bIsLargeCreature = false);
    
    /** Enable or disable screen shake effects */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void SetScreenShakeEnabled(bool bEnabled);
    
    /** Enable or disable damage flash effects */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void SetDamageFlashEnabled(bool bEnabled);
    
    /** Enable or disable footstep particle effects */
    UFUNCTION(BlueprintCallable, Category = "Immersive Feedback")
    void SetFootstepParticlesEnabled(bool bEnabled);
};

#include "Audio_ImmersiveFeedbackController.generated.h"