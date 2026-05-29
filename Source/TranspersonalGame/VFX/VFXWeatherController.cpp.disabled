#include "VFXWeatherController.h"
#include "Components/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AVFX_WeatherController::AVFX_WeatherController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("WeatherRoot"));
    
    // Initialize particle system components
    RainParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainParticles"));
    RainParticles->SetupAttachment(RootComponent);
    RainParticles->bAutoActivate = false;
    
    FogParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FogParticles"));
    FogParticles->SetupAttachment(RootComponent);
    FogParticles->bAutoActivate = false;
    
    SandstormParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SandstormParticles"));
    SandstormParticles->SetupAttachment(RootComponent);
    SandstormParticles->bAutoActivate = false;
    
    SnowParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SnowParticles"));
    SnowParticles->SetupAttachment(RootComponent);
    SnowParticles->bAutoActivate = false;
    
    // Initialize default weather settings
    CurrentWeather.WeatherType = EVFX_WeatherType::Clear;
    CurrentWeather.Intensity = 0.0f;
    CurrentWeather.Duration = 300.0f;
    CurrentWeather.AffectedArea = FVector(10000.0f, 10000.0f, 2000.0f);
    
    bAutoWeatherCycle = true;
    WeatherTransitionSpeed = 1.0f;
    CurrentWeatherTimer = 0.0f;
    bIsTransitioning = false;
}

void AVFX_WeatherController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with clear weather
    SetWeather(EVFX_WeatherType::Clear, 0.0f, 600.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Weather Controller initialized - Auto cycle: %s"), 
           bAutoWeatherCycle ? TEXT("ON") : TEXT("OFF"));
}

void AVFX_WeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateWeatherEffects(DeltaTime);
    
    if (bAutoWeatherCycle)
    {
        ProcessAutoWeatherCycle(DeltaTime);
    }
}

void AVFX_WeatherController::SetWeather(EVFX_WeatherType NewWeatherType, float Intensity, float Duration)
{
    FVFX_WeatherSettings NewSettings;
    NewSettings.WeatherType = NewWeatherType;
    NewSettings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewSettings.Duration = Duration;
    NewSettings.AffectedArea = CurrentWeather.AffectedArea;
    
    StartWeatherTransition(NewSettings);
}

void AVFX_WeatherController::StartWeatherTransition(const FVFX_WeatherSettings& NewWeatherSettings)
{
    CurrentWeather = NewWeatherSettings;
    CurrentWeatherTimer = 0.0f;
    bIsTransitioning = true;
    
    // Immediately activate new weather particles
    ActivateWeatherParticles(CurrentWeather.WeatherType, CurrentWeather.Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Weather transition started: %d, Intensity: %f"), 
           (int32)CurrentWeather.WeatherType, CurrentWeather.Intensity);
}

void AVFX_WeatherController::StopAllWeatherEffects()
{
    DeactivateAllParticles();
    CurrentWeather.WeatherType = EVFX_WeatherType::Clear;
    CurrentWeather.Intensity = 0.0f;
    bIsTransitioning = false;
}

void AVFX_WeatherController::SetWeatherIntensity(float NewIntensity)
{
    CurrentWeather.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    // Update particle intensity immediately
    if (CurrentWeather.WeatherType != EVFX_WeatherType::Clear)
    {
        ActivateWeatherParticles(CurrentWeather.WeatherType, CurrentWeather.Intensity);
    }
}

void AVFX_WeatherController::UpdateWeatherEffects(float DeltaTime)
{
    CurrentWeatherTimer += DeltaTime;
    
    // Check if weather duration has expired
    if (CurrentWeatherTimer >= CurrentWeather.Duration && CurrentWeather.WeatherType != EVFX_WeatherType::Clear)
    {
        if (bAutoWeatherCycle)
        {
            // Transition to new random weather
            EVFX_WeatherType NewWeather = GetRandomWeatherType();
            SetWeather(NewWeather, FMath::RandRange(0.3f, 0.8f), FMath::RandRange(180.0f, 600.0f));
        }
        else
        {
            // Return to clear weather
            SetWeather(EVFX_WeatherType::Clear, 0.0f, 300.0f);
        }
    }
    
    // Handle weather transition effects
    if (bIsTransitioning)
    {
        float TransitionProgress = FMath::Clamp(CurrentWeatherTimer / 30.0f, 0.0f, 1.0f); // 30 second transition
        
        if (TransitionProgress >= 1.0f)
        {
            bIsTransitioning = false;
        }
    }
}

