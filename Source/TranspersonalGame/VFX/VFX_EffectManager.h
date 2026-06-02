#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        NiagaraEffect = nullptr;
        EffectScale = FVector(1.0f);
        EffectDuration = 2.0f;
        bAutoDestroy = true;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_Cooking       UMETA(DisplayName = "Cooking Smoke"),
    Rain_Weather        UMETA(DisplayName = "Rain"),
    Wind_Particles      UMETA(DisplayName = "Wind Particles"),
    Sparks_Crafting     UMETA(DisplayName = "Crafting Sparks")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector FootLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodImpact(FVector ImpactLocation, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(FVector WaterLocation, float SplashIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX Debug")
    void TestAllEffects();

private:
    void InitializeEffectDatabase();
    void CleanupEffect(UNiagaraComponent* EffectComponent);

    UPROPERTY()
    float EffectCleanupTimer = 0.0f;
};