#include "LightingSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ALight_SystemManager::ALight_SystemManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize time settings presets
	DawnSettings.SunIntensity = 2.0f;
	DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f); // Orange dawn
	DawnSettings.SunAngle = -10.0f;
	DawnSettings.SkyIntensity = 0.8f;
	DawnSettings.SkyColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
	DawnSettings.FogDensity = 0.05f;
	DawnSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

	MorningSettings.SunIntensity = 4.0f;
	MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm morning
	MorningSettings.SunAngle = -30.0f;
	MorningSettings.SkyIntensity = 1.2f;
	MorningSettings.SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
	MorningSettings.FogDensity = 0.03f;
	MorningSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);

	MiddaySettings.SunIntensity = 6.0f;
	MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f); // Bright midday
	MiddaySettings.SunAngle = -80.0f;
	MiddaySettings.SkyIntensity = 1.8f;
	MiddaySettings.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
	MiddaySettings.FogDensity = 0.01f;
	MiddaySettings.FogColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	AfternoonSettings.SunIntensity = 5.0f;
	AfternoonSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm afternoon
	AfternoonSettings.SunAngle = -45.0f;
	AfternoonSettings.SkyIntensity = 1.5f;
	AfternoonSettings.SkyColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
	AfternoonSettings.FogDensity = 0.02f;
	AfternoonSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

	DuskSettings.SunIntensity = 3.0f;
	DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f); // Orange/red dusk
	DuskSettings.SunAngle = -5.0f;
	DuskSettings.SkyIntensity = 1.0f;
	DuskSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
	DuskSettings.FogDensity = 0.04f;
	DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);

	NightSettings.SunIntensity = 0.5f;
	NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Moonlight blue
	NightSettings.SunAngle = 45.0f; // Moon position
	NightSettings.SkyIntensity = 0.3f;
	NightSettings.SkyColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
	NightSettings.FogDensity = 0.06f;
	NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
}

void ALight_SystemManager::BeginPlay()
{
	Super::BeginPlay();
	
	FindLightingActors();
	UpdateLighting();
}

void ALight_SystemManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoAdvanceTime)
	{
		// Advance time (24 hours in DayDuration seconds)
		CurrentTime += (24.0f / DayDuration) * DeltaTime;
		
		// Wrap around 24 hours
		if (CurrentTime >= 24.0f)
		{
			CurrentTime -= 24.0f;
		}

		UpdateTimeOfDay();
		UpdateLighting();
	}
}

void ALight_SystemManager::SetTimeOfDay(float NewTime)
{
	CurrentTime = FMath::Clamp(NewTime, 0.0f, 24.0f);
	UpdateTimeOfDay();
	UpdateLighting();
}

void ALight_SystemManager::SetTimeOfDayInstant(ELight_TimeOfDay TimeOfDay)
{
	CurrentTimeOfDay = TimeOfDay;
	
	switch (TimeOfDay)
	{
		case ELight_TimeOfDay::Dawn:
			CurrentTime = 6.0f;
			break;
		case ELight_TimeOfDay::Morning:
			CurrentTime = 9.0f;
			break;
		case ELight_TimeOfDay::Midday:
			CurrentTime = 12.0f;
			break;
		case ELight_TimeOfDay::Afternoon:
			CurrentTime = 15.0f;
			break;
		case ELight_TimeOfDay::Dusk:
			CurrentTime = 18.0f;
			break;
		case ELight_TimeOfDay::Night:
			CurrentTime = 21.0f;
			break;
	}
	
	UpdateLighting();
}

void ALight_SystemManager::SetWeather(ELight_WeatherType NewWeather)
{
	CurrentWeather = NewWeather;
	ApplyWeatherEffects();
}

void ALight_SystemManager::UpdateLighting()
{
	if (!SunLight || !SkyLight || !AtmosphericFog)
	{
		FindLightingActors();
		if (!SunLight || !SkyLight || !AtmosphericFog)
		{
			return;
		}
	}

	FLight_TimeSettings CurrentSettings = GetCurrentTimeSettings();
	
	// Update sun light
	if (UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent())
	{
		SunComponent->SetIntensity(CurrentSettings.SunIntensity);
		SunComponent->SetLightColor(CurrentSettings.SunColor);
		
		// Set sun rotation based on time
		FRotator SunRotation = FRotator(CurrentSettings.SunAngle, 0.0f, 0.0f);
		SunLight->SetActorRotation(SunRotation);
	}

	// Update sky light
	if (USkyLightComponent* SkyComponent = SkyLight->GetLightComponent())
	{
		SkyComponent->SetIntensity(CurrentSettings.SkyIntensity);
		SkyComponent->SetLightColor(CurrentSettings.SkyColor);
		SkyComponent->RecaptureSky();
	}

	// Update atmospheric fog
	if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
	{
		FogComponent->SetFogDensity(CurrentSettings.FogDensity);
		FogComponent->SetFogInscatteringColor(CurrentSettings.FogColor);
	}

	ApplyWeatherEffects();
}

