#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Impact_Blood        UMETA(DisplayName = "Blood Impact"),
    Impact_Dust         UMETA(DisplayName = "Dust Impact"),
    Impact_Rock         UMETA(DisplayName = "Rock Impact"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Dino_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Dino_Roar           UMETA(DisplayName = "Dinosaur Roar Effect"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Water_Ripple        UMETA(DisplayName = "Water Ripple"),
    Ambient_Insects     UMETA(DisplayName = "Ambient Insects"),
    Ambient_Pollen      UMETA(DisplayName = "Pollen Particles")
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy;

    FVFX_EffectConfig()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAdvanced(const FVFX_EffectConfig& Config, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool LoadEffectAssets();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraSystem* GetEffectAsset(EVFX_EffectType EffectType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectAssets;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DefaultEffectDuration;

private:
    void InitializeEffectAssets();
    void CleanupExpiredEffects();
    UNiagaraComponent* CreateEffectComponent(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation);
};