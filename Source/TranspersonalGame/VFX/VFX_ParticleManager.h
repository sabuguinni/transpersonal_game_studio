#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    RainDrops       UMETA(DisplayName = "Rain Drops"),
    BreathVapor     UMETA(DisplayName = "Breath Vapor"),
    Sparks          UMETA(DisplayName = "Sparks"),
    Insects         UMETA(DisplayName = "Insects"),
    WaterSpray      UMETA(DisplayName = "Water Spray")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = -1.0f; // -1 = infinite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoActivate = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Campfire;
        Intensity = 1.0f;
        Duration = -1.0f;
        bAutoActivate = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX System")
    TArray<FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX System")
    TMap<EVFX_ParticleType, UNiagaraComponent*> ActiveParticles;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float CustomIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void StopParticleEffect(EVFX_ParticleType ParticleType);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void StopAllParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    bool IsParticleEffectActive(EVFX_ParticleType ParticleType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SetParticleIntensity(EVFX_ParticleType ParticleType, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    UNiagaraComponent* GetParticleComponent(EVFX_ParticleType ParticleType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void InitializeParticleSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX System", CallInEditor)
    void RefreshParticleConfigs();

private:
    void LoadDefaultParticleConfigs();
    UNiagaraComponent* CreateParticleComponent(const FVFX_ParticleConfig& Config);
};