#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Light_VolumetricEffects.generated.h"

UENUM(BlueprintType)
enum class ELight_VolumetricType : uint8
{
    Sunbeams,
    Firelight,
    Moonbeams,
    Mist,
    Smoke,
    Dust
};

USTRUCT(BlueprintType)
struct FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float Density = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float ScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bCastVolumetricShadows = true;

    FLight_VolumetricSettings()
    {
        Intensity = 1.0f;
        Color = FLinearColor::White;
        Density = 0.5f;
        ScatteringDistribution = 0.2f;
        bCastVolumetricShadows = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricEffects : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VolumeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* VolumetricLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    ELight_VolumetricType VolumetricType = ELight_VolumetricType::Sunbeams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricSettings VolumetricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bAnimateIntensity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float IntensityVariation = 0.2f;

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void SetVolumetricType(ELight_VolumetricType NewType);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void CreateSunbeamEffect(FVector SunDirection, float BeamLength = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void CreateFirelightEffect(float FlickerIntensity = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void CreateAtmosphericMist(float MistDensity = 0.1f);

private:
    void UpdateVolumetricEffect();
    void AnimateVolumetricIntensity(float DeltaTime);
    
    float BaseIntensity;
    float AnimationTime;
};