void AVFX_WeatherController::ActivateWeatherParticles(EVFX_WeatherType WeatherType, float Intensity)
{
    // First deactivate all particles
    DeactivateAllParticles();
    
    // Activate appropriate particle system based on weather type
    switch (WeatherType)
    {
        case EVFX_WeatherType::Rain:
        case EVFX_WeatherType::Storm:
            if (RainParticles)
            {
                RainParticles->Activate(true);
                UpdateParticleIntensity(RainParticles, Intensity);
            }
            break;
            
        case EVFX_WeatherType::Fog:
            if (FogParticles)
            {
                FogParticles->Activate(true);
                UpdateParticleIntensity(FogParticles, Intensity);
            }
            break;
            
        case EVFX_WeatherType::Sandstorm:
            if (SandstormParticles)
            {
                SandstormParticles->Activate(true);
                UpdateParticleIntensity(SandstormParticles, Intensity);
            }
            break;
            
        case EVFX_WeatherType::Snow:
            if (SnowParticles)
            {
                SnowParticles->Activate(true);
                UpdateParticleIntensity(SnowParticles, Intensity);
            }
            break;
            
        case EVFX_WeatherType::Clear:
        default:
            // All particles already deactivated
            break;
    }
}

void AVFX_WeatherController::DeactivateAllParticles()
{
    if (RainParticles) RainParticles->Deactivate();
    if (FogParticles) FogParticles->Deactivate();
    if (SandstormParticles) SandstormParticles->Deactivate();
    if (SnowParticles) SnowParticles->Deactivate();
}

void AVFX_WeatherController::UpdateParticleIntensity(UParticleSystemComponent* ParticleComp, float Intensity)
{
    if (!ParticleComp) return;
    
    // Scale particle emission rate based on intensity
    float EmissionRate = FMath::Lerp(50.0f, 500.0f, Intensity);
    ParticleComp->SetFloatParameter(FName("EmissionRate"), EmissionRate);
    
    // Scale particle size based on intensity
    float ParticleSize = FMath::Lerp(0.5f, 2.0f, Intensity);
    ParticleComp->SetFloatParameter(FName("ParticleSize"), ParticleSize);
    
    // Scale velocity based on intensity
    float Velocity = FMath::Lerp(100.0f, 800.0f, Intensity);
    ParticleComp->SetFloatParameter(FName("Velocity"), Velocity);
}

void AVFX_WeatherController::ProcessAutoWeatherCycle(float DeltaTime)
{
    // Auto weather cycle logic - change weather every 5-10 minutes
    static float NextWeatherChangeTime = FMath::RandRange(300.0f, 600.0f);
    
    if (CurrentWeatherTimer >= NextWeatherChangeTime)
    {
        EVFX_WeatherType NewWeather = GetBiomeAppropriateWeather();
        float NewIntensity = FMath::RandRange(0.2f, 0.8f);
        float NewDuration = FMath::RandRange(180.0f, 480.0f);
        
        SetWeather(NewWeather, NewIntensity, NewDuration);
        
        // Set next weather change time
        NextWeatherChangeTime = FMath::RandRange(300.0f, 600.0f);
    }
}

EVFX_WeatherType AVFX_WeatherController::GetRandomWeatherType() const
{
    int32 RandomWeather = FMath::RandRange(0, 5);
    
    switch (RandomWeather)
    {
        case 0: return EVFX_WeatherType::Clear;
        case 1: return EVFX_WeatherType::Rain;
        case 2: return EVFX_WeatherType::Storm;
        case 3: return EVFX_WeatherType::Fog;
        case 4: return EVFX_WeatherType::Sandstorm;
        case 5: return EVFX_WeatherType::Snow;
        default: return EVFX_WeatherType::Clear;
    }
}

EVFX_WeatherType AVFX_WeatherController::GetBiomeAppropriateWeather() const
{
    EBiomeType CurrentBiome = GetCurrentBiome();
    
    switch (CurrentBiome)
    {
        case EBiomeType::Forest:
        case EBiomeType::Swamp:
            // Forest and swamp prefer rain and fog
            return FMath::RandBool() ? EVFX_WeatherType::Rain : EVFX_WeatherType::Fog;
            
        case EBiomeType::Desert:
            // Desert prefers sandstorms or clear weather
            return FMath::RandBool() ? EVFX_WeatherType::Sandstorm : EVFX_WeatherType::Clear;
            
        case EBiomeType::Mountain:
            // Mountains can have snow or storms
            return FMath::RandBool() ? EVFX_WeatherType::Snow : EVFX_WeatherType::Storm;
            
        case EBiomeType::Savanna:
        default:
            // Savanna has varied weather
            return GetRandomWeatherType();
    }
}

EBiomeType AVFX_WeatherController::GetCurrentBiome() const
{
    FVector ActorLocation = GetActorLocation();
    
    // Determine biome based on world coordinates (from SharedTypes.h biome system)
    if (ActorLocation.X >= 40000 && ActorLocation.Y >= 40000)
    {
        return EBiomeType::Mountain;
    }
    else if (ActorLocation.X >= 50000 && FMath::Abs(ActorLocation.Y) <= 10000)
    {
        return EBiomeType::Desert;
    }
    else if (ActorLocation.X <= -40000 && ActorLocation.Y >= 35000)
    {
        return EBiomeType::Forest;
    }
    else if (ActorLocation.X <= -45000 && ActorLocation.Y <= -40000)
    {
        return EBiomeType::Swamp;
    }
    else
    {
        return EBiomeType::Savanna;
    }
}