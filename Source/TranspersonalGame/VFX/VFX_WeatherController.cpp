#include "VFX_WeatherController.h"
#include "NiagaraComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"

AVFX_WeatherController::AVFX_WeatherController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create sky light component
    SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLightComponent"));
    SkyLightComponent->SetupAttachment(RootComponent);

    // Create rain particle component
    RainParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainParticleComponent"));
    RainParticleComponent->SetupAttachment(RootComponent);

    // Create fog particle component
    FogParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogParticleComponent"));
    FogParticleComponent->SetupAttachment(RootComponent);

    // Initialize default weather settings
    CurrentWeatherSettings.WeatherType = EVFX_WeatherType::Clear;
    CurrentWeatherSettings.Intensity = 0.5f;
    CurrentWeatherSettings.TransitionDuration = 5.0f;
    CurrentWeatherSettings.SkyTint = FLinearColor::White;
    CurrentWeatherSettings.Visibility = 1.0f;

    // Initialize transition state
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 5.0f;

    // Initialize weather cycle
    WeatherCycleTimer = 0.0f;
    CurrentWeatherIndex = 0;
    bAutoWeatherCycle = false;
    WeatherCycleDuration = 300.0f;

    // Set up weather cycle sequence
    WeatherCycleSequence.Add(EVFX_WeatherType::Clear);
    WeatherCycleSequence.Add(EVFX_WeatherType::LightRain);
    WeatherCycleSequence.Add(EVFX_WeatherType::Clear);
    WeatherCycleSequence.Add(EVFX_WeatherType::Fog);
    WeatherCycleSequence.Add(EVFX_WeatherType::Clear);
    WeatherCycleSequence.Add(EVFX_WeatherType::HeavyRain);
}

void AVFX_WeatherController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherSystems();
    UpdateWeatherEffects();
    
    if (bAutoWeatherCycle)
    {
        StartWeatherCycle();
    }
}

void AVFX_WeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Process weather transitions
    if (bIsTransitioning)
    {
        ProcessWeatherTransition(DeltaTime);
    }

    // Process automatic weather cycling
    if (bAutoWeatherCycle)
    {
        WeatherCycleTimer += DeltaTime;
        if (WeatherCycleTimer >= WeatherCycleDuration)
        {
            CycleWeather();
            WeatherCycleTimer = 0.0f;
        }
    }

    // Update weather effects
    UpdateWeatherEffects();
}

void AVFX_WeatherController::SetWeather(EVFX_WeatherType NewWeatherType, float Intensity, float TransitionTime)
{
    if (NewWeatherType == CurrentWeatherSettings.WeatherType && 
        FMath::IsNearlyEqual(Intensity, CurrentWeatherSettings.Intensity, 0.01f))
    {
        return; // No change needed
    }

    if (TransitionTime <= 0.0f)
    {
        // Immediate change
        CurrentWeatherSettings.WeatherType = NewWeatherType;
        CurrentWeatherSettings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        UpdateWeatherEffects();
        OnWeatherChanged(NewWeatherType, CurrentWeatherSettings.Intensity);
    }
    else
    {
        // Gradual transition
        TransitionToWeather(NewWeatherType, TransitionTime);
    }
}

void AVFX_WeatherController::TransitionToWeather(EVFX_WeatherType TargetWeatherType, float TransitionDuration)
{
    if (bIsTransitioning && TransitionToWeather == TargetWeatherType)
    {
        return; // Already transitioning to this weather
    }

    TransitionFromWeather = CurrentWeatherSettings.WeatherType;
    TransitionToWeather = TargetWeatherType;
    this->TransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    OnWeatherTransitionStarted(TransitionFromWeather, TransitionToWeather);
    
    UE_LOG(LogTemp, Log, TEXT("WeatherController: Starting transition from %d to %d over %.1f seconds"), 
           (int32)TransitionFromWeather, (int32)TransitionToWeather, this->TransitionDuration);
}

void AVFX_WeatherController::StartWeatherCycle()
{
    bAutoWeatherCycle = true;
    WeatherCycleTimer = 0.0f;
    CurrentWeatherIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("WeatherController: Started automatic weather cycle"));
}

void AVFX_WeatherController::StopWeatherCycle()
{
    bAutoWeatherCycle = false;
    WeatherCycleTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("WeatherController: Stopped automatic weather cycle"));
}

void AVFX_WeatherController::InitializeWeatherSystems()
{
    // Initialize weather particle systems map
    WeatherParticleSystems.Empty();
    
    // Placeholder entries - actual Niagara assets would be assigned in Blueprint or loaded at runtime
    WeatherParticleSystems.Add(EVFX_WeatherType::Clear, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::LightRain, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::HeavyRain, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::Storm, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::Fog, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::Dust, nullptr);
    WeatherParticleSystems.Add(EVFX_WeatherType::Snow, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("WeatherController: Initialized weather systems with %d entries"), 
           WeatherParticleSystems.Num());
}

void AVFX_WeatherController::UpdateWeatherEffects()
{
    UpdateLighting();
    UpdateParticleEffects();
}

