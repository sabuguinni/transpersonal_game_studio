#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_CretaceousVolcanicSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_VolcanicIntensity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    LowActivity UMETA(DisplayName = "Low Activity"),
    Moderate    UMETA(DisplayName = "Moderate"),
    HighActivity UMETA(DisplayName = "High Activity"),
    Erupting    UMETA(DisplayName = "Erupting")
};

USTRUCT(BlueprintType)
struct FVFX_VolcanicEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TSoftObjectPtr<UNiagaraSystem> LavaFlowSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TSoftObjectPtr<UNiagaraSystem> ThermalVentSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    float EffectIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    float EffectRadius = 2000.0f;

    FVFX_VolcanicEffectData()
    {
        EffectIntensity = 1.0f;
        EffectRadius = 2000.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CretaceousVolcanicSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CretaceousVolcanicSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Volcanic system control
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SetVolcanicIntensity(EVFX_VolcanicIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void TriggerLavaFlow(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void ActivateThermalVent(const FVector& VentLocation, float SteamIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void TriggerVolcanicAsh(const FVector& EruptionCenter, float AshRadius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void StartVolcanicEruption();

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void StopVolcanicEruption();

    // Environmental interaction
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    bool IsLocationAffectedByVolcanicActivity(const FVector& TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    float GetVolcanicHeatAtLocation(const FVector& TestLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    EVFX_VolcanicIntensity CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    FVFX_VolcanicEffectData VolcanicEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> LavaFlowComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> ThermalVentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNiagaraComponent> VolcanicAshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float VolcanicUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float MaxVolcanicRange = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    bool bIsErupting = false;

private:
    FTimerHandle VolcanicUpdateTimer;
    TArray<TObjectPtr<UNiagaraComponent>> ActiveVolcanicEffects;

    void UpdateVolcanicEffects();
    void InitializeVolcanicSystems();
    void CleanupVolcanicEffects();
    UNiagaraComponent* CreateVolcanicEffect(UNiagaraSystem* System, const FVector& Location);
};