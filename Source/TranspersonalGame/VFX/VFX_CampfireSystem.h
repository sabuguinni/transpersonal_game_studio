#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Components/PointLightComponent.h"
#include "VFX_CampfireSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_FireIntensity : uint8
{
    Embers      UMETA(DisplayName = "Dying Embers"),
    Low         UMETA(DisplayName = "Low Flames"),
    Medium      UMETA(DisplayName = "Medium Fire"),
    High        UMETA(DisplayName = "Roaring Fire"),
    Bonfire     UMETA(DisplayName = "Large Bonfire")
};

USTRUCT(BlueprintType)
struct FVFX_FireParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float FlameHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float FlameIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float SmokeAmount = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float SparkCount = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    FLinearColor FireColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float LightRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float LightIntensity = 2000.0f;

    FVFX_FireParameters()
    {
        FlameHeight = 100.0f;
        FlameIntensity = 1.0f;
        SmokeAmount = 0.5f;
        SparkCount = 50.0f;
        FireColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
        LightRadius = 500.0f;
        LightIntensity = 2000.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CampfireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CampfireSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Fire Control
    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void StartFire(EVFX_FireIntensity Intensity = EVFX_FireIntensity::Medium);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SetFireIntensity(EVFX_FireIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void AddFuel(float FuelAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    bool IsFireActive() const { return bFireActive; }

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    float GetFuelLevel() const { return CurrentFuel; }

    // Wind Effects
    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SetWindDirection(FVector WindDir);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SetWindStrength(float Strength);

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* FlameEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SmokeEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SparkEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* EmberEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* FireAudio;

    // Niagara Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* NS_CampfireFlames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* NS_CampfireSmoke;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* NS_CampfireSparks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* NS_CampfireEmbers;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* FireCrackleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* FireIgniteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundCue* FireExtinguishSound;

    // Fire Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    FVFX_FireParameters FireParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float MaxFuel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    float FuelConsumptionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Settings")
    bool bAutoExtinguishWhenOutOfFuel = true;

    // Wind Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings")
    bool bEnableWindEffect = true;

private:
    // State
    bool bFireActive = false;
    float CurrentFuel = 50.0f;
    EVFX_FireIntensity CurrentIntensity = EVFX_FireIntensity::Medium;
    float LightFlickerTimer = 0.0f;
    float BaseLightIntensity = 2000.0f;

    // Internal Methods
    void UpdateFireEffects();
    void UpdateLightFlicker(float DeltaTime);
    void UpdateWindEffects();
    void ConsumeFuel(float DeltaTime);
    FVFX_FireParameters GetParametersForIntensity(EVFX_FireIntensity Intensity);
    void ApplyFireParameters(const FVFX_FireParameters& Params);
    void CreateComponents();
    void LoadAssets();
};