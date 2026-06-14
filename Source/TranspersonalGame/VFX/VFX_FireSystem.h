#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/StaticMeshActor.h"
#include "VFX_FireSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_FireType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    Torch           UMETA(DisplayName = "Torch"),
    WildFire        UMETA(DisplayName = "Wild Fire"),
    CookingFire     UMETA(DisplayName = "Cooking Fire")
};

USTRUCT(BlueprintType)
struct FVFX_FireParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    float FlameIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    float SmokeAmount = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    float EmberCount = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    FLinearColor FlameColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    bool bCastLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire VFX")
    float LightRadius = 500.0f;

    FVFX_FireParams()
    {
        FlameIntensity = 1.0f;
        SmokeAmount = 0.8f;
        EmberCount = 50.0f;
        FlameColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
        bCastLight = true;
        LightRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FireSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Fire control functions
    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void StartFire(EVFX_FireType FireType, const FVFX_FireParams& Params);

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    void SetFireIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Fire")
    bool IsFireActive() const { return bFireActive; }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    EVFX_FireType CurrentFireType = EVFX_FireType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    FVFX_FireParams FireParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    TSoftObjectPtr<UNiagaraSystem> FlameEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    TSoftObjectPtr<UNiagaraSystem> SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Fire")
    TSoftObjectPtr<UNiagaraSystem> EmberEffect;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* FlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* SmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* EmberComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UPointLightComponent* FireLight;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    bool bFireActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float CurrentIntensity = 1.0f;

    // Internal functions
    void UpdateFireEffects();
    void SetupFireLight();
    void LoadFireAssets();
};

#include "VFX_FireSystem.generated.h"