void ALight_SystemManager::FindLightingActors()
{
	TArray<AActor*> FoundActors;
	
	// Find directional light (sun)
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SunLight = Cast<ADirectionalLight>(FoundActors[0]);
	}

	// Find sky light
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SkyLight = Cast<ASkyLight>(FoundActors[0]);
	}

	// Find atmospheric fog
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
	}
}

FString ALight_SystemManager::GetTimeString() const
{
	int32 Hours = FMath::FloorToInt(CurrentTime);
	int32 Minutes = FMath::FloorToInt((CurrentTime - Hours) * 60.0f);
	
	return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

float ALight_SystemManager::GetNormalizedTime() const
{
	return CurrentTime / 24.0f;
}

void ALight_SystemManager::UpdateTimeOfDay()
{
	CurrentTimeOfDay = CalculateTimeOfDay(CurrentTime);
}

void ALight_SystemManager::InterpolateLighting(const FLight_TimeSettings& FromSettings, const FLight_TimeSettings& ToSettings, float Alpha)
{
	// This function can be used for smooth transitions between time periods
	// Currently using direct settings application, but could be enhanced for smoother transitions
}

FLight_TimeSettings ALight_SystemManager::GetCurrentTimeSettings() const
{
	// Simple time-based settings selection
	// Could be enhanced with interpolation between adjacent time periods
	switch (CurrentTimeOfDay)
	{
		case ELight_TimeOfDay::Dawn:
			return DawnSettings;
		case ELight_TimeOfDay::Morning:
			return MorningSettings;
		case ELight_TimeOfDay::Midday:
			return MiddaySettings;
		case ELight_TimeOfDay::Afternoon:
			return AfternoonSettings;
		case ELight_TimeOfDay::Dusk:
			return DuskSettings;
		case ELight_TimeOfDay::Night:
		default:
			return NightSettings;
	}
}

ELight_TimeOfDay ALight_SystemManager::CalculateTimeOfDay(float Time) const
{
	if (Time >= 5.0f && Time < 8.0f)
		return ELight_TimeOfDay::Dawn;
	else if (Time >= 8.0f && Time < 11.0f)
		return ELight_TimeOfDay::Morning;
	else if (Time >= 11.0f && Time < 14.0f)
		return ELight_TimeOfDay::Midday;
	else if (Time >= 14.0f && Time < 17.0f)
		return ELight_TimeOfDay::Afternoon;
	else if (Time >= 17.0f && Time < 20.0f)
		return ELight_TimeOfDay::Dusk;
	else
		return ELight_TimeOfDay::Night;
}

void ALight_SystemManager::ApplyWeatherEffects()
{
	if (!AtmosphericFog)
		return;

	UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
	if (!FogComponent)
		return;

	// Modify fog based on weather
	float WeatherFogMultiplier = 1.0f;
	FLinearColor WeatherFogTint = FLinearColor::White;

	switch (CurrentWeather)
	{
		case ELight_WeatherType::Clear:
			WeatherFogMultiplier = 1.0f;
			break;
		case ELight_WeatherType::Cloudy:
			WeatherFogMultiplier = 1.3f;
			WeatherFogTint = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
			break;
		case ELight_WeatherType::Overcast:
			WeatherFogMultiplier = 1.6f;
			WeatherFogTint = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
			break;
		case ELight_WeatherType::Foggy:
			WeatherFogMultiplier = 3.0f;
			WeatherFogTint = FLinearColor(0.9f, 0.9f, 0.95f, 1.0f);
			break;
		case ELight_WeatherType::Stormy:
			WeatherFogMultiplier = 2.0f;
			WeatherFogTint = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
			break;
	}

	// Apply weather effects to fog
	FLight_TimeSettings CurrentSettings = GetCurrentTimeSettings();
	float ModifiedDensity = CurrentSettings.FogDensity * WeatherFogMultiplier;
	FLinearColor ModifiedColor = CurrentSettings.FogColor * WeatherFogTint;

	FogComponent->SetFogDensity(ModifiedDensity);
	FogComponent->SetFogInscatteringColor(ModifiedColor);
}