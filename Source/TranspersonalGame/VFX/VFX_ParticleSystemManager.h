#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    CampfireFlames      UMETA(DisplayName = "Campfire Flames"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    RainDrops           UMETA(DisplayName = "Rain Drops"),
    VolcanicAsh         UMETA(DisplayName = "Volcanic Ash"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    LeafFall            UMETA(DisplayName = "Leaf Fall")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::DustCloud;
        Duration = 2.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Niagara systems para diferentes tipos de partículas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FootstepParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> CampfireParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> RainParticleSystem;

    // Array de componentes Niagara activos
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveParticleSystems;

    // Configurações globais de VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bVFXEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GlobalVFXScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float GlobalVFXIntensity = 1.0f;

public:
    // Função principal para spawnar efeitos de partículas
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnParticleEffect(
        EVFX_ParticleType ParticleType,
        const FVector& Location,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVFX_ParticleConfig& Config = FVFX_ParticleConfig()
    );

    // Funções específicas para diferentes tipos de VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void CreateDinosaurFootstepEffect(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateCampfireEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void CreateBloodSplatterEffect(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateDustCloudEffect(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void CreateRainEffect(const FVector& Location, float Radius = 1000.0f);

    // Gestão de sistemas activos
    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    int32 GetActiveEffectCount() const;

    // Configurações globais
    UFUNCTION(BlueprintCallable, Category = "VFX|Settings")
    void SetVFXEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX|Settings")
    void SetGlobalVFXScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "VFX|Settings")
    void SetGlobalVFXIntensity(float Intensity);

private:
    // Função helper para carregar sistema Niagara
    UNiagaraSystem* LoadParticleSystem(EVFX_ParticleType ParticleType);

    // Timer para limpeza automática
    FTimerHandle CleanupTimerHandle;

    // Função de limpeza automática
    void AutoCleanupEffects();
};