#include "Light_AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericLightingManager::ALight_AtmosphericLightingManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// Initialize default time of day settings
	DawnSettings.SunIntensity = 2.0f;
	DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
	DawnSettings.SunTemperature = 3200.0f;
	DawnSettings.FogDensity = 0.04f;
	DawnSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
	DawnSettings.VolumetricScattering = 2.0f;

	NoonSettings.SunIntensity = 5.0f;
	NoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
	NoonSettings.SunTemperature = 5800.0f;
	NoonSettings.FogDensity = 0.015f;
	NoonSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
	NoonSettings.VolumetricScattering = 1.2f;

	DuskSettings.SunIntensity = 1.5f;
	DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
	DuskSettings.SunTemperature = 2800.0f;
	DuskSettings.FogDensity = 0.05f;
	DuskSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
	DuskSettings.VolumetricScattering = 2.5f;

	NightSettings.SunIntensity = 0.1f;
	NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
	NightSettings.SunTemperature = 4000.0f;
	NightSettings.FogDensity = 0.02f;
	NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
	NightSettings.VolumetricScattering = 1.0f;

	// Initialize weather settings
	CurrentWeather.CloudCoverage = 0.3f;
	CurrentWeather.RainIntensity = 0.0f;
	CurrentWeather.WindStrength = 0.5f;
	CurrentWeather.AtmosphericPressure = 1.0f;
	CurrentWeather.WeatherTint = FLinearColor::White;

	// Initialize atmospheric settings
	bEnableVolumetricFog = true;
	bEnableGodRays = true;
	bEnableAtmosphericScattering = true;
	AtmosphericDensity = 1.0f;

	// Initialize light references to null
	SunLight = nullptr;
	SkyLight = nullptr;
	AtmosphericFog = nullptr;
	SkyAtmosphere = nullptr;
}

void ALight_AtmosphericLightingManager::BeginPlay()
{
	Super::BeginPlay();

	// Find and cache lighting actors in the world
	FindAndCacheLightingActors();

	// Apply initial prehistoric atmosphere settings
	ApplyPrehistoricAtmosphere();

	// Update lighting for current time of day
	UpdateLightingForTimeOfDay();
}

void ALight_AtmosphericLightingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update time of day if auto-advance is enabled
	if (bAutoAdvanceTime)
	{
		UpdateTimeOfDay(DeltaTime);
	}

	// Update weather transitions
	if (bIsTransitioningWeather)
	{
		UpdateWeatherTransition(DeltaTime);
	}

	// Update lighting based on current time and weather
	UpdateLightingForTimeOfDay();
}

void ALight_AtmosphericLightingManager::SetTimeOfDay(float NewTime)
{
	CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
	if (CurrentTimeOfDay < 0.0f)
	{
		CurrentTimeOfDay += 24.0f;
	}
	
	UpdateLightingForTimeOfDay();
	UpdateSunPosition();
}

void ALight_AtmosphericLightingManager::SetDayDuration(float Minutes)
{
	DayDurationMinutes = FMath::Max(Minutes, 1.0f);
}

void ALight_AtmosphericLightingManager::PauseTimeAdvancement()
{
	bAutoAdvanceTime = false;
}

void ALight_AtmosphericLightingManager::ResumeTimeAdvancement()
{
	bAutoAdvanceTime = true;
}

void ALight_AtmosphericLightingManager::SetWeatherSettings(const FLight_WeatherSettings& NewWeather)
{
	CurrentWeather = NewWeather;
	bIsTransitioningWeather = false;
}

void ALight_AtmosphericLightingManager::TransitionToWeather(const FLight_WeatherSettings& TargetWeather, float TransitionTime)
{
	if (TransitionTime <= 0.0f)
	{
		SetWeatherSettings(TargetWeather);
		return;
	}

	StartWeatherSettings = CurrentWeather;
	TargetWeatherSettings = TargetWeather;
	WeatherTransitionDuration = TransitionTime;
	WeatherTransitionProgress = 0.0f;
	bIsTransitioningWeather = true;
}

