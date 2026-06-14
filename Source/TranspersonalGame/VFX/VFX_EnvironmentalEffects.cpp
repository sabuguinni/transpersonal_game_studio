#include "VFX_EnvironmentalEffects.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AVFX_EnvironmentalEffects::AVFX_EnvironmentalEffects()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Niagara components
    WeatherEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeatherEffect"));
    RootComponent = WeatherEffect;

    AmbientParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AmbientParticles"));
    AmbientParticles->SetupAttachment(RootComponent);

    // Initialize default settings
    EnvironmentalSettings.Intensity = 1.0f;
    EnvironmentalSettings.WindStrength = 0.5f;
    EnvironmentalSettings.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    EnvironmentalSettings.bIsActive = true;

    CurrentWeatherType = EVFX_WeatherType::Clear;
    EffectRadius = 2000.0f;
    TransitionDuration = 5.0f;
}

void AVFX_EnvironmentalEffects::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherSystems();
    UpdateWeatherEffect();
}

void AVFX_EnvironmentalEffects::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentWeatherType = TargetWeatherType;
            OnTransitionComplete();
        }
        
        // Update transition blend
        if (WeatherEffect && WeatherEffect->GetAsset())
        {
            WeatherEffect->SetFloatParameter(TEXT("TransitionBlend"), TransitionProgress);
        }
    }

    // Update wind parameters
    if (WeatherEffect && EnvironmentalSettings.bIsActive)
    {
        WeatherEffect->SetVectorParameter(TEXT("WindDirection"), EnvironmentalSettings.WindDirection);
        WeatherEffect->SetFloatParameter(TEXT("WindStrength"), EnvironmentalSettings.WindStrength);
        WeatherEffect->SetFloatParameter(TEXT("EffectIntensity"), EnvironmentalSettings.Intensity);
    }
}

void AVFX_EnvironmentalEffects::InitializeWeatherSystems()
{
    // Load default Niagara systems (would be set in Blueprint or loaded from assets)
    // For now, we'll set up the component structure
    
    if (WeatherEffect)
    {
        WeatherEffect->SetAutoActivate(false);
        WeatherEffect->SetVisibility(EnvironmentalSettings.bIsActive);
    }

    if (AmbientParticles)
    {
        AmbientParticles->SetAutoActivate(true);
        AmbientParticles->SetVisibility(true);
    }
}

void AVFX_EnvironmentalEffects::UpdateWeatherEffect()
{
    if (!WeatherEffect)
    {
        return;
    }

    UNiagaraSystem* SystemToUse = nullptr;

    switch (CurrentWeatherType)
    {
        case EVFX_WeatherType::Rain:
            SystemToUse = RainSystem;
            break;
        case EVFX_WeatherType::Mist:
            SystemToUse = MistSystem;
            break;
        case EVFX_WeatherType::Dust:
            SystemToUse = DustSystem;
            break;
        case EVFX_WeatherType::VolcanicAsh:
            SystemToUse = VolcanicAshSystem;
            break;
        case EVFX_WeatherType::Clear:
        default:
            SystemToUse = nullptr;
            break;
    }

    if (SystemToUse)
    {
        WeatherEffect->SetAsset(SystemToUse);
        WeatherEffect->Activate();
        WeatherEffect->SetVisibility(true);
    }
    else
    {
        WeatherEffect->Deactivate();
        WeatherEffect->SetVisibility(false);
    }
}

void AVFX_EnvironmentalEffects::SetWeatherType(EVFX_WeatherType NewWeatherType)
{
    if (CurrentWeatherType != NewWeatherType)
    {
        CurrentWeatherType = NewWeatherType;
        UpdateWeatherEffect();
    }
}

void AVFX_EnvironmentalEffects::UpdateEnvironmentalSettings(const FVFX_EnvironmentalSettings& NewSettings)
{
    EnvironmentalSettings = NewSettings;
    
    if (WeatherEffect)
    {
        WeatherEffect->SetVisibility(EnvironmentalSettings.bIsActive);
        if (!EnvironmentalSettings.bIsActive)
        {
            WeatherEffect->Deactivate();
        }
        else if (CurrentWeatherType != EVFX_WeatherType::Clear)
        {
            WeatherEffect->Activate();
        }
    }
}

void AVFX_EnvironmentalEffects::StartWeatherTransition(EVFX_WeatherType TargetWeather)
{
    if (TargetWeather == CurrentWeatherType || bIsTransitioning)
    {
        return;
    }

    TargetWeatherType = TargetWeather;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;

    // Set up transition parameters
    if (WeatherEffect)
    {
        WeatherEffect->SetFloatParameter(TEXT("TransitionBlend"), 0.0f);
    }
}

void AVFX_EnvironmentalEffects::StopAllEffects()
{
    if (WeatherEffect)
    {
        WeatherEffect->Deactivate();
        WeatherEffect->SetVisibility(false);
    }

    if (AmbientParticles)
    {
        AmbientParticles->Deactivate();
        AmbientParticles->SetVisibility(false);
    }

    bIsTransitioning = false;
    GetWorldTimerManager().ClearTimer(TransitionTimer);
}

void AVFX_EnvironmentalEffects::OnTransitionComplete()
{
    UpdateWeatherEffect();
    
    if (WeatherEffect)
    {
        WeatherEffect->SetFloatParameter(TEXT("TransitionBlend"), 1.0f);
    }
}