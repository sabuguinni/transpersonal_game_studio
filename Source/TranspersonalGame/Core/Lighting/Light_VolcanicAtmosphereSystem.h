#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Light_VolcanicAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolcanicLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float LavaGlowIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    FLinearColor LavaColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float AttenuationRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float FogDensity = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolcanicAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolcanicAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MainVolcanicGlow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* LavaPitLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpotLightComponent* CaveEntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UExponentialHeightFogComponent* VolcanicFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    FLight_VolcanicLightingConfig LightingConfig;

public:
    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void SetupVolcanicLighting();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void UpdateLavaIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void SetVolcanicFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void EnableVolcanicAtmosphere(bool bEnable);

    virtual void Tick(float DeltaTime) override;

private:
    float FlickerTimer = 0.0f;
    float BaseIntensity = 2000.0f;
};