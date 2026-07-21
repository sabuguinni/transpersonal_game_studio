#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VFX_PrehistoricFireSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_FireType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    Torch           UMETA(DisplayName = "Torch"),
    WildFire        UMETA(DisplayName = "Wild Fire"),
    CookingFire     UMETA(DisplayName = "Cooking Fire")
};

UENUM(BlueprintType)
enum class EVFX_FireIntensity : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Intense         UMETA(DisplayName = "Intense")
};

USTRUCT(BlueprintType)
struct FVFX_FireEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    TSoftObjectPtr<UNiagaraSystem> FlameEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    TSoftObjectPtr<UNiagaraSystem> SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    TSoftObjectPtr<UNiagaraSystem> EmberEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    TSoftObjectPtr<UNiagaraSystem> SparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Audio")
    TSoftObjectPtr<USoundCue> CracklingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Properties")
    float FlameHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Properties")
    float FlameRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Properties")
    float SmokeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Properties")
    float EmberCount = 20.0f;

    FVFX_FireEffectData()
    {
        FlameHeight = 100.0f;
        FlameRadius = 50.0f;
        SmokeIntensity = 1.0f;
        EmberCount = 20.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_PrehistoricFireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_PrehistoricFireSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Fire Control Functions
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void StartFire(EVFX_FireType FireType, EVFX_FireIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void SetFireIntensity(EVFX_FireIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    bool IsFireActive() const { return bIsFireActive; }

    // Weather Interaction
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void ApplyRainEffect(float RainIntensity);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Fire VFX")
    void ApplyWindEffect(FVector WindDirection, float WindStrength);

protected:
    // Fire Effect Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Components")
    TObjectPtr<UNiagaraComponent> FlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Components")
    TObjectPtr<UNiagaraComponent> SmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Components")
    TObjectPtr<UNiagaraComponent> EmberComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Components")
    TObjectPtr<UNiagaraComponent> SparkComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Components")
    TObjectPtr<UAudioComponent> FireAudioComponent;

    // Fire Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Configuration")
    TMap<EVFX_FireType, FVFX_FireEffectData> FireEffectDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Configuration")
    float FuelLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Configuration")
    float FuelConsumptionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Configuration")
    float MaxFuelLevel = 100.0f;

    // Fire State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire State")
    bool bIsFireActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire State")
    EVFX_FireType CurrentFireType = EVFX_FireType::Campfire;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire State")
    EVFX_FireIntensity CurrentIntensity = EVFX_FireIntensity::Medium;

private:
    void InitializeFireEffects();
    void UpdateFireEffects(float DeltaTime);
    void UpdateFuelConsumption(float DeltaTime);
    void SetEffectParameters();
    void LoadFireEffectData();
};