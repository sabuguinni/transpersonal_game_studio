#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Light_VolumetricFogManager.generated.h"

UENUM(BlueprintType)
enum class ELight_FogType : uint8
{
    None            UMETA(DisplayName = "None"),
    Light           UMETA(DisplayName = "Light Mist"),
    Medium          UMETA(DisplayName = "Medium Fog"),
    Heavy           UMETA(DisplayName = "Heavy Fog"),
    Volumetric      UMETA(DisplayName = "Volumetric Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float StartDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricFogEmissive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricFogExtinctionScale = 1.0f;

    FLight_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        StartDistance = 5000.0f;
        FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);
        bVolumetricFog = true;
        VolumetricFogScatteringDistribution = 0.2f;
        VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        VolumetricFogEmissive = 0.0f;
        VolumetricFogExtinctionScale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricFogManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricFogManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog System")
    ELight_FogType CurrentFogType = ELight_FogType::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog System")
    FLight_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog System")
    float FogTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    bool bEnableAtmosphericPerspective = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    float AtmosphericPerspectiveDistance = 100000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AVolumetricCloud* VolumetricCloudActor;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void SetFogType(ELight_FogType NewFogType);

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void ApplyFogSettings(const FLight_FogSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void TransitionToFogType(ELight_FogType TargetFogType, float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void CreateMorningMist();

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void CreateEveningFog();

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void CreateStormFog();

    UFUNCTION(BlueprintCallable, Category = "Fog System")
    void ClearAllFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void EnableVolumetricLightScattering(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SetAtmosphericPerspective(bool bEnable, float Distance = 100000.0f);

    UFUNCTION(BlueprintPure, Category = "Fog System")
    ELight_FogType GetCurrentFogType() const { return CurrentFogType; }

    UFUNCTION(BlueprintPure, Category = "Fog System")
    FLight_FogSettings GetCurrentFogSettings() const { return FogSettings; }

private:
    void InitializeFogComponents();
    void UpdateFogTransition(float DeltaTime);
    void ApplyFogTypeSettings(ELight_FogType FogType);
    
    // Transition variables
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 3.0f;
    ELight_FogType TargetFogType;
    FLight_FogSettings StartFogSettings;
    FLight_FogSettings TargetFogSettings;
};