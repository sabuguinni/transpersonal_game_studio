#include "CretaceousLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth day cycle

	// ── Dawn Preset (5:00-7:00) ──────────────────────────────────────────────
	DawnPreset.PresetName = TEXT("Dawn");
	DawnPreset.SunPitchDegrees = -8.0f;
	DawnPreset.SunYawDegrees = -60.0f;
	DawnPreset.SunIntensity = 3.0f;
	DawnPreset.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);  // Deep orange
	DawnPreset.SkyLightIntensity = 0.8f;
	DawnPreset.FogDensity = 0.035f;
	DawnPreset.FogColor = FLinearColor(0.85f, 0.65f, 0.45f, 1.0f); // Warm haze
	DawnPreset.VolumetricFogExtinction = 1.2f;
	DawnPreset.AmbientTemperatureCelsius = 22.0f;
	DawnPreset.FogCutoffDistance = 40000.0f;

	// ── Noon Preset (11:00-13:00) ────────────────────────────────────────────
	NoonPreset.PresetName = TEXT("Noon");
	NoonPreset.SunPitchDegrees = -75.0f;
	NoonPreset.SunYawDegrees = 0.0f;
	NoonPreset.SunIntensity = 18.0f;
	NoonPreset.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);  // Near-white
	NoonPreset.SkyLightIntensity = 3.0f;
	NoonPreset.FogDensity = 0.008f;
	NoonPreset.FogColor = FLinearColor(0.6f, 0.78f, 1.0f, 1.0f);   // Clear blue
	NoonPreset.VolumetricFogExtinction = 0.3f;
	NoonPreset.AmbientTemperatureCelsius = 42.0f;
	NoonPreset.FogCutoffDistance = 100000.0f;

	// ── Dusk Preset (17:00-19:00) ────────────────────────────────────────────
	DuskPreset.PresetName = TEXT("Dusk");
	DuskPreset.SunPitchDegrees = -5.0f;
	DuskPreset.SunYawDegrees = 120.0f;
	DuskPreset.SunIntensity = 2.5f;
	DuskPreset.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);    // Deep red-orange
	DuskPreset.SkyLightIntensity = 0.6f;
	DuskPreset.FogDensity = 0.04f;
	DuskPreset.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);    // Amber haze
	DuskPreset.VolumetricFogExtinction = 1.5f;
	DuskPreset.AmbientTemperatureCelsius = 28.0f;
	DuskPreset.FogCutoffDistance = 35000.0f;

	// ── Night Preset (21:00-4:00) ────────────────────────────────────────────
	NightPreset.PresetName = TEXT("Night");
	NightPreset.SunPitchDegrees = 30.0f;  // Below horizon
	NightPreset.SunYawDegrees = 180.0f;
	NightPreset.SunIntensity = 0.1f;
	NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);   // Moonlight blue
	NightPreset.SkyLightIntensity = 0.3f;
	NightPreset.FogDensity = 0.025f;
	NightPreset.FogColor = FLinearColor(0.1f, 0.15f, 0.35f, 1.0f); // Dark blue mist
	NightPreset.VolumetricFogExtinction = 0.8f;
	NightPreset.AmbientTemperatureCelsius = 18.0f;
	NightPreset.FogCutoffDistance = 25000.0f;
}

void ACretaceousLightingSystem::BeginPlay()
{
	Super::BeginPlay();
	FindAndCacheLightingActors();
	// Apply initial state
	SetTimeOfDay(CurrentTimeOfDayHours);
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnableDayNightCycle)
	{
		return;
	}

	// Advance time (DayNightSpeedMultiplier = 120 → 1 real second = 2 game minutes)
	const float GameMinutesPerSecond = DayNightSpeedMultiplier / 60.0f;
	const float HoursPerSecond = GameMinutesPerSecond / 60.0f;
	CurrentTimeOfDayHours += HoursPerSecond * DeltaTime;

	// Wrap at 24 hours
	if (CurrentTimeOfDayHours >= 24.0f)
	{
		CurrentTimeOfDayHours -= 24.0f;
	}

	// Update lighting
	SetTimeOfDay(CurrentTimeOfDayHours);

	// Hourly tick event
	HourAccumulator += DeltaTime;
	const int32 CurrentHourInt = FMath::FloorToInt(CurrentTimeOfDayHours);
	if (CurrentHourInt != LastHourTick)
	{
		LastHourTick = CurrentHourInt;
		OnGameHourTick(CurrentTimeOfDayHours);
	}
}

