#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn"),
	Morning		UMETA(DisplayName = "Morning"), 
	Midday		UMETA(DisplayName = "Midday"),
	Afternoon	UMETA(DisplayName = "Afternoon"),
	Dusk		UMETA(DisplayName = "Dusk"),
	Night		UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Cloudy		UMETA(DisplayName = "Cloudy"),
	Overcast	UMETA(DisplayName = "Overcast"),
	Foggy		UMETA(DisplayName = "Foggy"),
	Stormy		UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SunIntensity = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SunAngle = -45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SkyIntensity = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float FogDensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

	FLight_TimeSettings()
	{
		SunIntensity = 5.0f;
		SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
		SunAngle = -45.0f;
		SkyIntensity = 1.5f;
		SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
		FogDensity = 0.02f;
		FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
	}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_SystemManager : public AActor
{
	GENERATED_BODY()

public:
	ALight_SystemManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Time of day system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
	float DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
	float CurrentTime = 6.0f; // Start at 6 AM

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
	bool bAutoAdvanceTime = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time System")
	ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

	// Weather system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float WeatherTransitionSpeed = 1.0f;

	// Lighting references
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
	ADirectionalLight* SunLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
	ASkyLight* SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
	AExponentialHeightFog* AtmosphericFog;

	// Time of day presets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings DawnSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings MorningSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings MiddaySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings AfternoonSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings DuskSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
	FLight_TimeSettings NightSettings;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Time System")
	void SetTimeOfDay(float NewTime);

	UFUNCTION(BlueprintCallable, Category = "Time System")
	void SetTimeOfDayInstant(ELight_TimeOfDay TimeOfDay);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(ELight_WeatherType NewWeather);

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void UpdateLighting();

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void FindLightingActors();

	UFUNCTION(BlueprintPure, Category = "Time System")
	FString GetTimeString() const;

	UFUNCTION(BlueprintPure, Category = "Time System")
	float GetNormalizedTime() const;

private:
	void UpdateTimeOfDay();
	void InterpolateLighting(const FLight_TimeSettings& FromSettings, const FLight_TimeSettings& ToSettings, float Alpha);
	FLight_TimeSettings GetCurrentTimeSettings() const;
	ELight_TimeOfDay CalculateTimeOfDay(float Time) const;
	void ApplyWeatherEffects();
};