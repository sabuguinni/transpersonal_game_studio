#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Light_VolumetricController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    bool bEnableVolumetricFog = true;
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolumetricController : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolumetricController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Volumetric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricSettings StormSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Settings")
    FLight_VolumetricSettings ClearSettings;

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EWeatherType CurrentWeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float TransitionSpeed = 1.0f;

    // References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    UExponentialHeightFogComponent* FogComponent;

private:
    FLight_VolumetricSettings CurrentSettings;
    FLight_VolumetricSettings TargetSettings;
    bool bTransitioning = false;
    float TransitionProgress = 0.0f;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Volumetrics")
    void SetWeatherType(EWeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Volumetrics")
    void ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Volumetrics")
    void TransitionToSettings(const FLight_VolumetricSettings& NewSettings, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyCretaceousVolumetrics();

    UFUNCTION(BlueprintCallable, Category = "Volumetrics")
    FLight_VolumetricSettings GetCurrentSettings() const { return CurrentSettings; }

private:
    void FindOrCreateFogActor();
    void InitializeDefaultSettings();
    void UpdateVolumetricTransition(float DeltaTime);
    FLight_VolumetricSettings LerpSettings(const FLight_VolumetricSettings& A, const FLight_VolumetricSettings& B, float Alpha);
};