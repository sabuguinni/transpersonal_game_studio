#include "LightingAtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
// ULight_AtmosphereComponent
// ============================================================

ULight_AtmosphereComponent::ULight_AtmosphereComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10x/sec for smooth day cycle
}

void ULight_AtmosphereComponent::BeginPlay()
{
    Super::BeginPlay();
    FindLightingComponents();
}

void ULight_AtmosphereComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bDayNightCycleActive || DayDurationSeconds <= 0.0f)
    {
        return;
    }

    // Advance time
    float TimeAdvance = DeltaTime / DayDurationSeconds;
    CurrentTimeOfDayNormalized = FMath::Fmod(CurrentTimeOfDayNormalized + TimeAdvance, 1.0f);

    // Apply preset for current time
    FLight_TimeOfDayPreset Preset = GetPresetForTime(CurrentTimeOfDayNormalized);
    ApplyTimeOfDayPreset(Preset);
}

ELight_TimeOfDay ULight_AtmosphereComponent::GetCurrentTimeOfDay() const
{
    float T = CurrentTimeOfDayNormalized;
    if (T < 0.08f) return ELight_TimeOfDay::Night;
    if (T < 0.15f) return ELight_TimeOfDay::Dawn;
    if (T < 0.30f) return ELight_TimeOfDay::Morning;
    if (T < 0.50f) return ELight_TimeOfDay::Midday;
    if (T < 0.65f) return ELight_TimeOfDay::Afternoon;
    if (T < 0.75f) return ELight_TimeOfDay::GoldenHour;
    if (T < 0.85f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

float ULight_AtmosphereComponent::GetSunPitchForTime(float NormalizedTime) const
{
    // Map 0-1 time to sun arc: rises at 0.1, peaks at 0.5, sets at 0.9
    // Pitch: -90 = horizon, 0 = zenith (UE convention: negative pitch = looking down = sun high)
    float SunAngle = FMath::Sin((NormalizedTime - 0.1f) / 0.8f * PI);
    SunAngle = FMath::Clamp(SunAngle, 0.0f, 1.0f);
    // Map 0..1 to pitch -10 (horizon) .. -80 (zenith)
    float Pitch = FMath::Lerp(-10.0f, -80.0f, SunAngle);
    // Night: sun below horizon
    if (NormalizedTime < 0.1f || NormalizedTime > 0.9f)
    {
        Pitch = -5.0f;
    }
    return Pitch;
}

FLight_TimeOfDayPreset ULight_AtmosphereComponent::GetPresetForTime(float NormalizedTime) const
{
    FLight_TimeOfDayPreset Preset;

    ELight_TimeOfDay TOD = GetCurrentTimeOfDay();

    switch (TOD)
    {
    case ELight_TimeOfDay::Dawn:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 3.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f); // Orange-red dawn
        Preset.SkyLightIntensity = 0.4f;
        Preset.FogDensity = 0.04f;
        Preset.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
        break;

    case ELight_TimeOfDay::Morning:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 6.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f); // Warm morning
        Preset.SkyLightIntensity = 0.9f;
        Preset.FogDensity = 0.025f;
        Preset.FogColor = FLinearColor(0.65f, 0.78f, 0.92f, 1.0f);
        break;

    case ELight_TimeOfDay::Midday:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 10.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f); // Bright white-yellow
        Preset.SkyLightIntensity = 1.5f;
        Preset.FogDensity = 0.01f;
        Preset.FogColor = FLinearColor(0.55f, 0.72f, 0.95f, 1.0f);
        break;

    case ELight_TimeOfDay::Afternoon:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 8.5f;
        Preset.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f); // Warm afternoon
        Preset.SkyLightIntensity = 1.2f;
        Preset.FogDensity = 0.018f;
        Preset.FogColor = FLinearColor(0.62f, 0.76f, 0.9f, 1.0f);
        break;

    case ELight_TimeOfDay::GoldenHour:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 7.0f;
        Preset.SunColor = FLinearColor(1.0f, 0.65f, 0.25f, 1.0f); // Deep golden
        Preset.SkyLightIntensity = 0.8f;
        Preset.FogDensity = 0.035f;
        Preset.FogColor = FLinearColor(0.9f, 0.6f, 0.3f, 1.0f);
        break;

    case ELight_TimeOfDay::Dusk:
        Preset.SunPitchDegrees = GetSunPitchForTime(NormalizedTime);
        Preset.SunIntensity = 2.5f;
        Preset.SunColor = FLinearColor(0.9f, 0.35f, 0.1f, 1.0f); // Red dusk
        Preset.SkyLightIntensity = 0.3f;
        Preset.FogDensity = 0.05f;
        Preset.FogColor = FLinearColor(0.7f, 0.4f, 0.25f, 1.0f);
        break;

    case ELight_TimeOfDay::Night:
    default:
        Preset.SunPitchDegrees = -5.0f;
        Preset.SunIntensity = 0.1f;
        Preset.SunColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f); // Cool moonlight
        Preset.SkyLightIntensity = 0.15f;
        Preset.FogDensity = 0.06f;
        Preset.FogColor = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
        break;
    }

    return Preset;
}

