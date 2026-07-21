#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "EnvArt_VolumetricEffects.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_EffectType : uint8
{
    Dust UMETA(DisplayName = "Dust Particles"),
    Pollen UMETA(DisplayName = "Pollen"),
    Mist UMETA(DisplayName = "Forest Mist"),
    Fireflies UMETA(DisplayName = "Fireflies"),
    Leaves UMETA(DisplayName = "Falling Leaves"),
    Spores UMETA(DisplayName = "Spores")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EEnvArt_EffectType EffectType = EEnvArt_EffectType::Dust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FVector EffectScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bAffectedByWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float WindStrength = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_VolumetricEffects : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_VolumetricEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EffectBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    FEnvArt_EffectSettings EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings")
    bool bPlayerProximityActivation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Settings", meta = (EditCondition = "bPlayerProximityActivation"))
    float ActivationDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bTimeOfDayAffected = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float NightIntensityMultiplier = 0.3f;

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SetEffectType(EEnvArt_EffectType NewType);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ActivateEffect();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void DeactivateEffect();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateDustEffect();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreatePollenEffect();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void CreateMistEffect();

private:
    void UpdateEffectParameters();
    void CheckPlayerProximity();
    float GetTimeOfDayMultiplier();

    UPROPERTY()
    class APlayerController* PlayerController;

    bool bIsActive = false;
    float TimeAccumulator = 0.0f;
};