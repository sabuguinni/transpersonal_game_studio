#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire UMETA(DisplayName = "Campfire"),
    Dust_Footstep UMETA(DisplayName = "Footstep Dust"),
    Blood_Impact UMETA(DisplayName = "Blood Splatter"),
    Water_Splash UMETA(DisplayName = "Water Splash"),
    Smoke_General UMETA(DisplayName = "General Smoke"),
    Rain_Weather UMETA(DisplayName = "Rain"),
    Wind_Particles UMETA(DisplayName = "Wind Debris"),
    Volcanic_Ash UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* PrimaryEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* SecondaryEffectComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EffectData CurrentEffectData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_EffectType, class UNiagaraSystem*> EffectSystems;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayEffect(EVFX_EffectType EffectType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayBloodSplatter(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void PlayWaterSplash(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartWeatherEffect(EVFX_EffectType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopWeatherEffect();

private:
    void InitializeEffectSystems();
    UNiagaraSystem* GetEffectSystem(EVFX_EffectType EffectType);
    void UpdateEffectParameters();

    float EffectTimer;
    bool bEffectActive;
};