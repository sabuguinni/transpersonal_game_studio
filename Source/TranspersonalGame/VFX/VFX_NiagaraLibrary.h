#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Dust_Impact         UMETA(DisplayName = "Impact Dust"),
    Blood_Splatter      UMETA(DisplayName = "Blood Splatter"),
    Blood_Drip          UMETA(DisplayName = "Blood Drip"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Breath_Vapor        UMETA(DisplayName = "Breath Vapor"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Sparks_Tool         UMETA(DisplayName = "Tool Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Intensity = 1.0f;
        Scale = FVector::OneVector;
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

    // VFX Effect Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectIntensity(UNiagaraComponent* EffectComponent, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectScale(UNiagaraComponent* EffectComponent, FVector Scale);

    // Prehistoric Environment Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateWaterEffect(FVector Location, float FlowRate = 1.0f);

    // Dinosaur Interaction Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateBreathVapor(FVector Location, float Temperature = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* CreateBloodEffect(FVector Location, EVFX_EffectType BloodType = EVFX_EffectType::Blood_Splatter);

    // Player Interaction Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    UNiagaraComponent* CreateCraftingSparks(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    UNiagaraComponent* CreateImpactEffect(FVector Location, EVFX_EffectType ImpactType = EVFX_EffectType::Dust_Impact);

protected:
    // VFX Library Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

private:
    void InitializeEffectLibrary();
    void CleanupExpiredEffects();
    UNiagaraSystem* GetEffectAsset(EVFX_EffectType EffectType);
    void ApplyLODSettings(UNiagaraComponent* EffectComponent, float Distance);
};