void ACretaceousLightingSystem::SetTimeOfDay(float NewHour)
{
	CurrentTimeOfDayHours = FMath::Clamp(NewHour, 0.0f, 24.0f);

	// Determine new phase
	ELight_TimeOfDay NewPhase = CalculatePhase(CurrentTimeOfDayHours);
	if (NewPhase != CurrentPhase)
	{
		ELight_TimeOfDay OldPhase = CurrentPhase;
		CurrentPhase = NewPhase;
		bIsNight = (CurrentPhase == ELight_TimeOfDay::Night);
		OnTimeOfDayPhaseChanged(NewPhase, OldPhase);
	}

	// Build interpolated preset for current hour
	FLight_TimeOfDayPreset CurrentPreset;
	const float H = CurrentTimeOfDayHours;

	if (H >= 5.0f && H < 7.0f)
	{
		// Dawn: pure dawn preset
		float Alpha = (H - 5.0f) / 2.0f;
		CurrentPreset = LerpPresets(NightPreset, DawnPreset, Alpha);
	}
	else if (H >= 7.0f && H < 11.0f)
	{
		// Morning: dawn → noon
		float Alpha = (H - 7.0f) / 4.0f;
		CurrentPreset = LerpPresets(DawnPreset, NoonPreset, Alpha);
	}
	else if (H >= 11.0f && H < 14.0f)
	{
		// Noon: peak
		CurrentPreset = NoonPreset;
	}
	else if (H >= 14.0f && H < 17.0f)
	{
		// Afternoon: noon → dusk
		float Alpha = (H - 14.0f) / 3.0f;
		CurrentPreset = LerpPresets(NoonPreset, DuskPreset, Alpha);
	}
	else if (H >= 17.0f && H < 20.0f)
	{
		// Dusk
		float Alpha = (H - 17.0f) / 3.0f;
		CurrentPreset = LerpPresets(DuskPreset, NightPreset, Alpha);
	}
	else
	{
		// Night (20:00 - 5:00)
		CurrentPreset = NightPreset;
	}

	CurrentAmbientTemperature = CurrentPreset.AmbientTemperatureCelsius;

	// Apply to scene actors
	UpdateSunLight(CurrentPreset);
	UpdateSkyLight(CurrentPreset);
	UpdateHeightFog(CurrentPreset);
}

void ACretaceousLightingSystem::ApplyPreset(const FLight_TimeOfDayPreset& Preset)
{
	UpdateSunLight(Preset);
	UpdateSkyLight(Preset);
	UpdateHeightFog(Preset);
	CurrentAmbientTemperature = Preset.AmbientTemperatureCelsius;
}

void ACretaceousLightingSystem::FindAndCacheLightingActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Find DirectionalLight
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunLight = *It;
		break;
	}

	// Find SkyLight
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		SkyLightActor = *It;
		break;
	}

	// Find ExponentialHeightFog
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		HeightFogActor = *It;
		break;
	}
}

FLight_TimeOfDayPreset ACretaceousLightingSystem::LerpPresets(
	const FLight_TimeOfDayPreset& A,
	const FLight_TimeOfDayPreset& B,
	float Alpha) const
{
	FLight_TimeOfDayPreset Result;
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
	Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
	Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
	Result.SunColor = FLinearColor(
		FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
		FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
		FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
		1.0f);
	Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
	Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
	Result.FogColor = FLinearColor(
		FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
		FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
		FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
		1.0f);
	Result.VolumetricFogExtinction = FMath::Lerp(A.VolumetricFogExtinction, B.VolumetricFogExtinction, Alpha);
	Result.AmbientTemperatureCelsius = FMath::Lerp(A.AmbientTemperatureCelsius, B.AmbientTemperatureCelsius, Alpha);
	Result.FogCutoffDistance = FMath::Lerp(A.FogCutoffDistance, B.FogCutoffDistance, Alpha);

	return Result;
}

void ACretaceousLightingSystem::UpdateSunLight(const FLight_TimeOfDayPreset& Preset)
{
	if (!SunLight) return;

	SunLight->SetActorRotation(FRotator(Preset.SunPitchDegrees, Preset.SunYawDegrees, 0.0f));

	UDirectionalLightComponent* DC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
	if (DC)
	{
		DC->SetIntensity(Preset.SunIntensity);
		DC->SetLightColor(Preset.SunColor.ToFColor(true));
	}
}

void ACretaceousLightingSystem::UpdateSkyLight(const FLight_TimeOfDayPreset& Preset)
{
	if (!SkyLightActor) return;

	USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
	if (SLC)
	{
		SLC->SetIntensity(Preset.SkyLightIntensity);
	}
}

void ACretaceousLightingSystem::UpdateHeightFog(const FLight_TimeOfDayPreset& Preset)
{
	if (!HeightFogActor) return;

	UExponentialHeightFogComponent* FC = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
	if (FC)
	{
		FC->SetFogDensity(Preset.FogDensity);
		FC->SetFogInscatteringColor(Preset.FogColor.ToFColor(true));
		FC->SetFogCutoffDistance(Preset.FogCutoffDistance);
	}
}

ELight_TimeOfDay ACretaceousLightingSystem::CalculatePhase(float Hour) const
{
	if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
	if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
	if (Hour >= 11.0f && Hour < 14.0f) return ELight_TimeOfDay::Noon;
	if (Hour >= 14.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
	if (Hour >= 17.0f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
	return ELight_TimeOfDay::Night;
}
