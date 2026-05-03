#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFXSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXImpactType ImpactType;

    FVFX_ImpactData()
    {
        Location = FVector::ZeroVector;
        Normal = FVector::UpVector;
        Intensity = 1.0f;
        ImpactType = EVFXImpactType::Ground;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFXWeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bIsActive;

    FVFX_WeatherData()
    {
        WeatherType = EVFXWeatherType::Clear;
        Intensity = 0.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Sistemas de partículas Niagara
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* BloodImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* BreathVaporSystem;

    // Configurações de VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ParticleDistanceCulling;

    // Estado actual do tempo
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FVFX_WeatherData CurrentWeather;

public:
    // Métodos principais de VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactVFX(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepVFX(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodVFX(FVector Location, FVector Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherVFX(const FVFX_WeatherData& WeatherData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVFX(FVector Location, float Temperature);

    // Gestão de sistemas
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(EVFXQualityLevel QualityLevel);

protected:
    // Métodos internos
    void UpdateWeatherVFX(float DeltaTime);
    void UpdateParticleLOD();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

private:
    // Cache de componentes activos
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;
    
    // Timers para limpeza
    float CleanupTimer;
    const float CleanupInterval = 5.0f;
};