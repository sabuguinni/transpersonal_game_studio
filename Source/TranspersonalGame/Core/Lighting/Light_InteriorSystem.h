#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "Light_InteriorSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_InteriorType : uint8
{
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    TribalHut       UMETA(DisplayName = "Tribal Hut"),
    SacredSpace     UMETA(DisplayName = "Sacred Space")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FireConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float MainIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float RimIntensity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLinearColor FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FireTemperature = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    bool bEnableFlickering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerIntensity = 0.3f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float EntranceLightIntensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor EntranceLightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float EntranceTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DepthFillIntensity = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor DepthFillColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricDensity = 0.1f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_InteriorSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_InteriorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MainFireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* RimFireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpotLightComponent* EntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    URectLightComponent* DepthFillLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    ELight_InteriorType InteriorType = ELight_InteriorType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLight_FireConfiguration FireConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLight_AtmosphereConfiguration AtmosphereConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    bool bAutoConfigureForInteriorType = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float InteriorRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float CeilingHeight = 300.0f;

private:
    float FlickerTimer = 0.0f;
    float BaseFireIntensity = 1500.0f;
    float BaseRimIntensity = 600.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetInteriorType(ELight_InteriorType NewType);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void ConfigureLightingForType();

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetFireConfiguration(const FLight_FireConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetAtmosphereConfiguration(const FLight_AtmosphereConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void EnableFlickering(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetFlickerParameters(float Speed, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void UpdateFireIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetDayNightCycle(float TimeOfDay); // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetWeatherInfluence(float RainIntensity, float WindStrength);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior Lighting")
    void OnPlayerEnterInterior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interior Lighting")
    void OnPlayerExitInterior();
};