void AVFX_WeatherController::UpdateLighting()
{
    if (!SkyLightComponent)
    {
        return;
    }

    // Adjust sky light based on weather
    float LightIntensity = 1.0f;
    FLinearColor LightColor = FLinearColor::White;

    switch (CurrentWeatherSettings.WeatherType)
    {
        case EVFX_WeatherType::Clear:
            LightIntensity = 1.0f;
            LightColor = FLinearColor(1.0f, 1.0f, 0.95f); // Slightly warm
            break;
            
        case EVFX_WeatherType::LightRain:
            LightIntensity = 0.7f;
            LightColor = FLinearColor(0.8f, 0.85f, 0.9f); // Cool blue tint
            break;
            
        case EVFX_WeatherType::HeavyRain:
            LightIntensity = 0.4f;
            LightColor = FLinearColor(0.6f, 0.7f, 0.8f); // Darker blue-gray
            break;
            
        case EVFX_WeatherType::Storm:
            LightIntensity = 0.2f;
            LightColor = FLinearColor(0.4f, 0.5f, 0.6f); // Dark stormy
            break;
            
        case EVFX_WeatherType::Fog:
            LightIntensity = 0.5f;
            LightColor = FLinearColor(0.9f, 0.9f, 0.85f); // Muted warm
            break;
            
        case EVFX_WeatherType::Dust:
            LightIntensity = 0.6f;
            LightColor = FLinearColor(1.0f, 0.8f, 0.6f); // Dusty orange
            break;
            
        case EVFX_WeatherType::Snow:
            LightIntensity = 0.8f;
            LightColor = FLinearColor(0.95f, 0.95f, 1.0f); // Cool white
            break;
    }

    // Apply intensity modifier
    LightIntensity *= (0.5f + CurrentWeatherSettings.Intensity * 0.5f);
    
    SkyLightComponent->SetIntensity(LightIntensity);
    SkyLightComponent->SetLightColor(LightColor);
}

void AVFX_WeatherController::UpdateParticleEffects()
{
    // Update rain particles
    if (RainParticleComponent)
    {
        bool bShouldShowRain = (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::LightRain ||
                               CurrentWeatherSettings.WeatherType == EVFX_WeatherType::HeavyRain ||
                               CurrentWeatherSettings.WeatherType == EVFX_WeatherType::Storm);
        
        if (bShouldShowRain && !RainParticleComponent->IsActive())
        {
            RainParticleComponent->Activate();
        }
        else if (!bShouldShowRain && RainParticleComponent->IsActive())
        {
            RainParticleComponent->Deactivate();
        }
        
        if (bShouldShowRain)
        {
            float RainIntensity = CurrentWeatherSettings.Intensity;
            if (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::HeavyRain)
            {
                RainIntensity *= 2.0f;
            }
            else if (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::Storm)
            {
                RainIntensity *= 3.0f;
            }
            
            RainParticleComponent->SetFloatParameter(TEXT("RainIntensity"), RainIntensity);
            RainParticleComponent->SetFloatParameter(TEXT("SpawnRate"), RainIntensity * 1000.0f);
        }
    }

    // Update fog particles
    if (FogParticleComponent)
    {
        bool bShouldShowFog = (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::Fog);
        
        if (bShouldShowFog && !FogParticleComponent->IsActive())
        {
            FogParticleComponent->Activate();
        }
        else if (!bShouldShowFog && FogParticleComponent->IsActive())
        {
            FogParticleComponent->Deactivate();
        }
        
        if (bShouldShowFog)
        {
            FogParticleComponent->SetFloatParameter(TEXT("FogDensity"), CurrentWeatherSettings.Intensity);
            FogParticleComponent->SetFloatParameter(TEXT("Visibility"), CurrentWeatherSettings.Visibility);
        }
    }
}

void AVFX_WeatherController::ProcessWeatherTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentWeatherSettings.WeatherType = TransitionToWeather;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        
        OnWeatherTransitionCompleted(TransitionToWeather);
        OnWeatherChanged(TransitionToWeather, CurrentWeatherSettings.Intensity);
        
        UE_LOG(LogTemp, Log, TEXT("WeatherController: Transition to %d completed"), (int32)TransitionToWeather);
    }
    else
    {
        // Interpolate between weather states
        // This is a simplified transition - in a full implementation, you'd interpolate
        // all weather parameters smoothly
        
        // For now, just update the intensity based on transition progress
        float BaseIntensity = CurrentWeatherSettings.Intensity;
        
        // Adjust intensity during transition
        if (TransitionToWeather == EVFX_WeatherType::Clear)
        {
            CurrentWeatherSettings.Intensity = BaseIntensity * (1.0f - TransitionProgress);
        }
        else if (TransitionFromWeather == EVFX_WeatherType::Clear)
        {
            CurrentWeatherSettings.Intensity = BaseIntensity * TransitionProgress;
        }
    }
}

void AVFX_WeatherController::CycleWeather()
{
    if (WeatherCycleSequence.Num() == 0)
    {
        return;
    }

    CurrentWeatherIndex = (CurrentWeatherIndex + 1) % WeatherCycleSequence.Num();
    EVFX_WeatherType NextWeather = WeatherCycleSequence[CurrentWeatherIndex];
    
    TransitionToWeather(NextWeather, 10.0f); // 10 second transition
    
    UE_LOG(LogTemp, Log, TEXT("WeatherController: Cycling to weather %d (index %d)"), 
           (int32)NextWeather, CurrentWeatherIndex);
}