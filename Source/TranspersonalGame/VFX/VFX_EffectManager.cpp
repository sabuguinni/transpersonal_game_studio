#include "VFX_EffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create Niagara components
    PrimaryEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PrimaryEffect"));
    PrimaryEffectComponent->SetupAttachment(RootComponent);

    SecondaryEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SecondaryEffect"));
    SecondaryEffectComponent->SetupAttachment(RootComponent);

    // Initialize default values
    EffectTimer = 0.0f;
    bEffectActive = false;

    // Set default effect data
    CurrentEffectData.EffectType = EVFX_EffectType::Fire_Campfire;
    CurrentEffectData.Duration = 5.0f;
    CurrentEffectData.Intensity = 1.0f;
    CurrentEffectData.Scale = FVector(1.0f, 1.0f, 1.0f);
    CurrentEffectData.bLooping = false;
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: BeginPlay - Effect manager initialized"));
}

void AVFX_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEffectActive && !CurrentEffectData.bLooping)
    {
        EffectTimer += DeltaTime;
        if (EffectTimer >= CurrentEffectData.Duration)
        {
            StopEffect();
        }
    }

    UpdateEffectParameters();
}

void AVFX_EffectManager::PlayEffect(EVFX_EffectType EffectType, FVector Location, float Intensity)
{
    CurrentEffectData.EffectType = EffectType;
    CurrentEffectData.Intensity = Intensity;
    
    SetActorLocation(Location);
    
    UNiagaraSystem* EffectSystem = GetEffectSystem(EffectType);
    if (EffectSystem && PrimaryEffectComponent)
    {
        PrimaryEffectComponent->SetAsset(EffectSystem);
        PrimaryEffectComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
        PrimaryEffectComponent->SetVectorParameter(TEXT("Scale"), CurrentEffectData.Scale);
        PrimaryEffectComponent->Activate(true);
        
        bEffectActive = true;
        EffectTimer = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Playing effect type %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }
}

void AVFX_EffectManager::StopEffect()
{
    if (PrimaryEffectComponent)
    {
        PrimaryEffectComponent->Deactivate();
    }
    
    if (SecondaryEffectComponent)
    {
        SecondaryEffectComponent->Deactivate();
    }
    
    bEffectActive = false;
    EffectTimer = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect stopped"));
}

void AVFX_EffectManager::SetEffectIntensity(float NewIntensity)
{
    CurrentEffectData.Intensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    
    if (PrimaryEffectComponent && bEffectActive)
    {
        PrimaryEffectComponent->SetFloatParameter(TEXT("Intensity"), CurrentEffectData.Intensity);
    }
}

void AVFX_EffectManager::PlayCampfireEffect(FVector Location)
{
    CurrentEffectData.bLooping = true;
    CurrentEffectData.Duration = -1.0f; // Infinite duration for looping
    PlayEffect(EVFX_EffectType::Fire_Campfire, Location, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Campfire effect started at %s"), *Location.ToString());
}

void AVFX_EffectManager::PlayFootstepDust(FVector Location, float DinosaurSize)
{
    float DustIntensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    CurrentEffectData.bLooping = false;
    CurrentEffectData.Duration = 2.0f;
    PlayEffect(EVFX_EffectType::Dust_Footstep, Location, DustIntensity);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Footstep dust effect - Size: %f"), DinosaurSize);
}

void AVFX_EffectManager::PlayBloodSplatter(FVector Location, FVector Direction)
{
    CurrentEffectData.bLooping = false;
    CurrentEffectData.Duration = 3.0f;
    PlayEffect(EVFX_EffectType::Blood_Impact, Location, 1.0f);
    
    if (PrimaryEffectComponent)
    {
        PrimaryEffectComponent->SetVectorParameter(TEXT("Direction"), Direction);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Blood splatter effect at %s"), *Location.ToString());
}

void AVFX_EffectManager::PlayWaterSplash(FVector Location, float SplashSize)
{
    float SplashIntensity = FMath::Clamp(SplashSize, 0.3f, 2.0f);
    CurrentEffectData.bLooping = false;
    CurrentEffectData.Duration = 2.5f;
    PlayEffect(EVFX_EffectType::Water_Splash, Location, SplashIntensity);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Water splash effect - Size: %f"), SplashSize);
}

void AVFX_EffectManager::StartWeatherEffect(EVFX_EffectType WeatherType)
{
    if (WeatherType == EVFX_EffectType::Rain_Weather || 
        WeatherType == EVFX_EffectType::Volcanic_Ash ||
        WeatherType == EVFX_EffectType::Wind_Particles)
    {
        CurrentEffectData.bLooping = true;
        CurrentEffectData.Duration = -1.0f;
        PlayEffect(WeatherType, GetActorLocation(), 1.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Weather effect started - Type: %d"), (int32)WeatherType);
    }
}

void AVFX_EffectManager::StopWeatherEffect()
{
    StopEffect();
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Weather effect stopped"));
}

void AVFX_EffectManager::InitializeEffectSystems()
{
    // Initialize effect systems map with placeholder references
    // In a real implementation, these would be loaded from content browser
    EffectSystems.Empty();
    
    // Log initialization for debugging
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect systems initialized"));
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Available effect types: Fire_Campfire, Dust_Footstep, Blood_Impact, Water_Splash, Rain_Weather, Wind_Particles, Volcanic_Ash"));
}

UNiagaraSystem* AVFX_EffectManager::GetEffectSystem(EVFX_EffectType EffectType)
{
    // Return placeholder system for now
    // In production, this would return the actual Niagara system for each effect type
    if (EffectSystems.Contains(EffectType))
    {
        return EffectSystems[EffectType];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect system not found for type %d"), (int32)EffectType);
    return nullptr;
}

void AVFX_EffectManager::UpdateEffectParameters()
{
    if (bEffectActive && PrimaryEffectComponent)
    {
        // Update dynamic parameters based on game state
        float TimeBasedIntensity = CurrentEffectData.Intensity;
        
        // Add slight variation for more natural look
        if (CurrentEffectData.EffectType == EVFX_EffectType::Fire_Campfire)
        {
            float FlickerVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.1f;
            TimeBasedIntensity += FlickerVariation;
        }
        
        PrimaryEffectComponent->SetFloatParameter(TEXT("DynamicIntensity"), TimeBasedIntensity);
    }
}