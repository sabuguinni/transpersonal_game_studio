#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "VFX_FireEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_FireType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    Torch           UMETA(DisplayName = "Torch"),
    Wildfire        UMETA(DisplayName = "Wildfire"),
    Cooking         UMETA(DisplayName = "Cooking Fire")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FireSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float FireIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float SmokeAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float EmberCount = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    FLinearColor FlameColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    bool bCastLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float LightRadius = 500.0f;

    FVFX_FireSettings()
    {
        FireIntensity = 1.0f;
        SmokeAmount = 1.0f;
        EmberCount = 50.0f;
        FlameColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
        bCastLight = true;
        LightRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FireEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FireEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Fire Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void StartFire(EVFX_FireType FireType, const FVFX_FireSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void SetFireIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    bool IsFireActive() const;

    // Fire State Management
    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    float GetFireIntensity() const { return CurrentSettings.FireIntensity; }

protected:
    // VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FlameEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* SmokeEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* EmberEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UAudioComponent* FireAudio;

    // Fire Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FlameNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* SmokeNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* EmberNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class USoundCue* FireCrackleSound;

    // Fire State
    UPROPERTY(BlueprintReadOnly, Category = "Fire State")
    EVFX_FireType CurrentFireType;

    UPROPERTY(BlueprintReadOnly, Category = "Fire State")
    FVFX_FireSettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Fire State")
    bool bIsFireActive;

    UPROPERTY(BlueprintReadOnly, Category = "Fire State")
    float FuelRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Fire State")
    float BurnRate;

private:
    void UpdateFireEffects();
    void UpdateLighting();
    void UpdateAudio();
    void LoadFireAssets();
    void CreateVFXComponents();

    // Fire simulation
    float FireFlickerTime;
    float LightFlickerIntensity;
};