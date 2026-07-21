#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    FootstepDust,
    BloodSplatter,
    AmbientPollen,
    CampfireSparks,
    RainDroplets,
    BreathVapor,
    ImpactDebris
};

USTRUCT(BlueprintType)
struct FVFX_ParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup = (VFX))
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_ParticleType, TSoftObjectPtr<UNiagaraSystem>> ParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_ParticleType, FVFX_ParticleSettings> ParticleSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveParticles;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector ImpactLocation, EDamageType DamageType, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnAmbientParticles(FVector CenterLocation, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateDayNightParticles(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredParticles();

    UFUNCTION(CallInEditor, Category = "VFX Debug")
    void TestAllParticleTypes();

protected:
    void InitializeParticleSystems();
    void ConfigureParticleSettings();
    UNiagaraComponent* CreateParticleComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation);

private:
    FTimerHandle CleanupTimer;
    float AmbientParticleIntensity = 1.0f;
};