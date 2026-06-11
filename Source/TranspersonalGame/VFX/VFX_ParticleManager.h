#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TranspersonalGame/SharedTypes.h"
#include "VFX_ParticleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_ParticleConfig()
    {
        ParticleSystem = nullptr;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep       UMETA(DisplayName = "Dinosaur Footstep"),
    Blood_Splatter      UMETA(DisplayName = "Blood Impact"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_Rising        UMETA(DisplayName = "Smoke"),
    Sparks_Impact       UMETA(DisplayName = "Sparks"),
    None                UMETA(DisplayName = "None")
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
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_EffectType, FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveParticles;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnParticleEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector Location, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

private:
    void InitializeParticleConfigs();
    void CleanupNullComponents();
};