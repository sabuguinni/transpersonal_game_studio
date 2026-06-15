#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepLight    UMETA(DisplayName = "Light Footstep"),
    FootstepHeavy    UMETA(DisplayName = "Heavy Footstep"),
    BloodSplatter    UMETA(DisplayName = "Blood Splatter"),
    RockImpact       UMETA(DisplayName = "Rock Impact"),
    WoodImpact       UMETA(DisplayName = "Wood Impact"),
    WaterSplash      UMETA(DisplayName = "Water Splash"),
    FireSparks       UMETA(DisplayName = "Fire Sparks"),
    DustCloud        UMETA(DisplayName = "Dust Cloud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float EffectDuration;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::FootstepLight;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactIntensity = 1.0f;
        EffectDuration = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EVFX_ImpactType, UNiagaraSystem*> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void PlayImpactEffect(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void PlayFootstepEffect(FVector Location, bool bIsHeavyFootstep = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void PlayBloodEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void PlayFireEffect(FVector Location, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void PlayWaterSplashEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Manager")
    void StopAllEffects();

private:
    void LoadDefaultEffects();
    UNiagaraComponent* CreateEffectComponent(UNiagaraSystem* NiagaraSystem, const FVector& Location, const FRotator& Rotation);
    bool IsLocationValid(const FVector& Location) const;
};