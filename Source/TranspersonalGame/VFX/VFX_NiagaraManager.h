#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    FootstepDust        UMETA(DisplayName = "Footstep Dust"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    CampfireFire        UMETA(DisplayName = "Campfire Fire"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    CraftingSparks      UMETA(DisplayName = "Crafting Sparks"),
    EnvironmentalDust   UMETA(DisplayName = "Environmental Dust"),
    VolumetricGodRays   UMETA(DisplayName = "Volumetric God Rays"),
    WeatherRain         UMETA(DisplayName = "Weather Rain")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float IntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::FootstepDust;
        IntensityScale = 1.0f;
        Duration = 2.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* PrimaryVFXComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EffectData> AvailableEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType CurrentEffectType = EVFX_EffectType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bPlayerTriggered = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bDinosaurTriggered = true;

private:
    float LastTriggerTime = 0.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerEffect(EVFX_EffectType EffectType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectType(EVFX_EffectType NewEffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectActive() const;

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "VFX")
    void OnEffectTriggered(EVFX_EffectType EffectType, FVector Location);

    void InitializeEffectData();
    bool CanTriggerEffect() const;
    UNiagaraSystem* GetNiagaraSystemForEffect(EVFX_EffectType EffectType) const;
};