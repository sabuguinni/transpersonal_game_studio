#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvironmentalAtmosphere.generated.h"

/**
 * Environmental Atmosphere Manager for Cretaceous period ambiance
 * Manages dynamic lighting, fog, and atmospheric effects for different biomes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalAtmosphere : public AActor
{
	GENERATED_BODY()

public:
	AEnvironmentalAtmosphere();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Configure lighting for golden hour Cretaceous atmosphere */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void SetCretaceousLighting();

	/** Configure atmospheric fog for prehistoric ambiance */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void SetCretaceousFog();

	/** Update atmosphere based on time of day */
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void UpdateAtmosphereForTimeOfDay(float TimeOfDay);

protected:
	/** Reference to the main directional light (sun) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	class ADirectionalLight* SunLight;

	/** Reference to the atmospheric fog */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
	class AExponentialHeightFog* AtmosphericFog;

	/** Sun light color for different times of day */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor GoldenHourColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor MidDayColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SunsetColor;

	/** Fog density settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseFogDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogHeightFalloff;

	/** Atmospheric scattering colors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FLinearColor InscatteringColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	FLinearColor DirectionalInscatteringColor;

private:
	/** Find or create the main directional light */
	void FindOrCreateSunLight();

	/** Find or create atmospheric fog */
	void FindOrCreateAtmosphericFog();
};