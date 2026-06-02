#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFX_TribalDialogueSystem.generated.h"

/**
 * VFX system for tribal NPC dialogue integration
 * Creates visual effects to complement the Audio Agent's tribal NPC audio system
 * Handles dialogue bubbles, sound wave visualizations, and ambient effects
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_TribalDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_TribalDialogueSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Dialogue bubble VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dialogue")
    void ShowDialogueBubble(const FString& DialogueText, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dialogue")
    void HideDialogueBubble();

    // Sound wave visualization
    UFUNCTION(BlueprintCallable, Category = "VFX|Audio")
    void CreateSoundWaveEffect(const FVector& SourceLocation, float Intensity = 1.0f);

    // Screen shake for audio integration
    UFUNCTION(BlueprintCallable, Category = "VFX|Audio")
    void TriggerAudioScreenShake(float Magnitude = 0.5f);

    // Ambient effects for tribal areas
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StartAmbientTribalEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StopAmbientTribalEffects();

protected:
    // VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* DialogueBubbleEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* SoundWaveEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* AmbientDustEffect;

    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float DialogueBubbleLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float SoundWaveMaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float AmbientEffectIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bAutoStartAmbientEffects;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    bool bDialogueBubbleActive;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    bool bAmbientEffectsActive;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|State")
    float CurrentDialogueTimer;

private:
    // Internal methods
    void InitializeVFXComponents();
    void UpdateDialogueBubbleTimer(float DeltaTime);
    void CleanupExpiredEffects();

    // Effect management
    TArray<class UNiagaraComponent*> ActiveSoundWaveEffects;
    float LastSoundWaveTime;
    static constexpr float SoundWaveCooldown = 0.1f;
};