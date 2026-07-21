#include "DynamicLightingSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ADynamicLightingSystem::ADynamicLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create sun light component
    SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLightComponent->SetupAttachment(RootComponent);
    SunLightComponent->SetIntensity(5.0f);
    SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
    SunLightComponent->SetCastShadows(true);
    SunLightComponent->SetCastVolumetricShadow(true);

    // Create sky light component
    SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLightComponent->SetupAttachment(RootComponent);
    SkyLightComponent->SetIntensity(1.0f);
    SkyLightComponent->SetSourceType(SLS_CapturedScene);

    // Initialize time settings
    TimeOfDay = 12.0f; // Start at midday
    DayDurationMinutes = 24.0f; // 24 minutes = 1 full day
    bEnableDayNightCycle = true;

    InitializeLightingPresets();
}

void ADynamicLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial Cretaceous atmosphere
    ApplyCretaceousAtmosphere();
    UpdateLighting();
}

void ADynamicLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Advance time of day
        float TimeIncrement = (DeltaTime / 60.0f) / DayDurationMinutes * 24.0f;
        TimeOfDay += TimeIncrement;
        
        // Wrap around 24 hours
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }

        UpdateLighting();
    }
}

void ADynamicLightingSystem::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateLighting();
}

void ADynamicLightingSystem::SetLightingPreset(ELight_TimeOfDay TimeOfDayEnum)
{
    FLight_LightingPreset Preset = GetLightingPresetForTime(TimeOfDayEnum);
    
    if (SunLightComponent)
    {
        SunLightComponent->SetLightColor(Preset.SunColor);
        SunLightComponent->SetIntensity(Preset.SunIntensity);
        SetActorRotation(Preset.SunRotation);
    }

    if (SkyLightComponent)
    {
        SkyLightComponent->SetLightColor(Preset.SkyColor);
        SkyLightComponent->SetIntensity(Preset.SkyIntensity);
        SkyLightComponent->RecaptureSky();
    }
}

