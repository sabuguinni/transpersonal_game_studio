#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "SharedTypes.h"
#include "Light_AtmosphericLightingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float SunIntensity = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float SunTemperature = 5400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float FogDensity = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float VolumetricScattering = 1.5f;

	FLight_TimeOfDaySettings()
	{
		SunIntensity = 4.0f;
		SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
		SunTemperature = 5400.0f;
		FogDensity = 0.025f;
		FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
		VolumetricScattering = 1.5f;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float CloudCoverage = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WindStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float AtmosphericPressure = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FLinearColor WeatherTint = FLinearColor::White;

	FLight_WeatherSettings()
	{
		CloudCoverage = 0.3f;
		RainIntensity = 0.0f;
		WindStrength = 0.5f;
		AtmosphericPressure = 1.0f;
		WeatherTint = FLinearColor::White;
	}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericLightingManager : public AActor
{
	GENERATED_BODY()

public:
	ALight_AtmosphericLightingManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Core lighting components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
	class USceneComponent* RootSceneComponent;

	// Time of day system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float CurrentTimeOfDay = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float DayDurationMinutes = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	bool bAutoAdvanceTime = true;

	// Lighting settings for different times
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
	FLight_TimeOfDaySettings DawnSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
	FLight_TimeOfDaySettings NoonSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
	FLight_TimeOfDaySettings DuskSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
	FLight_TimeOfDaySettings NightSettings;

	// Weather system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FLight_WeatherSettings CurrentWeather;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherTransitionSpeed = 1.0f;

	// Atmospheric effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bEnableVolumetricFog = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bEnableGodRays = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bEnableAtmosphericScattering = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float AtmosphericDensity = 1.0f;

	// Light references
	UPROPERTY(BlueprintReadOnly, Category = "Light References")
	ADirectionalLight* SunLight;

	UPROPERTY(BlueprintReadOnly, Category = "Light References")
	ASkyLight* SkyLight;

	UPROPERTY(BlueprintReadOnly, Category = "Light References")
	AExponentialHeightFog* AtmosphericFog;

	UPROPERTY(BlueprintReadOnly, Category = "Light References")
	ASkyAtmosphere* SkyAtmosphere;

public:
	// Time of day functions
	UFUNCTION(BlueprintCallable, Category = "Time of Day")
	void SetTimeOfDay(float NewTime);

	UFUNCTION(BlueprintCallable, Category = "Time of Day")
	float GetTimeOfDay() const { return CurrentTimeOfDay; }

	UFUNCTION(BlueprintCallable, Category = "Time of Day")
	void SetDayDuration(float Minutes);

	UFUNCTION(BlueprintCallable, Category = "Time of Day")
	void PauseTimeAdvancement();

	UFUNCTION(BlueprintCallable, Category = "Time of Day")
	void ResumeTimeAdvancement();

	// Weather functions
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherSettings(const FLight_WeatherSettings& NewWeather);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void TransitionToWeather(const FLight_WeatherSettings& TargetWeather, float TransitionTime);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	FLight_WeatherSettings GetCurrentWeather() const { return CurrentWeather; }

	// Lighting control functions
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void UpdateLightingForTimeOfDay();

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetSunIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetSunColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetFogDensity(float Density);

	// Atmospheric functions
	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void EnableVolumetricFog(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void EnableGodRays(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Atmosphere")
	void SetAtmosphericDensity(float Density);

	// Utility functions
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void FindAndCacheLightingActors();

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void ApplyPrehistoricAtmosphere();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
	void SetupDefaultLighting();

private:
	// Internal update functions
	void UpdateTimeOfDay(float DeltaTime);
	void UpdateWeatherTransition(float DeltaTime);
	void UpdateSunPosition();
	void UpdateFogSettings();
	void UpdateSkyAtmosphere();
	
	// Interpolation helpers
	FLight_TimeOfDaySettings InterpolateTimeSettings(float TimeOfDay);
	FLinearColor InterpolateColor(const FLinearColor& A, const FLinearColor& B, float Alpha);
	float InterpolateFloat(float A, float B, float Alpha);

	// Weather transition state
	bool bIsTransitioningWeather = false;
	FLight_WeatherSettings TargetWeatherSettings;
	FLight_WeatherSettings StartWeatherSettings;
	float WeatherTransitionProgress = 0.0f;
	float WeatherTransitionDuration = 5.0f;
};