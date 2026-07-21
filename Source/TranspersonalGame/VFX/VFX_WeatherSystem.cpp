#include "VFX_WeatherSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

UVFX_WeatherSystem::UVFX_WeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default weather settings
    CurrentWeatherSettings.WeatherType = EVFX_WeatherType::Clear;
    CurrentWeatherSettings.Intensity = 0.0f;
    CurrentWeatherSettings.Duration = 0.0f;
    CurrentWeatherSettings.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    CurrentWeatherSettings.LightIntensity = 3.0f;
    CurrentWeatherSettings.FogDensity = 0.02f;
    CurrentWeatherSettings.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    CurrentWeatherSettings.WindStrength = 10.0f;
    
    TargetWeatherSettings = CurrentWeatherSettings;
    
    // Lightning settings
    LightningMinInterval = 5.0f;
    LightningMaxInterval = 15.0f;
    NextLightningTime = 0.0f;
}

void UVFX_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEnvironmentReferences();
    
    // Create audio component for weather sounds
    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->bAutoActivate = false;
        WeatherAudioComponent->SetVolumeMultiplier(0.7f);
    }
}

void UVFX_WeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update weather transition
    if (bTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
    
    // Update weather timer
    if (bWeatherActive && CurrentWeatherSettings.Duration > 0.0f)
    {
        WeatherTimer += DeltaTime;
        if (WeatherTimer >= CurrentWeatherSettings.Duration)
        {
            StopCurrentWeather();
        }
    }
    
    // Handle lightning for storm weather
    if (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::Storm)
    {
        HandleLightning(DeltaTime);
    }
}

void UVFX_WeatherSystem::SetWeather(EVFX_WeatherType NewWeatherType, float Intensity, float Duration)
{
    FVFX_WeatherSettings NewSettings;
    NewSettings.WeatherType = NewWeatherType;
    NewSettings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewSettings.Duration = Duration;
    
    // Set weather-specific parameters
    switch (NewWeatherType)
    {
        case EVFX_WeatherType::Clear:
            NewSettings.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
            NewSettings.LightIntensity = 3.0f;
            NewSettings.FogDensity = 0.02f;
            break;
            
        case EVFX_WeatherType::LightRain:
        case EVFX_WeatherType::HeavyRain:
            NewSettings.SkyColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
            NewSettings.LightIntensity = 1.5f;
            NewSettings.FogDensity = 0.05f;
            break;
            
        case EVFX_WeatherType::Storm:
            NewSettings.SkyColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);
            NewSettings.LightIntensity = 1.0f;
            NewSettings.FogDensity = 0.08f;
            break;
            
        case EVFX_WeatherType::Fog:
            NewSettings.SkyColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
            NewSettings.LightIntensity = 2.0f;
            NewSettings.FogDensity = 0.15f;
            break;
            
        case EVFX_WeatherType::Snow:
            NewSettings.SkyColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            NewSettings.LightIntensity = 2.5f;
            NewSettings.FogDensity = 0.03f;
            break;
            
        case EVFX_WeatherType::Dust:
            NewSettings.SkyColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
            NewSettings.LightIntensity = 1.8f;
            NewSettings.FogDensity = 0.12f;
            break;
            
        case EVFX_WeatherType::Wind:
            NewSettings.SkyColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
            NewSettings.LightIntensity = 2.8f;
            NewSettings.FogDensity = 0.025f;
            NewSettings.WindStrength = 25.0f;
            break;
    }
    
    TransitionToWeather(NewSettings, 3.0f);
}

void UVFX_WeatherSystem::TransitionToWeather(const FVFX_WeatherSettings& NewSettings, float TransitionTime)
{
    TargetWeatherSettings = NewSettings;
    TransitionDuration = TransitionTime;
    TransitionProgress = 0.0f;
    bTransitioning = true;
    bWeatherActive = true;
    WeatherTimer = 0.0f;
    
    // Start weather effect immediately if not transitioning from another weather
    if (CurrentWeatherSettings.WeatherType == EVFX_WeatherType::Clear)
    {
        SpawnWeatherEffect(NewSettings.WeatherType, NewSettings.Intensity);
    }
}

void UVFX_WeatherSystem::StopCurrentWeather()
{
    FVFX_WeatherSettings ClearSettings;
    ClearSettings.WeatherType = EVFX_WeatherType::Clear;
    ClearSettings.Intensity = 0.0f;
    ClearSettings.Duration = 0.0f;
    
    TransitionToWeather(ClearSettings, 2.0f);
    bWeatherActive = false;
}

void UVFX_WeatherSystem::TriggerLightning(FVector Location)
{
    if (!LightningSystem.IsNull())
    {
        UWorld* World = GetWorld();
        if (World)
        {
            if (Location == FVector::ZeroVector)
            {
                // Random location around player
                APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
                if (PlayerPawn)
                {
                    FVector PlayerLocation = PlayerPawn->GetActorLocation();
                    float RandomRadius = FMath::RandRange(500.0f, 2000.0f);
                    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
                    Location = PlayerLocation + FVector(
                        FMath::Cos(RandomAngle) * RandomRadius,
                        FMath::Sin(RandomAngle) * RandomRadius,
                        FMath::RandRange(500.0f, 1500.0f)
                    );
                }
            }
            
            // Spawn lightning effect
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World,
                LightningSystem.LoadSynchronous(),
                Location,
                FRotator::ZeroRotator,
                FVector::OneVector,
                true,
                true,
                ENCPoolMethod::None,
                true
            );
            
            // Flash the directional light
            if (SunLight && SunLight->GetLightComponent())
            {
                UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
                float OriginalIntensity = LightComp->Intensity;
                LightComp->SetIntensity(OriginalIntensity * 3.0f);
                
                // Reset intensity after brief flash
                FTimerHandle LightningTimer;
                World->GetTimerManager().SetTimer(LightningTimer, [this, LightComp, OriginalIntensity]()
                {
                    if (LightComp)
                    {
                        LightComp->SetIntensity(OriginalIntensity);
                    }
                }, 0.1f, false);
            }
        }
    }
}

