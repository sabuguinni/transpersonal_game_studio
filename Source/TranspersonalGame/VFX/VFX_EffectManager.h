#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    Fire_Campfire,
    Dust_Footstep,
    Dust_Impact,
    Smoke_Volcanic,
    Water_Spray,
    Blood_Impact,
    Debris_Destruction,
    Weather_Rain,
    Weather_Snow,
    Breath_Vapor
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor TintColor = FLinearColor::White;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Duration = 2.0f;
        Scale = 1.0f;
        TintColor = FLinearColor::White;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    UVFX_EffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Manager")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Manager")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Manager")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Manager")
    float EffectCullDistance = 5000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float ScaleOverride = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(FVector Location, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurImpactEffect(FVector Location, float DinosaurMass = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectQuality(int32 QualityLevel);

private:
    void InitializeEffectDatabase();
    void CullDistantEffects();
    EVFX_EffectType GetFootstepEffectType(ESurfaceType SurfaceType);
};