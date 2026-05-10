#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VFX_CretaceousFireSystem.generated.h"

/**
 * Realistic fire and smoke VFX system for Cretaceous period survival gameplay
 * Handles campfires, natural fires, and smoke effects with authentic prehistoric atmosphere
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousFireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousFireSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Fire VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire VFX")
    class UNiagaraComponent* FireParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire VFX")
    class UNiagaraComponent* SmokeParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire VFX")
    class UNiagaraComponent* EmberParticleComponent;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Audio")
    class UAudioComponent* FireCrackleAudioComponent;

    // Fire Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float FireIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float SmokeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    bool bIsLit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float FuelLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float BurnRate = 1.0f;

    // Wind Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WindStrength = 1.0f;

    // Niagara Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FireNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* SmokeNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* EmberNiagaraSystem;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* FireCrackleSoundCue;

public:
    // Fire Control Functions
    UFUNCTION(BlueprintCallable, Category = "Fire Control")
    void LightFire();

    UFUNCTION(BlueprintCallable, Category = "Fire Control")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Fire Control")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "Fire Control")
    void SetFireIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Fire Control")
    void SetWindEffect(FVector NewWindDirection, float NewWindStrength);

    // Fire State Queries
    UFUNCTION(BlueprintPure, Category = "Fire State")
    bool IsFireLit() const { return bIsLit; }

    UFUNCTION(BlueprintPure, Category = "Fire State")
    float GetFuelLevel() const { return FuelLevel; }

    UFUNCTION(BlueprintPure, Category = "Fire State")
    float GetFireIntensity() const { return FireIntensity; }

private:
    // Internal fire management
    void UpdateFireEffects();
    void UpdateSmokeEffects();
    void UpdateEmberEffects();
    void UpdateAudioEffects();
    void ConsumeFuel(float DeltaTime);
    void ApplyWindEffects();

    // Fire state tracking
    float LastIntensityUpdate = 0.0f;
    float IntensityUpdateInterval = 0.1f;
    bool bNeedsEffectUpdate = true;
};