void UVFX_WeatherSystem::SetLightningFrequency(float MinInterval, float MaxInterval)
{
    LightningMinInterval = FMath::Max(MinInterval, 1.0f);
    LightningMaxInterval = FMath::Max(MaxInterval, LightningMinInterval);
}

void UVFX_WeatherSystem::InitializeEnvironmentReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }
    
    // Find sky light
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyLight = *ActorItr;
        break;
    }
    
    // Find height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        HeightFog = *ActorItr;
        break;
    }
}

void UVFX_WeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bTransitioning = false;
        CurrentWeatherSettings = TargetWeatherSettings;
        
        // Update weather effects
        UpdateWeatherEffects();
    }
    
    // Interpolate between current and target settings
    float Alpha = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);
    ApplyWeatherSettings(TargetWeatherSettings, Alpha);
}

void UVFX_WeatherSystem::ApplyWeatherSettings(const FVFX_WeatherSettings& Settings, float Alpha)
{
    UpdateLighting(Settings, Alpha);
    UpdateFog(Settings, Alpha);
}

void UVFX_WeatherSystem::UpdateLighting(const FVFX_WeatherSettings& Settings, float Alpha)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        float NewIntensity = FMath::Lerp(CurrentWeatherSettings.LightIntensity, Settings.LightIntensity, Alpha);
        LightComp->SetIntensity(NewIntensity);
    }
    
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        FLinearColor NewColor = FMath::Lerp(CurrentWeatherSettings.SkyColor, Settings.SkyColor, Alpha);
        SkyComp->SetLightColor(NewColor);
    }
}

void UVFX_WeatherSystem::UpdateFog(const FVFX_WeatherSettings& Settings, float Alpha)
{
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        float NewDensity = FMath::Lerp(CurrentWeatherSettings.FogDensity, Settings.FogDensity, Alpha);
        FogComp->SetFogDensity(NewDensity);
    }
}

void UVFX_WeatherSystem::UpdateWeatherEffects()
{
    // Stop current weather effect
    StopWeatherEffect();
    
    // Start new weather effect
    if (CurrentWeatherSettings.WeatherType != EVFX_WeatherType::Clear)
    {
        SpawnWeatherEffect(CurrentWeatherSettings.WeatherType, CurrentWeatherSettings.Intensity);
    }
}

void UVFX_WeatherSystem::HandleLightning(float DeltaTime)
{
    if (NextLightningTime <= 0.0f)
    {
        // Set next lightning time
        NextLightningTime = FMath::RandRange(LightningMinInterval, LightningMaxInterval);
        TriggerLightning();
    }
    else
    {
        NextLightningTime -= DeltaTime;
    }
}

UNiagaraSystem* UVFX_WeatherSystem::GetWeatherSystem(EVFX_WeatherType WeatherType)
{
    switch (WeatherType)
    {
        case EVFX_WeatherType::LightRain:
        case EVFX_WeatherType::HeavyRain:
        case EVFX_WeatherType::Storm:
            return RainSystem.LoadSynchronous();
            
        case EVFX_WeatherType::Snow:
            return SnowSystem.LoadSynchronous();
            
        case EVFX_WeatherType::Fog:
            return FogSystem.LoadSynchronous();
            
        case EVFX_WeatherType::Dust:
            return DustStormSystem.LoadSynchronous();
            
        case EVFX_WeatherType::Wind:
            return WindSystem.LoadSynchronous();
            
        default:
            return nullptr;
    }
}

void UVFX_WeatherSystem::SpawnWeatherEffect(EVFX_WeatherType WeatherType, float Intensity)
{
    UNiagaraSystem* WeatherSystem = GetWeatherSystem(WeatherType);
    if (!WeatherSystem) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create weather component
    ActiveWeatherComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        WeatherSystem,
        GetOwner()->GetRootComponent(),
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepWorldPosition,
        true
    );
    
    if (ActiveWeatherComponent)
    {
        // Set intensity parameter
        ActiveWeatherComponent->SetFloatParameter(FName("Intensity"), Intensity);
        
        // Set wind parameters
        ActiveWeatherComponent->SetVectorParameter(FName("WindDirection"), CurrentWeatherSettings.WindDirection);
        ActiveWeatherComponent->SetFloatParameter(FName("WindStrength"), CurrentWeatherSettings.WindStrength);
    }
}

void UVFX_WeatherSystem::StopWeatherEffect()
{
    if (ActiveWeatherComponent)
    {
        ActiveWeatherComponent->DestroyComponent();
        ActiveWeatherComponent = nullptr;
    }
}