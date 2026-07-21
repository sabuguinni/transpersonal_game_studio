#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "VFX_PrehistoricManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Impact_Footstep     UMETA(DisplayName = "Footstep Impact"),
    Dust_Movement       UMETA(DisplayName = "Movement Dust"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact")
};

USTRUCT(BlueprintType)
struct FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectSettings()
    {
        Intensity = 1.0f;
        Duration = 5.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_PrehistoricManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_PrehistoricManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* MainVFXComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType CurrentEffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EffectSettings EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> CampfireVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> FootstepImpactVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> DustCloudVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> RainVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodImpactVFX;

private:
    UPROPERTY()
    float EffectTimer;

    UPROPERTY()
    bool bEffectActive;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void PlayEffect(EVFX_EffectType EffectType, const FVFX_EffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetEffectScale(const FVector& NewScale);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX Testing")
    void TestCampfireEffect();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX Testing")
    void TestFootstepEffect();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX Testing")
    void TestDustEffect();

protected:
    void LoadVFXAsset(EVFX_EffectType EffectType);
    void UpdateVFXParameters();
    void HandleEffectTimer(float DeltaTime);
};