void ALight_AtmosphericLightingManager::UpdateLightingForTimeOfDay()
{
	FLight_TimeOfDaySettings CurrentSettings = InterpolateTimeSettings(CurrentTimeOfDay);

	// Update sun light
	if (SunLight && SunLight->GetLightComponent())
	{
		UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
		LightComp->SetIntensity(CurrentSettings.SunIntensity);
		LightComp->SetLightColor(CurrentSettings.SunColor);
		LightComp->SetTemperature(CurrentSettings.SunTemperature);
		
		if (bEnableGodRays)
		{
			LightComp->SetCastVolumetricShadow(true);
			LightComp->SetVolumetricScatteringIntensity(CurrentSettings.VolumetricScattering);
		}
	}

	// Update fog
	UpdateFogSettings();

	// Update sky atmosphere
	UpdateSkyAtmosphere();
}

void ALight_AtmosphericLightingManager::SetSunIntensity(float Intensity)
{
	if (SunLight && SunLight->GetLightComponent())
	{
		SunLight->GetLightComponent()->SetIntensity(Intensity);
	}
}

void ALight_AtmosphericLightingManager::SetSunColor(FLinearColor Color)
{
	if (SunLight && SunLight->GetLightComponent())
	{
		SunLight->GetLightComponent()->SetLightColor(Color);
	}
}

void ALight_AtmosphericLightingManager::SetFogDensity(float Density)
{
	if (AtmosphericFog && AtmosphericFog->GetComponent())
	{
		AtmosphericFog->GetComponent()->SetFogDensity(Density);
	}
}

void ALight_AtmosphericLightingManager::EnableVolumetricFog(bool bEnable)
{
	bEnableVolumetricFog = bEnable;
	
	if (AtmosphericFog && AtmosphericFog->GetComponent())
	{
		AtmosphericFog->GetComponent()->SetVolumetricFog(bEnable);
	}
}

void ALight_AtmosphericLightingManager::EnableGodRays(bool bEnable)
{
	bEnableGodRays = bEnable;
	
	if (SunLight && SunLight->GetLightComponent())
	{
		SunLight->GetLightComponent()->SetCastVolumetricShadow(bEnable);
	}
}

void ALight_AtmosphericLightingManager::SetAtmosphericDensity(float Density)
{
	AtmosphericDensity = FMath::Clamp(Density, 0.1f, 5.0f);
	UpdateSkyAtmosphere();
}

void ALight_AtmosphericLightingManager::FindAndCacheLightingActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Find directional light (sun)
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SunLight = Cast<ADirectionalLight>(FoundActors[0]);
	}

	// Find sky light
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SkyLight = Cast<ASkyLight>(FoundActors[0]);
	}

	// Find atmospheric fog
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
	}

	// Find sky atmosphere
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
	}
}

void ALight_AtmosphericLightingManager::ApplyPrehistoricAtmosphere()
{
	// Apply prehistoric-specific atmospheric settings
	if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
	{
		USkyAtmosphereComponent* AtmComp = SkyAtmosphere->GetAtmosphereComponent();
		AtmComp->RayleighScatteringScale = 0.8f * AtmosphericDensity;
		AtmComp->RayleighExponentialDistribution = 8.0f;
		AtmComp->MieScatteringScale = 0.6f * AtmosphericDensity;
		AtmComp->MieAbsorptionScale = 0.4f;
		AtmComp->OtherAbsorptionScale = 0.2f;
	}

	// Set up volumetric fog for prehistoric atmosphere
	if (AtmosphericFog && AtmosphericFog->GetComponent())
	{
		UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
		FogComp->SetVolumetricFog(bEnableVolumetricFog);
		FogComp->SetVolumetricFogScatteringDistribution(0.6f);
		FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.85f, 0.7f, 1.0f));
		FogComp->SetVolumetricFogEmissive(FLinearColor(0.1f, 0.08f, 0.05f, 1.0f));
		FogComp->SetVolumetricFogExtinctionScale(1.2f);
	}
}

void ALight_AtmosphericLightingManager::SetupDefaultLighting()
{
	FindAndCacheLightingActors();
	ApplyPrehistoricAtmosphere();
	SetTimeOfDay(12.0f); // Set to noon
}

void ALight_AtmosphericLightingManager::UpdateTimeOfDay(float DeltaTime)
{
	float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
	CurrentTimeOfDay += TimeIncrement;
	
	if (CurrentTimeOfDay >= 24.0f)
	{
		CurrentTimeOfDay -= 24.0f;
	}

	UpdateSunPosition();
}

