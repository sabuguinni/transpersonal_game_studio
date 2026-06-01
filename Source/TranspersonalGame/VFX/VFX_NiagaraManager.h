#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    FootstepDust,
    CampfireFire,
    BloodImpact,
    WeatherRain,
    WeaponSpark,
    BreathVapor
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Duration = 2.0f;
        Scale = 1.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Effect Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectRegistry;

    // Active VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Active")
    TArray<UNiagaraComponent*> ActiveEffects;

    // VFX Spawning Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Prehistoric-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Prehistoric")
    void PlayFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Prehistoric")
    void PlayCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Prehistoric")
    void PlayBloodImpact(FVector Location, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX Prehistoric")
    void PlayWeatherRain(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Prehistoric")
    void PlayBreathVapor(FVector Location, float Temperature);

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void RegisterEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

private:
    // Internal management
    void InitializeEffectRegistry();
    void UpdateActiveEffects(float DeltaTime);
    
    // Cleanup timer
    float CleanupTimer = 0.0f;
    float CleanupInterval = 5.0f;
};