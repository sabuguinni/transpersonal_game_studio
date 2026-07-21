#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    DustImpact      UMETA(DisplayName = "Dust Impact"),
    BloodImpact     UMETA(DisplayName = "Blood Impact"),
    Rain            UMETA(DisplayName = "Rain"),
    DinosaurBreath  UMETA(DisplayName = "Dinosaur Breath"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    InsectSwarm     UMETA(DisplayName = "Insect Swarm")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleSystemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_ParticleType ParticleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifeSpan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate;

    FVFX_ParticleSystemData()
    {
        ParticleType = EVFX_ParticleType::Campfire;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        LifeSpan = 0.0f;
        bAutoActivate = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_ParticleSystemData> ParticleSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveNiagaraComponents;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopParticleEffect(EVFX_ParticleType ParticleType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsParticleEffectActive(EVFX_ParticleType ParticleType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetParticleSystemData(const TArray<FVFX_ParticleSystemData>& NewParticleSystemData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    TArray<FVFX_ParticleSystemData> GetParticleSystemData() const;

protected:
    UFUNCTION()
    void InitializeParticleSystems();

    UFUNCTION()
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem);

    UFUNCTION()
    void CleanupInactiveComponents();
};