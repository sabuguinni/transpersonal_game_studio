#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "VFX_EffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    DustImpact,
    Campfire,
    Rain,
    Mist,
    Sandstorm,
    DinosaurFootstep,
    DinosaurBreath,
    AmbientPollen,
    AmbientInsects,
    BloodSplatter,
    WeaponImpact
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::DustImpact;
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Duration = 5.0f;
        Intensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TArray<FVFX_EffectData> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnableDinosaurVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnableAmbientVFX;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FVector Scale = FVector::OneVector, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstepVFX(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireVFX(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherVFX(EVFX_EffectType WeatherType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatterVFX(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeaponImpactVFX(FVector Location, FVector Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ClearAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherEffectsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetDinosaurVFXEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

protected:
    UFUNCTION()
    void CleanupExpiredEffects();

    UFUNCTION()
    void UpdateEffectLOD();

    UFUNCTION()
    bool IsEffectInRange(const FVFX_EffectData& Effect) const;

private:
    float EffectCleanupTimer;
    float LODUpdateTimer;
};