void ALight_AtmosphericLightingManager::UpdateWeatherTransition(float DeltaTime)
{
	WeatherTransitionProgress += DeltaTime / WeatherTransitionDuration;
	
	if (WeatherTransitionProgress >= 1.0f)
	{
		CurrentWeather = TargetWeatherSettings;
		bIsTransitioningWeather = false;
		WeatherTransitionProgress = 1.0f;
	}
	else
	{
		// Interpolate weather settings
		float Alpha = WeatherTransitionProgress;
		CurrentWeather.CloudCoverage = InterpolateFloat(StartWeatherSettings.CloudCoverage, TargetWeatherSettings.CloudCoverage, Alpha);
		CurrentWeather.RainIntensity = InterpolateFloat(StartWeatherSettings.RainIntensity, TargetWeatherSettings.RainIntensity, Alpha);
		CurrentWeather.WindStrength = InterpolateFloat(StartWeatherSettings.WindStrength, TargetWeatherSettings.WindStrength, Alpha);
		CurrentWeather.AtmosphericPressure = InterpolateFloat(StartWeatherSettings.AtmosphericPressure, TargetWeatherSettings.AtmosphericPressure, Alpha);
		CurrentWeather.WeatherTint = InterpolateColor(StartWeatherSettings.WeatherTint, TargetWeatherSettings.WeatherTint, Alpha);
	}
}

void ALight_AtmosphericLightingManager::UpdateSunPosition()
{
	if (!SunLight) return;

	// Calculate sun position based on time of day
	float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at sunrise
	float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle * 0.5f)) * 80.0f; // Max 80 degrees elevation
	
	FRotator SunRotation;
	SunRotation.Pitch = -SunElevation;
	SunRotation.Yaw = SunAngle;
	SunRotation.Roll = 0.0f;
	
	SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphericLightingManager::UpdateFogSettings()
{
	if (!AtmosphericFog || !AtmosphericFog->GetComponent()) return;

	FLight_TimeOfDaySettings CurrentSettings = InterpolateTimeSettings(CurrentTimeOfDay);
	UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
	
	// Apply weather influence on fog
	float WeatherFogMultiplier = 1.0f + (CurrentWeather.CloudCoverage * 0.5f) + (CurrentWeather.RainIntensity * 1.0f);
	float FinalFogDensity = CurrentSettings.FogDensity * WeatherFogMultiplier;
	
	FogComp->SetFogDensity(FinalFogDensity);
	FogComp->SetFogInscatteringColor(CurrentSettings.FogColor * CurrentWeather.WeatherTint);
	
	if (bEnableVolumetricFog)
	{
		FogComp->SetVolumetricFogScatteringDistribution(0.6f + (CurrentWeather.AtmosphericPressure - 1.0f) * 0.2f);
	}
}

void ALight_AtmosphericLightingManager::UpdateSkyAtmosphere()
{
	if (!SkyAtmosphere || !SkyAtmosphere->GetAtmosphereComponent()) return;

	USkyAtmosphereComponent* AtmComp = SkyAtmosphere->GetAtmosphereComponent();
	
	// Apply atmospheric density and weather effects
	float WeatherScatteringMultiplier = 1.0f + (CurrentWeather.CloudCoverage * 0.3f);
	
	AtmComp->RayleighScatteringScale = 0.8f * AtmosphericDensity * WeatherScatteringMultiplier;
	AtmComp->MieScatteringScale = 0.6f * AtmosphericDensity * WeatherScatteringMultiplier;
	AtmComp->MieAbsorptionScale = 0.4f + (CurrentWeather.RainIntensity * 0.2f);
}

