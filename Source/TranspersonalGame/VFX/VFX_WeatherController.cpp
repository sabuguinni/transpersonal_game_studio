#include "VFX_WeatherController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"

AVFX_WeatherController::AVFX_WeatherController()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    WeatherParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeatherParticleComponent"));
    WeatherParticleComponent->SetupAttachment(RootComponent);

    CurrentWeatherType = EVFX_WeatherType::Clear;
    bAutoChangeWeather = true;
    WeatherChangeInterval = 300.0f;
    TransitionDuration = 30.0f;
    CurrentWeatherTimer = 0.0f;
    bIsTransitioning = false;
}

void AVFX_WeatherController::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateWeatherSystem();
}

void AVFX_WeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoChangeWeather && !bIsTransitioning)
    {
        CurrentWeatherTimer += DeltaTime;
        if (CurrentWeatherTimer >= WeatherChangeInterval)
        {
            EVFX_WeatherType NewWeather = GetRandomWeatherType();
            StartWeatherTransition(NewWeather, TransitionDuration);
            CurrentWeatherTimer = 0.0f;
        }
    }

    if (bIsTransitioning)
    {
        ProcessWeatherTransition(DeltaTime);
    }
}

void AVFX_WeatherController::ChangeWeather(EVFX_WeatherType NewWeatherType)
{
    if (NewWeatherType != CurrentWeatherType)
    {
        CurrentWeatherType = NewWeatherType;
        UpdateWeatherSystem();
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
    }
}

void AVFX_WeatherController::StartWeatherTransition(EVFX_WeatherType TargetWeather, float Duration)
{
    if (TargetWeather != CurrentWeatherType)
    {
        TransitionTargetWeather = TargetWeather;
        TransitionDurationCurrent = Duration;
        TransitionTimer = 0.0f;
        bIsTransitioning = true;
    }
}

void AVFX_WeatherController::SetWeatherIntensity(float NewIntensity)
{
    if (WeatherParticleComponent && WeatherParticleComponent->GetAsset())
    {
        // Set intensity parameter on the Niagara system
        WeatherParticleComponent->SetFloatParameter(TEXT("Intensity"), FMath::Clamp(NewIntensity, 0.0f, 2.0f));
    }
}

EVFX_WeatherType AVFX_WeatherController::GetRandomWeatherType() const
{
    TArray<EVFX_WeatherType> WeatherTypes = {
        EVFX_WeatherType::Clear,
        EVFX_WeatherType::Rain_Light,
        EVFX_WeatherType::Rain_Heavy,
        EVFX_WeatherType::Fog_Light,
        EVFX_WeatherType::Snow_Light
    };

    int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
    return WeatherTypes[RandomIndex];
}

void AVFX_WeatherController::UpdateWeatherSystem()
{
    FVFX_WeatherSettings Settings = GetWeatherSettings(CurrentWeatherType);
    ApplyWeatherSettings(Settings);
}

void AVFX_WeatherController::ProcessWeatherTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDurationCurrent, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        ChangeWeather(TransitionTargetWeather);
        bIsTransitioning = false;
    }
    else
    {
        // Interpolate weather intensity during transition
        float IntensityMultiplier = (CurrentWeatherType == EVFX_WeatherType::Clear) ? TransitionProgress : (1.0f - TransitionProgress);
        SetWeatherIntensity(IntensityMultiplier);
    }
}

FVFX_WeatherSettings AVFX_WeatherController::GetWeatherSettings(EVFX_WeatherType WeatherType) const
{
    for (const FVFX_WeatherSettings& Settings : WeatherConfigurations)
    {
        if (Settings.WeatherType == WeatherType)
        {
            return Settings;
        }
    }

    // Return default settings if not found
    FVFX_WeatherSettings DefaultSettings;
    DefaultSettings.WeatherType = WeatherType;
    return DefaultSettings;
}

void AVFX_WeatherController::ApplyWeatherSettings(const FVFX_WeatherSettings& Settings)
{
    if (WeatherParticleComponent)
    {
        if (Settings.ParticleSystem.IsValid())
        {
            UNiagaraSystem* System = Settings.ParticleSystem.LoadSynchronous();
            if (System)
            {
                WeatherParticleComponent->SetAsset(System);
                WeatherParticleComponent->SetFloatParameter(TEXT("Intensity"), Settings.Intensity);
                WeatherParticleComponent->Activate();
            }
        }
        else
        {
            WeatherParticleComponent->Deactivate();
        }
    }

    UpdateDirectionalLight(Settings);
}

void AVFX_WeatherController::UpdateDirectionalLight(const FVFX_WeatherSettings& Settings)
{
    // Find directional light in the level
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);

    for (AActor* LightActor : DirectionalLights)
    {
        if (ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(LightActor))
        {
            UDirectionalLightComponent* LightComponent = DirectionalLight->GetLightComponent();
            if (LightComponent)
            {
                LightComponent->SetLightColor(Settings.LightColor);
                LightComponent->SetIntensity(Settings.LightIntensity);
            }
        }
    }
}