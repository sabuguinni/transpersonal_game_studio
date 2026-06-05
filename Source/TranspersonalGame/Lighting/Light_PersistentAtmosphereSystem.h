#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Light_PersistentAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULight_PersistentAtmosphereSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULight_PersistentAtmosphereSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FixAtmosphereOnMapLoad();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SaveAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void LoadAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void RemoveProblematicAtmosphereActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateOptimalAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetAtmosphereSettings() const { return CurrentSettings; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphereSettings CurrentSettings;

    UPROPERTY()
    TWeakObjectPtr<ADirectionalLight> MainSunLight;

    UPROPERTY()
    TWeakObjectPtr<AExponentialHeightFog> MainFog;

    UPROPERTY()
    TWeakObjectPtr<ASkyAtmosphere> MainAtmosphere;

    void FindOrCreateAtmosphereActors();
    void ConfigureDirectionalLight(ADirectionalLight* Light);
    void ConfigureHeightFog(AExponentialHeightFog* Fog);
    void ConfigureSkyAtmosphere(ASkyAtmosphere* Atmosphere);

    UFUNCTION()
    void OnMapChanged();

    FDelegateHandle MapChangedHandle;
};