FLight_TimeOfDaySettings ALight_AtmosphericLightingManager::InterpolateTimeSettings(float TimeOfDay)
{
	FLight_TimeOfDaySettings Result;
	
	if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f) // Dawn (5-7 AM)
	{
		float Alpha = (TimeOfDay - 5.0f) / 2.0f;
		Result.SunIntensity = InterpolateFloat(NightSettings.SunIntensity, DawnSettings.SunIntensity, Alpha);
		Result.SunColor = InterpolateColor(NightSettings.SunColor, DawnSettings.SunColor, Alpha);
		Result.SunTemperature = InterpolateFloat(NightSettings.SunTemperature, DawnSettings.SunTemperature, Alpha);
		Result.FogDensity = InterpolateFloat(NightSettings.FogDensity, DawnSettings.FogDensity, Alpha);
		Result.FogColor = InterpolateColor(NightSettings.FogColor, DawnSettings.FogColor, Alpha);
		Result.VolumetricScattering = InterpolateFloat(NightSettings.VolumetricScattering, DawnSettings.VolumetricScattering, Alpha);
	}
	else if (TimeOfDay >= 7.0f && TimeOfDay < 11.0f) // Morning (7-11 AM)
	{
		float Alpha = (TimeOfDay - 7.0f) / 4.0f;
		Result.SunIntensity = InterpolateFloat(DawnSettings.SunIntensity, NoonSettings.SunIntensity, Alpha);
		Result.SunColor = InterpolateColor(DawnSettings.SunColor, NoonSettings.SunColor, Alpha);
		Result.SunTemperature = InterpolateFloat(DawnSettings.SunTemperature, NoonSettings.SunTemperature, Alpha);
		Result.FogDensity = InterpolateFloat(DawnSettings.FogDensity, NoonSettings.FogDensity, Alpha);
		Result.FogColor = InterpolateColor(DawnSettings.FogColor, NoonSettings.FogColor, Alpha);
		Result.VolumetricScattering = InterpolateFloat(DawnSettings.VolumetricScattering, NoonSettings.VolumetricScattering, Alpha);
	}
	else if (TimeOfDay >= 11.0f && TimeOfDay < 16.0f) // Noon (11 AM - 4 PM)
	{
		Result = NoonSettings;
	}
	else if (TimeOfDay >= 16.0f && TimeOfDay < 19.0f) // Afternoon (4-7 PM)
	{
		float Alpha = (TimeOfDay - 16.0f) / 3.0f;
		Result.SunIntensity = InterpolateFloat(NoonSettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
		Result.SunColor = InterpolateColor(NoonSettings.SunColor, DuskSettings.SunColor, Alpha);
		Result.SunTemperature = InterpolateFloat(NoonSettings.SunTemperature, DuskSettings.SunTemperature, Alpha);
		Result.FogDensity = InterpolateFloat(NoonSettings.FogDensity, DuskSettings.FogDensity, Alpha);
		Result.FogColor = InterpolateColor(NoonSettings.FogColor, DuskSettings.FogColor, Alpha);
		Result.VolumetricScattering = InterpolateFloat(NoonSettings.VolumetricScattering, DuskSettings.VolumetricScattering, Alpha);
	}
	else if (TimeOfDay >= 19.0f && TimeOfDay < 21.0f) // Dusk (7-9 PM)
	{
		float Alpha = (TimeOfDay - 19.0f) / 2.0f;
		Result.SunIntensity = InterpolateFloat(DuskSettings.SunIntensity, NightSettings.SunIntensity, Alpha);
		Result.SunColor = InterpolateColor(DuskSettings.SunColor, NightSettings.SunColor, Alpha);
		Result.SunTemperature = InterpolateFloat(DuskSettings.SunTemperature, NightSettings.SunTemperature, Alpha);
		Result.FogDensity = InterpolateFloat(DuskSettings.FogDensity, NightSettings.FogDensity, Alpha);
		Result.FogColor = InterpolateColor(DuskSettings.FogColor, NightSettings.FogColor, Alpha);
		Result.VolumetricScattering = InterpolateFloat(DuskSettings.VolumetricScattering, NightSettings.VolumetricScattering, Alpha);
	}
	else // Night (9 PM - 5 AM)
	{
		Result = NightSettings;
	}
	
	return Result;
}

FLinearColor ALight_AtmosphericLightingManager::InterpolateColor(const FLinearColor& A, const FLinearColor& B, float Alpha)
{
	return FLinearColor::LerpUsingHSV(A, B, Alpha);
}

float ALight_AtmosphericLightingManager::InterpolateFloat(float A, float B, float Alpha)
{
	return FMath::Lerp(A, B, Alpha);
}