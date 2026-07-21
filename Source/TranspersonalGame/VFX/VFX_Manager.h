#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_Manager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    FVFX_EffectData()
    {
        EffectName = TEXT("DefaultEffect");
        Location = FVector::ZeroVector;
        Duration = 2.0f;
        Intensity = 1.0f;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire         UMETA(DisplayName = "Fire"),
    Dust         UMETA(DisplayName = "Dust"),
    Blood        UMETA(DisplayName = "Blood"),
    Water        UMETA(DisplayName = "Water"),
    Smoke        UMETA(DisplayName = "Smoke"),
    Impact       UMETA(DisplayName = "Impact"),
    Weather      UMETA(DisplayName = "Weather")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_Manager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustCloud(FVector Location, float Radius = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage = 1000.0f);

    // Effect Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(FString EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // VFX Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* FireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* WaterEffect;

    // Active Effects Tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    TArray<FVFX_EffectData> ActiveEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveComponents;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFXLOD;

private:
    // Internal Methods
    void CleanupExpiredEffects();
    void LoadVFXAssets();
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* System, FVector Location);
    void SetEffectLOD(UNiagaraComponent* Component, float Distance);

    // Timers
    FTimerHandle CleanupTimer;
};