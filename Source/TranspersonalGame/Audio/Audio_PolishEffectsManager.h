#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_PolishEffectsManager.generated.h"

class UAudioComponent;
class USoundCue;
class UParticleSystemComponent;
class UParticleSystem;
class UMaterialParameterCollection;

/**
 * Audio Polish Effects Manager
 * Handles game feel and feedback systems including:
 * - Screen shake when large dinosaurs walk nearby
 * - Damage flash effect (red screen overlay when hit)
 * - Footstep dust particles for player and dinosaurs
 * - Day/night cycle using rotating directional light
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_PolishEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_PolishEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Screen Shake System
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerScreenShake(float Intensity, FVector Location);
    
    // Damage Flash System
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void TriggerDamageFlash(float Intensity = 1.0f);
    
    // Footstep Particle System
    UFUNCTION(BlueprintCallable, Category = "Polish Effects")
    void SpawnFootstepDust(FVector Location, float Scale = 1.0f);
    
    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayProximityAlert(FVector ThreatLocation, float ThreatSize);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalAudio(const FString& BiomeType);
    
    // Day/Night Cycle
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetDayNightSpeed(float NewDayDuration);
    
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);
    
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const;

protected:
    // Initialization
    void InitializeEffectsSystem();
    
    // Update functions
    void UpdateProximityDetection();
    void UpdateDamageFlashEffect(float DeltaTime);
    void UpdateFootstepEffects();
    void UpdateDayNightCycle();

    // Screen Shake Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeIntensity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeRadius;
    
    // Damage Flash Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float DamageFlashDuration;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float DamageFlashIntensity;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Flash")
    float DamageFlashCurrentIntensity;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Flash")
    float DamageFlashTimer;
    
    // Footstep Dust Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Effects")
    float FootstepDustScale;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Effects")
    float FootstepDustLifetime;
    
    // Day/Night Cycle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDuration; // Duration of full day in seconds
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle")
    float CurrentTimeOfDay; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    
    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* ProximityAudioComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* EnvironmentalAudioComponent;
    
    // Particle Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
    UParticleSystemComponent* FootstepParticleComponent;
    
    // Asset References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* ProximityAlertSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* EnvironmentalSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Assets")
    UParticleSystem* FootstepParticleSystem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Assets")
    UMaterialParameterCollection* DamageFlashMPC;
    
    // World References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World References")
    ADirectionalLight* DirectionalLight;
    
    // Internal State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsInitialized;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float LastProximityAlertTime;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FVector LastPlayerLocation;
    
    // Timers
    FTimerHandle DayNightTimerHandle;
};