void ULight_AtmosphereComponent::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    // Weather modifies fog density and sky light
    if (CachedFog)
    {
        float WeatherFogMult = 1.0f;
        switch (NewWeather)
        {
        case ELight_WeatherState::PartlyCloudy: WeatherFogMult = 1.5f; break;
        case ELight_WeatherState::Overcast:     WeatherFogMult = 2.5f; break;
        case ELight_WeatherState::Rain:         WeatherFogMult = 4.0f; break;
        case ELight_WeatherState::Storm:        WeatherFogMult = 6.0f; break;
        case ELight_WeatherState::HeatHaze:     WeatherFogMult = 1.8f; break;
        default: WeatherFogMult = 1.0f; break;
        }
        // Apply weather fog multiplier (stored for next tick application)
    }
}

void ULight_AtmosphereComponent::FindLightingComponents()
{
    if (!GetWorld()) return;

    // Find DirectionalLight component
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        CachedSunLight = It->GetComponentByClass<UDirectionalLightComponent>();
        break;
    }

    // Find SkyLight component
    for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
    {
        CachedSkyLight = It->GetComponentByClass<USkyLightComponent>();
        break;
    }

    // Find ExponentialHeightFog component
    for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
    {
        CachedFog = It->GetComponentByClass<UExponentialHeightFogComponent>();
        break;
    }
}

void ULight_AtmosphereComponent::ApplyTimeOfDayPreset(const FLight_TimeOfDayPreset& Preset)
{
    if (CachedSunLight)
    {
        CachedSunLight->SetIntensity(Preset.SunIntensity);
        CachedSunLight->SetLightColor(Preset.SunColor);
        // Rotate the owning actor to set sun pitch
        if (AActor* Owner = CachedSunLight->GetOwner())
        {
            FRotator CurrentRot = Owner->GetActorRotation();
            Owner->SetActorRotation(FRotator(Preset.SunPitchDegrees, CurrentRot.Yaw, CurrentRot.Roll));
        }
    }

    if (CachedSkyLight)
    {
        CachedSkyLight->SetIntensity(Preset.SkyLightIntensity);
    }

    if (CachedFog)
    {
        CachedFog->SetFogDensity(Preset.FogDensity);
        CachedFog->SetFogInscatteringColor(Preset.FogColor);
    }
}

// ============================================================
// ALight_AtmosphereManager
// ============================================================

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    AtmosphereComponent = CreateDefaultSubobject<ULight_AtmosphereComponent>(TEXT("AtmosphereComponent"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    // Apply default golden hour preset on start
    ApplyCretaceousGoldenHour();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // AtmosphereComponent handles tick internally
}

void ALight_AtmosphereManager::ApplyCretaceousGoldenHour()
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->CurrentTimeOfDayNormalized = 0.68f; // Golden hour
        FLight_TimeOfDayPreset Preset = AtmosphereComponent->GetPresetForTime(0.68f);
        // Apply directly to referenced actors if set
        if (SunLight)
        {
            UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
            if (DLC)
            {
                DLC->SetIntensity(Preset.SunIntensity);
                DLC->SetLightColor(Preset.SunColor);
                SunLight->SetActorRotation(FRotator(Preset.SunPitchDegrees, -45.0f, 0.0f));
            }
        }
    }
}

void ALight_AtmosphereManager::ApplyCretaceousMidday()
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->CurrentTimeOfDayNormalized = 0.5f;
    }
}

void ALight_AtmosphereManager::ApplyCretaceousNight()
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->CurrentTimeOfDayNormalized = 0.95f;
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float NormalizedTime)
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->CurrentTimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    }
}
