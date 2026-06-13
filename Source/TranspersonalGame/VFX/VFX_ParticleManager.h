#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    DinosaurDust    UMETA(DisplayName = "Dinosaur Dust"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    BreathVapor     UMETA(DisplayName = "Breath Vapor"),
    WeatherRain     UMETA(DisplayName = "Weather Rain"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    InsectSwarm     UMETA(DisplayName = "Insect Swarm")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_ParticleData()
    {
        NiagaraSystem = nullptr;
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UVFX_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TMap<EVFX_ParticleType, FVFX_ParticleData> ParticleDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveParticles;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurDustEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatterEffect(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplashEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVaporEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherRainEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVolcanicAshEffect(FVector Location, FVector WindDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnInsectSwarmEffect(FVector Location, float SwarmSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveParticleCount() const;

private:
    void InitializeParticleDatabase();
    void RemoveNullParticles();
};