#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFX_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire        UMETA(DisplayName = "Fire"),
    Dust        UMETA(DisplayName = "Dust"),
    Blood       UMETA(DisplayName = "Blood"),
    Weather     UMETA(DisplayName = "Weather"),
    Breath      UMETA(DisplayName = "Breath"),
    Impact      UMETA(DisplayName = "Impact"),
    Explosion   UMETA(DisplayName = "Explosion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire;
        DefaultScale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 5.0f;
        bAutoActivate = false;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TArray<FVFX_EffectData> EffectLibrary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachComponent, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    // Preset effect spawners for common scenarios
    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnDustImpact(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnBloodSplatter(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    UNiagaraComponent* SpawnDinosaurBreath(USceneComponent* AttachComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void StartWeatherEffect(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void StopWeatherEffect();

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);

protected:
    // Internal helper functions
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem);
    void InitializeEffectLibrary();
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);

private:
    UPROPERTY()
    UNiagaraComponent* WeatherEffectComponent;

public:
    virtual void Tick(float DeltaTime) override;
};