#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire      UMETA(DisplayName = "Campfire"),
    Dust_Footstep      UMETA(DisplayName = "Dinosaur Footstep"),
    Weather_Rain       UMETA(DisplayName = "Rain"),
    Blood_Impact       UMETA(DisplayName = "Blood Impact"),
    Water_Splash       UMETA(DisplayName = "Water Splash"),
    Smoke_Cooking      UMETA(DisplayName = "Cooking Smoke"),
    Sparks_Crafting    UMETA(DisplayName = "Crafting Sparks")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        DefaultScale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 5.0f;
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TArray<FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnFootstepDust(FVector Location, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnRainEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnBloodImpact(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnWaterSplash(FVector Location, float SplashSize = 1.0f);

protected:
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);
    
    void InitializeEffectDatabase();
    
    void CleanupFinishedEffects();

public:
    virtual void Tick(float DeltaTime) override;
};