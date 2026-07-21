#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraComponent.h"
#include "../SharedTypes.h"
#include "VFX_ScreenShakeVisualController.generated.h"

/**
 * VFX Screen Shake Visual Controller
 * Creates visual effects that synchronize with audio polish system screen shake
 * Provides realistic visual feedback for T-Rex proximity, combat impacts, and environmental tremors
 * Designed for Cretaceous period survival game with scientific accuracy
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ScreenShakeVisualController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ScreenShakeVisualController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* ScreenShakeParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UStaticMeshComponent* DebrisVisualizerComponent;

    // Screen Shake Visual Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake VFX")
    float ShakeIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake VFX")
    float ShakeDurationSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake VFX")
    float DebrisParticleScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake VFX")
    FLinearColor ShakeOverlayColor = FLinearColor(0.8f, 0.6f, 0.4f, 0.3f);

    // T-Rex Proximity Visual Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float TRexProximityThreshold = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float GroundCrackIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Proximity")
    float SeismicTremorScale = 2.0f;

    // Combat Impact Visual Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float CombatImpactShakeScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float BloodSplatterIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    FLinearColor DamageFlashColor = FLinearColor(1.0f, 0.2f, 0.2f, 0.5f);

    // Environmental Shake Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float EnvironmentalShakeThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float DustCloudScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float RockDebrisAmount = 50.0f;

    // Material References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ScreenShakeOverlayMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInstanceDynamic* DynamicShakeOverlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* DebrisMaterial;

    // Niagara Effect References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Effects")
    UNiagaraSystem* ScreenShakeParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Effects")
    UNiagaraSystem* GroundCrackSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Effects")
    UNiagaraSystem* DustCloudSystem;

    // Audio Sync Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    bool bSyncWithAudioSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    float AudioSyncDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    float AudioVisualSyncMultiplier = 1.0f;

public:
    // Screen Shake Visual Methods
    UFUNCTION(BlueprintCallable, Category = "Screen Shake VFX")
    void TriggerScreenShakeVisual(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake VFX")
    void StartScreenShakeOverlay(FLinearColor OverlayColor, float FadeDuration);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake VFX")
    void StopScreenShakeOverlay(float FadeOutDuration);

    // T-Rex Proximity Visual Methods
    UFUNCTION(BlueprintCallable, Category = "T-Rex Proximity")
    void TriggerTRexProximityVisuals(FVector TRexLocation, float Distance);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Proximity")
    void CreateGroundCrackEffects(FVector ImpactLocation, float CrackRadius);

    UFUNCTION(BlueprintCallable, Category = "T-Rex Proximity")
    void SpawnSeismicTremorParticles(FVector EpicenterLocation, float TremorIntensity);

    // Combat Impact Visual Methods
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerCombatImpactShake(FVector ImpactLocation, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void CreateBloodSplatterVisuals(FVector ImpactPoint, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerDamageFlashOverlay(float DamageAmount, FLinearColor FlashColor);

    // Environmental Shake Methods
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void TriggerEnvironmentalShake(FVector ShakeSource, float ShakeRadius);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SpawnDustCloudParticles(FVector DustLocation, float CloudSize);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void CreateRockDebrisEffects(FVector DebrisLocation, int32 DebrisCount);

    // Audio Synchronization Methods
    UFUNCTION(BlueprintCallable, Category = "Audio Sync")
    void SyncWithAudioShakeSystem(float AudioIntensity, float AudioDuration);

    UFUNCTION(BlueprintCallable, Category = "Audio Sync")
    void RegisterAudioSyncCallback();

    UFUNCTION(BlueprintCallable, Category = "Audio Sync")
    void UnregisterAudioSyncCallback();

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "VFX Utilities")
    void SetShakeIntensityMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "VFX Utilities")
    float CalculateDistanceBasedIntensity(FVector SourceLocation, FVector TargetLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "VFX Utilities")
    void UpdateMaterialParameters(float IntensityValue, FLinearColor ColorValue);

private:
    // Internal state tracking
    bool bIsShakeActive = false;
    float CurrentShakeIntensity = 0.0f;
    float ShakeTimer = 0.0f;
    FVector LastShakeLocation = FVector::ZeroVector;

    // Internal methods
    void InitializeComponents();
    void LoadDefaultAssets();
    void UpdateShakeVisuals(float DeltaTime);
    void CleanupExpiredEffects();
};