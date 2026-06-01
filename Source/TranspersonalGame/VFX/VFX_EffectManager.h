#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_EffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bIsActive;

    FVFX_EffectData()
    {
        EffectName = TEXT("Default");
        Location = FVector::ZeroVector;
        Duration = 5.0f;
        Intensity = 1.0f;
        bIsActive = false;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DinosaurFootstep,
    CampfireFire,
    WeatherRain,
    WeatherFog,
    VolcanicAsh,
    BloodSplatter,
    DustCloud,
    WaterSplash,
    RockImpact,
    VegetationRustle
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TArray<FVFX_EffectData> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableEnvironmentalVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDinosaurVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableWeatherVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float EffectCullingDistance;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateDinosaurFootstepEffect(FVector Location, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateCampfireEffect(FVector Location, float FireIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateBloodEffect(FVector Location, FVector ImpactDirection, float Amount);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateRockImpactEffect(FVector Location, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(const FString& EffectName);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalVFXIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    bool IsEffectActive(const FString& EffectName) const;

protected:
    void UpdateEffects(float DeltaTime);
    void CullDistantEffects();
    void ManagePerformance();
    
    FVFX_EffectData* FindEffect(const FString& EffectName);
    void RemoveEffect(const FString& EffectName);
    void AddEffect(const FVFX_EffectData& NewEffect);
};