#include "VFX_EnvironmentalEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"

UVFX_EnvironmentalEffectsManager::UVFX_EnvironmentalEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    GlobalVFXIntensity = 1.0f;
    MaxActiveEffects = 50;
    bEnableEnvironmentalVFX = true;
}

void UVFX_EnvironmentalEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXSystems();
    
    // Set up cleanup timer to run every 5 seconds
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_EnvironmentalEffectsManager::CleanupExpiredEffects,
            5.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Environmental Effects Manager initialized"));
}

void UVFX_EnvironmentalEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor active effects and update intensity
    if (ActiveEffects.Num() > MaxActiveEffects)
    {
        // Remove oldest effects if we exceed the limit
        int32 EffectsToRemove = ActiveEffects.Num() - MaxActiveEffects;
        for (int32 i = 0; i < EffectsToRemove; i++)
        {
            if (ActiveEffects.IsValidIndex(0) && ActiveEffects[0])
            {
                ActiveEffects[0]->DestroyComponent();
                ActiveEffects.RemoveAt(0);
            }
        }
    }
}

void UVFX_EnvironmentalEffectsManager::InitializeVFXSystems()
{
    // Initialize default Niagara system references
    // These would be set up in Blueprint or loaded from content
    UE_LOG(LogTemp, Log, TEXT("Initializing VFX systems for environmental effects"));
    
    // Clear any existing active effects
    ActiveEffects.Empty();
}

void UVFX_EnvironmentalEffectsManager::SpawnEnvironmentalEffect(const FVFX_EnvironmentalEffect& EffectData)
{
    if (!bEnableEnvironmentalVFX)
    {
        return;
    }
    
    UNiagaraSystem* SystemToUse = nullptr;
    
    // Select appropriate Niagara system based on effect type
    switch (EffectData.EffectType)
    {
        case EVFX_EnvironmentalType::DustCloud:
            SystemToUse = DustCloudSystem.LoadSynchronous();
            break;
        case EVFX_EnvironmentalType::WaterSplash:
            SystemToUse = WaterSplashSystem.LoadSynchronous();
            break;
        case EVFX_EnvironmentalType::RainDroplets:
            SystemToUse = RainSystem.LoadSynchronous();
            break;
        case EVFX_EnvironmentalType::VolcanicAsh:
            SystemToUse = VolcanicAshSystem.LoadSynchronous();
            break;
        default:
            SystemToUse = DustCloudSystem.LoadSynchronous();
            break;
    }
    
    if (SystemToUse)
    {
        UNiagaraComponent* NewEffect = CreateNiagaraEffect(
            SystemToUse,
            EffectData.SpawnLocation,
            EffectData.SpawnRotation
        );
        
        if (NewEffect)
        {
            // Set effect parameters
            NewEffect->SetFloatParameter(TEXT("Intensity"), EffectData.Intensity * GlobalVFXIntensity);
            NewEffect->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
            
            ActiveEffects.Add(NewEffect);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned environmental effect at location: %s"), 
                *EffectData.SpawnLocation.ToString());
        }
    }
}

void UVFX_EnvironmentalEffectsManager::SpawnDustCloud(FVector Location, float Intensity)
{
    FVFX_EnvironmentalEffect EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::DustCloud;
    EffectData.SpawnLocation = Location;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 2.0f;
    
    SpawnEnvironmentalEffect(EffectData);
}

void UVFX_EnvironmentalEffectsManager::SpawnWaterSplash(FVector Location, float Intensity)
{
    FVFX_EnvironmentalEffect EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::WaterSplash;
    EffectData.SpawnLocation = Location;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 1.5f;
    
    SpawnEnvironmentalEffect(EffectData);
}

void UVFX_EnvironmentalEffectsManager::SpawnRainEffect(FVector Location, float Duration)
{
    FVFX_EnvironmentalEffect EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::RainDroplets;
    EffectData.SpawnLocation = Location;
    EffectData.Duration = Duration;
    EffectData.Intensity = 1.0f;
    
    SpawnEnvironmentalEffect(EffectData);
}

void UVFX_EnvironmentalEffectsManager::SpawnVolcanicAsh(FVector Location, float Duration)
{
    FVFX_EnvironmentalEffect EffectData;
    EffectData.EffectType = EVFX_EnvironmentalType::VolcanicAsh;
    EffectData.SpawnLocation = Location;
    EffectData.Duration = Duration;
    EffectData.Intensity = 0.8f;
    
    SpawnEnvironmentalEffect(EffectData);
}

void UVFX_EnvironmentalEffectsManager::UpdateWeatherEffects(EWeatherType WeatherType, float Intensity)
{
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            StopRainSystem();
            break;
        case EWeatherType::Rainy:
            StartRainSystem();
            SetGlobalVFXIntensity(Intensity);
            break;
        case EWeatherType::Stormy:
            StartRainSystem();
            SetGlobalVFXIntensity(FMath::Clamp(Intensity * 1.5f, 0.0f, 2.0f));
            break;
        case EWeatherType::Foggy:
            // Spawn fog mist effects
            if (AActor* Owner = GetOwner())
            {
                SpawnEnvironmentalEffect({
                    EVFX_EnvironmentalType::FogMist,
                    nullptr,
                    Owner->GetActorLocation(),
                    FRotator::ZeroRotator,
                    30.0f,
                    Intensity,
                    false
                });
            }
            break;
    }
}

void UVFX_EnvironmentalEffectsManager::StartRainSystem()
{
    if (AActor* Owner = GetOwner())
    {
        FVector RainLocation = Owner->GetActorLocation() + FVector(0, 0, 1000);
        SpawnRainEffect(RainLocation, 60.0f); // Long duration rain
    }
}

void UVFX_EnvironmentalEffectsManager::StopRainSystem()
{
    // Find and stop rain effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects.IsValidIndex(i) && ActiveEffects[i])
        {
            // Check if this is a rain effect (simplified check)
            ActiveEffects[i]->SetFloatParameter(TEXT("Duration"), 0.5f); // Quick fade out
        }
    }
}

void UVFX_EnvironmentalEffectsManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEffects.IsValidIndex(i) || !ActiveEffects[i] || !ActiveEffects[i]->IsActive())
        {
            if (ActiveEffects.IsValidIndex(i) && ActiveEffects[i])
            {
                ActiveEffects[i]->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("VFX cleanup complete. Active effects: %d"), ActiveEffects.Num());
}

void UVFX_EnvironmentalEffectsManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalVFXIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    
    // Update all active effects with new intensity
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && Effect->IsActive())
        {
            float CurrentIntensity = 1.0f;
            Effect->GetFloatParameter(TEXT("Intensity"), CurrentIntensity);
            Effect->SetFloatParameter(TEXT("Intensity"), CurrentIntensity * GlobalVFXIntensity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to: %f"), GlobalVFXIntensity);
}

UNiagaraComponent* UVFX_EnvironmentalEffectsManager::CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
    if (!System || !GetOwner())
    {
        return nullptr;
    }
    
    UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    return NewComponent;
}

void UVFX_EnvironmentalEffectsManager::RemoveExpiredEffect(UNiagaraComponent* Effect)
{
    if (Effect)
    {
        ActiveEffects.Remove(Effect);
        Effect->DestroyComponent();
    }
}