ELight_TimeOfDay ADynamicLightingSystem::GetCurrentTimeOfDay() const
{
    if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (TimeOfDay >= 7.0f && TimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (TimeOfDay >= 11.0f && TimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (TimeOfDay >= 15.0f && TimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ADynamicLightingSystem::UpdateLighting()
{
    if (!SunLightComponent || !SkyLightComponent)
        return;

    ELight_TimeOfDay CurrentTime = GetCurrentTimeOfDay();
    
    // Calculate sun angle based on time of day
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f - 90.0f; // -90 at sunrise, 0 at noon, 90 at sunset
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SetActorRotation(SunRotation);

    // Get current and next presets for interpolation
    FLight_LightingPreset CurrentPreset;
    FLight_LightingPreset NextPreset;
    float Alpha = 0.0f;

    // Determine interpolation based on time
    if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f) // Dawn
    {
        CurrentPreset = NightPreset;
        NextPreset = DawnPreset;
        Alpha = (TimeOfDay - 5.0f) / 2.0f;
    }
    else if (TimeOfDay >= 7.0f && TimeOfDay < 11.0f) // Morning
    {
        CurrentPreset = DawnPreset;
        NextPreset = MorningPreset;
        Alpha = (TimeOfDay - 7.0f) / 4.0f;
    }
    else if (TimeOfDay >= 11.0f && TimeOfDay < 15.0f) // Midday
    {
        CurrentPreset = MorningPreset;
        NextPreset = MiddayPreset;
        Alpha = (TimeOfDay - 11.0f) / 4.0f;
    }
    else if (TimeOfDay >= 15.0f && TimeOfDay < 18.0f) // Afternoon
    {
        CurrentPreset = MiddayPreset;
        NextPreset = AfternoonPreset;
        Alpha = (TimeOfDay - 15.0f) / 3.0f;
    }
    else if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f) // Dusk
    {
        CurrentPreset = AfternoonPreset;
        NextPreset = DuskPreset;
        Alpha = (TimeOfDay - 18.0f) / 2.0f;
    }
    else // Night
    {
        CurrentPreset = DuskPreset;
        NextPreset = NightPreset;
        if (TimeOfDay >= 20.0f)
            Alpha = FMath::Min((TimeOfDay - 20.0f) / 4.0f, 1.0f);
        else
            Alpha = (TimeOfDay + 4.0f) / 4.0f; // Early morning hours
    }

    InterpolateLighting(CurrentPreset, NextPreset, Alpha);
}

void ADynamicLightingSystem::ApplyCretaceousAtmosphere()
{
    if (SunLightComponent)
    {
        // Warm tropical sunlight typical of Cretaceous period
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
        SunLightComponent->SetIntensity(5.0f);
        SunLightComponent->SetCastShadows(true);
        SunLightComponent->SetCastVolumetricShadow(true);
        SunLightComponent->SetVolumetricScatteringIntensity(1.0f);
    }

    if (SkyLightComponent)
    {
        // Clear tropical sky
        SkyLightComponent->SetLightColor(FLinearColor(0.5f, 0.7f, 1.0f, 1.0f));
        SkyLightComponent->SetIntensity(1.0f);
        SkyLightComponent->RecaptureSky();
    }

    UE_LOG(LogTemp, Warning, TEXT("Applied Cretaceous atmosphere settings"));
}

void ADynamicLightingSystem::InterpolateLighting(const FLight_LightingPreset& PresetA, const FLight_LightingPreset& PresetB, float Alpha)
{
    if (!SunLightComponent || !SkyLightComponent)
        return;

    // Interpolate sun properties
    FLinearColor InterpolatedSunColor = UKismetMathLibrary::LinearColorLerp(PresetA.SunColor, PresetB.SunColor, Alpha);
    float InterpolatedSunIntensity = FMath::Lerp(PresetA.SunIntensity, PresetB.SunIntensity, Alpha);

    SunLightComponent->SetLightColor(InterpolatedSunColor);
    SunLightComponent->SetIntensity(InterpolatedSunIntensity);

    // Interpolate sky properties
    FLinearColor InterpolatedSkyColor = UKismetMathLibrary::LinearColorLerp(PresetA.SkyColor, PresetB.SkyColor, Alpha);
    float InterpolatedSkyIntensity = FMath::Lerp(PresetA.SkyIntensity, PresetB.SkyIntensity, Alpha);

    SkyLightComponent->SetLightColor(InterpolatedSkyColor);
    SkyLightComponent->SetIntensity(InterpolatedSkyIntensity);

    // Recapture sky lighting periodically
    static float LastRecaptureTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRecaptureTime > 5.0f) // Recapture every 5 seconds
    {
        SkyLightComponent->RecaptureSky();
        LastRecaptureTime = CurrentTime;
    }
}

FLight_LightingPreset ADynamicLightingSystem::GetLightingPresetForTime(ELight_TimeOfDay TimeOfDayEnum) const
{
    switch (TimeOfDayEnum)
    {
        case ELight_TimeOfDay::Dawn:
            return DawnPreset;
        case ELight_TimeOfDay::Morning:
            return MorningPreset;
        case ELight_TimeOfDay::Midday:
            return MiddayPreset;
        case ELight_TimeOfDay::Afternoon:
            return AfternoonPreset;
        case ELight_TimeOfDay::Dusk:
            return DuskPreset;
        case ELight_TimeOfDay::Night:
            return NightPreset;
        default:
            return MiddayPreset;
    }
}

void ADynamicLightingSystem::InitializeLightingPresets()
{
    // Dawn - soft orange/pink light
    DawnPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    DawnPreset.SkyIntensity = 0.5f;

    // Morning - warm yellow light
    MorningPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningPreset.SunIntensity = 4.0f;
    MorningPreset.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    MorningPreset.SkyIntensity = 0.8f;

    // Midday - bright white/yellow light
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    MiddayPreset.SunIntensity = 6.0f;
    MiddayPreset.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MiddayPreset.SkyIntensity = 1.0f;

    // Afternoon - warm golden light
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AfternoonPreset.SunIntensity = 4.5f;
    AfternoonPreset.SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    AfternoonPreset.SkyIntensity = 0.9f;

    // Dusk - deep orange/red light
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    DuskPreset.SunIntensity = 2.0f;
    DuskPreset.SkyColor = FLinearColor(0.9f, 0.5f, 0.7f, 1.0f);
    DuskPreset.SkyIntensity = 0.4f;

    // Night - cool blue moonlight
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SkyColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightPreset.SkyIntensity = 0.2f;
}