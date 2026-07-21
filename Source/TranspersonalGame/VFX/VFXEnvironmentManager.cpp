#include "VFXEnvironmentManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UVFX_EnvironmentManager::UVFX_EnvironmentManager()
{
    MaxEffectDistance = 10000.0f;
    MaxActiveEffects = 10;
    CurrentWeatherType = EVFX_EnvironmentType::None;
    CurrentIntensity = EVFX_Intensity::Medium;
}

void UVFX_EnvironmentManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Environment Manager initialized"));
    
    // Initialize default environment configurations
    FVFX_EnvironmentConfig RainConfig;
    RainConfig.EnvironmentType = EVFX_EnvironmentType::Rain;
    RainConfig.Duration = 120.0f;
    RainConfig.FadeInTime = 8.0f;
    RainConfig.FadeOutTime = 8.0f;
    EnvironmentConfigs.Add(EVFX_EnvironmentType::Rain, RainConfig);
    
    FVFX_EnvironmentConfig SnowConfig;
    SnowConfig.EnvironmentType = EVFX_EnvironmentType::Snow;
    SnowConfig.Duration = 180.0f;
    SnowConfig.FadeInTime = 10.0f;
    SnowConfig.FadeOutTime = 10.0f;
    EnvironmentConfigs.Add(EVFX_EnvironmentType::Snow, SnowConfig);
    
    FVFX_EnvironmentConfig FogConfig;
    FogConfig.EnvironmentType = EVFX_EnvironmentType::Fog;
    FogConfig.Duration = 300.0f;
    FogConfig.FadeInTime = 15.0f;
    FogConfig.FadeOutTime = 15.0f;
    EnvironmentConfigs.Add(EVFX_EnvironmentType::Fog, FogConfig);
    
    FVFX_EnvironmentConfig DustConfig;
    DustConfig.EnvironmentType = EVFX_EnvironmentType::Dust;
    DustConfig.Duration = 60.0f;
    DustConfig.FadeInTime = 3.0f;
    DustConfig.FadeOutTime = 5.0f;
    EnvironmentConfigs.Add(EVFX_EnvironmentType::Dust, DustConfig);
    
    FVFX_EnvironmentConfig VolcanicConfig;
    VolcanicConfig.EnvironmentType = EVFX_EnvironmentType::Volcanic;
    VolcanicConfig.Duration = 240.0f;
    VolcanicConfig.FadeInTime = 20.0f;
    VolcanicConfig.FadeOutTime = 30.0f;
    EnvironmentConfigs.Add(EVFX_EnvironmentType::Volcanic, VolcanicConfig);
    
    // Setup cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(EffectCleanupTimer, this, &UVFX_EnvironmentManager::CleanupTemporaryEffects, 10.0f, true);
    }
}

void UVFX_EnvironmentManager::Deinitialize()
{
    StopAllEnvironmentEffects();
    CleanupTemporaryEffects();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EffectCleanupTimer);
        World->GetTimerManager().ClearTimer(WeatherTransitionTimer);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Environment Manager deinitialized"));
    Super::Deinitialize();
}

void UVFX_EnvironmentManager::StartEnvironmentEffect(EVFX_EnvironmentType EffectType, EVFX_Intensity Intensity, float Duration)
{
    if (EffectType == EVFX_EnvironmentType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start None environment effect"));
        return;
    }
    
    // Stop existing effect of this type
    if (IsEnvironmentEffectActive(EffectType))
    {
        StopEnvironmentEffect(EffectType);
    }
    
    // Get configuration
    FVFX_EnvironmentConfig* Config = EnvironmentConfigs.Find(EffectType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("No configuration found for environment effect type: %d"), (int32)EffectType);
        return;
    }
    
    // Update config with parameters
    Config->Intensity = Intensity;
    Config->Duration = Duration;
    
    // Create the effect
    CreateEnvironmentEffect(EffectType, *Config);
    
    CurrentWeatherType = EffectType;
    CurrentIntensity = Intensity;
    
    UE_LOG(LogTemp, Warning, TEXT("Started environment effect: %d with intensity: %d for duration: %.1f"), 
           (int32)EffectType, (int32)Intensity, Duration);
}

void UVFX_EnvironmentManager::StopEnvironmentEffect(EVFX_EnvironmentType EffectType)
{
    if (UNiagaraComponent** FoundEffect = ActiveEnvironmentEffects.Find(EffectType))
    {
        if (UNiagaraComponent* Effect = *FoundEffect)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
        ActiveEnvironmentEffects.Remove(EffectType);
        
        if (CurrentWeatherType == EffectType)
        {
            CurrentWeatherType = EVFX_EnvironmentType::None;
            CurrentIntensity = EVFX_Intensity::Medium;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Stopped environment effect: %d"), (int32)EffectType);
    }
}

void UVFX_EnvironmentManager::StopAllEnvironmentEffects()
{
    for (auto& EffectPair : ActiveEnvironmentEffects)
    {
        if (UNiagaraComponent* Effect = EffectPair.Value)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    ActiveEnvironmentEffects.Empty();
    
    CurrentWeatherType = EVFX_EnvironmentType::None;
    CurrentIntensity = EVFX_Intensity::Medium;
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped all environment effects"));
}

void UVFX_EnvironmentManager::StartBiomeWeather(EBiomeType BiomeType, EVFX_EnvironmentType WeatherType, EVFX_Intensity Intensity)
{
    // Biome-specific weather logic
    FVector BiomeLocation = FVector::ZeroVector;
    
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BiomeLocation = FVector(0.0f, 0.0f, 0.0f);
            break;
        case EBiomeType::Pantano:
            BiomeLocation = FVector(-50000.0f, -45000.0f, 0.0f);
            break;
        case EBiomeType::Floresta:
            BiomeLocation = FVector(-45000.0f, 40000.0f, 0.0f);
            break;
        case EBiomeType::Deserto:
            BiomeLocation = FVector(55000.0f, 0.0f, 0.0f);
            break;
        case EBiomeType::Montanha:
            BiomeLocation = FVector(40000.0f, 50000.0f, 0.0f);
            break;
    }
    
    // Update spawn location in config
    if (FVFX_EnvironmentConfig* Config = EnvironmentConfigs.Find(WeatherType))
    {
        Config->SpawnLocation = BiomeLocation;
    }
    
    StartEnvironmentEffect(WeatherType, Intensity);
    
    UE_LOG(LogTemp, Warning, TEXT("Started biome weather - Biome: %d, Weather: %d at location: %s"), 
           (int32)BiomeType, (int32)WeatherType, *BiomeLocation.ToString());
}

void UVFX_EnvironmentManager::TransitionWeather(EVFX_EnvironmentType FromType, EVFX_EnvironmentType ToType, float TransitionTime)
{
    if (FromType != EVFX_EnvironmentType::None && IsEnvironmentEffectActive(FromType))
    {
        // Fade out current weather
        if (UWorld* World = GetWorld())
        {
            FTimerDelegate FadeOutDelegate;
            FadeOutDelegate.BindUFunction(this, FName("StopEnvironmentEffect"), FromType);
            World->GetTimerManager().SetTimer(WeatherTransitionTimer, FadeOutDelegate, TransitionTime * 0.5f, false);
        }
    }
    
    if (ToType != EVFX_EnvironmentType::None)
    {
        // Fade in new weather
        if (UWorld* World = GetWorld())
        {
            FTimerDelegate FadeInDelegate;
            FadeInDelegate.BindUFunction(this, FName("StartEnvironmentEffect"), ToType, CurrentIntensity, 120.0f);
            World->GetTimerManager().SetTimer(WeatherTransitionTimer, FadeInDelegate, TransitionTime * 0.5f, false);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Weather transition started - From: %d to: %d over %.1f seconds"), 
           (int32)FromType, (int32)ToType, TransitionTime);
}

void UVFX_EnvironmentManager::SpawnDustCloud(FVector Location, float Radius, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create temporary dust cloud effect
    UNiagaraComponent* DustEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, 
        nullptr, // Will be set to dust system when available
        Location,
        FRotator::ZeroRotator,
        FVector(Radius / 500.0f) // Scale based on radius
    );
    
    if (DustEffect)
    {
        TemporaryEffects.Add(DustEffect);
        
        // Auto-destroy after duration
        FTimerHandle DestroyTimer;
        FTimerDelegate DestroyDelegate;
        DestroyDelegate.BindLambda([this, DustEffect]()
        {
            if (DustEffect && IsValid(DustEffect))
            {
                TemporaryEffects.Remove(DustEffect);
                DustEffect->Deactivate();
                DustEffect->DestroyComponent();
            }
        });
        World->GetTimerManager().SetTimer(DestroyTimer, DestroyDelegate, Duration, false);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned dust cloud at location: %s, radius: %.1f, duration: %.1f"), 
               *Location.ToString(), Radius, Duration);
    }
}

void UVFX_EnvironmentManager::SpawnVolcanicAsh(FVector Location, float Radius, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create volcanic ash effect
    UNiagaraComponent* AshEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        nullptr, // Will be set to volcanic ash system when available
        Location,
        FRotator::ZeroRotator,
        FVector(Radius / 1000.0f)
    );
    
    if (AshEffect)
    {
        TemporaryEffects.Add(AshEffect);
        
        FTimerHandle DestroyTimer;
        FTimerDelegate DestroyDelegate;
        DestroyDelegate.BindLambda([this, AshEffect]()
        {
            if (AshEffect && IsValid(AshEffect))
            {
                TemporaryEffects.Remove(AshEffect);
                AshEffect->Deactivate();
                AshEffect->DestroyComponent();
            }
        });
        World->GetTimerManager().SetTimer(DestroyTimer, DestroyDelegate, Duration, false);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned volcanic ash at location: %s, radius: %.1f, duration: %.1f"), 
               *Location.ToString(), Radius, Duration);
    }
}

void UVFX_EnvironmentManager::SpawnBreathSteam(FVector Location, FVector Direction, float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create breath steam effect
    FRotator SteamRotation = Direction.Rotation();
    UNiagaraComponent* SteamEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        nullptr, // Will be set to steam system when available
        Location,
        SteamRotation,
        FVector::OneVector
    );
    
    if (SteamEffect)
    {
        TemporaryEffects.Add(SteamEffect);
        
        FTimerHandle DestroyTimer;
        FTimerDelegate DestroyDelegate;
        DestroyDelegate.BindLambda([this, SteamEffect]()
        {
            if (SteamEffect && IsValid(SteamEffect))
            {
                TemporaryEffects.Remove(SteamEffect);
                SteamEffect->Deactivate();
                SteamEffect->DestroyComponent();
            }
        });
        World->GetTimerManager().SetTimer(DestroyTimer, DestroyDelegate, Duration, false);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned breath steam at location: %s, direction: %s, duration: %.1f"), 
               *Location.ToString(), *Direction.ToString(), Duration);
    }
}

bool UVFX_EnvironmentManager::IsEnvironmentEffectActive(EVFX_EnvironmentType EffectType) const
{
    if (const UNiagaraComponent* const* FoundEffect = ActiveEnvironmentEffects.Find(EffectType))
    {
        return *FoundEffect != nullptr && IsValid(*FoundEffect) && (*FoundEffect)->IsActive();
    }
    return false;
}

EVFX_Intensity UVFX_EnvironmentManager::GetCurrentWeatherIntensity() const
{
    return CurrentIntensity;
}

TArray<EVFX_EnvironmentType> UVFX_EnvironmentManager::GetActiveEffects() const
{
    TArray<EVFX_EnvironmentType> ActiveTypes;
    for (const auto& EffectPair : ActiveEnvironmentEffects)
    {
        if (EffectPair.Value && IsValid(EffectPair.Value) && EffectPair.Value->IsActive())
        {
            ActiveTypes.Add(EffectPair.Key);
        }
    }
    return ActiveTypes;
}

void UVFX_EnvironmentManager::CreateEnvironmentEffect(EVFX_EnvironmentType EffectType, const FVFX_EnvironmentConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEnvironmentType(EffectType);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Niagara system found for environment type: %d"), (int32)EffectType);
        return;
    }
    
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Config.SpawnLocation,
        FRotator::ZeroRotator,
        Config.SpawnScale
    );
    
    if (NewEffect)
    {
        ActiveEnvironmentEffects.Add(EffectType, NewEffect);
        
        // Set intensity parameters
        float IntensityValue = 0.5f;
        switch (Config.Intensity)
        {
            case EVFX_Intensity::Light: IntensityValue = 0.25f; break;
            case EVFX_Intensity::Medium: IntensityValue = 0.5f; break;
            case EVFX_Intensity::Heavy: IntensityValue = 0.75f; break;
            case EVFX_Intensity::Extreme: IntensityValue = 1.0f; break;
        }
        
        NewEffect->SetFloatParameter(FName("Intensity"), IntensityValue);
        
        UE_LOG(LogTemp, Warning, TEXT("Created environment effect: %d at location: %s"), 
               (int32)EffectType, *Config.SpawnLocation.ToString());
    }
}

void UVFX_EnvironmentManager::DestroyEnvironmentEffect(EVFX_EnvironmentType EffectType)
{
    if (UNiagaraComponent** FoundEffect = ActiveEnvironmentEffects.Find(EffectType))
    {
        if (UNiagaraComponent* Effect = *FoundEffect)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
        ActiveEnvironmentEffects.Remove(EffectType);
    }
}

void UVFX_EnvironmentManager::CleanupTemporaryEffects()
{
    for (int32 i = TemporaryEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = TemporaryEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            TemporaryEffects.RemoveAt(i);
        }
    }
}

UNiagaraSystem* UVFX_EnvironmentManager::GetNiagaraSystemForEnvironmentType(EVFX_EnvironmentType EffectType) const
{
    // Placeholder - will be replaced with actual Niagara system references
    // These would typically be loaded from content browser paths
    switch (EffectType)
    {
        case EVFX_EnvironmentType::Rain:
            // return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_Rain"));
            break;
        case EVFX_EnvironmentType::Snow:
            // return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_Snow"));
            break;
        case EVFX_EnvironmentType::Fog:
            // return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_Fog"));
            break;
        case EVFX_EnvironmentType::Dust:
            // return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_Dust"));
            break;
        case EVFX_EnvironmentType::Volcanic:
            // return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Environment/NS_Volcanic"));
            break;
    }
    
    return nullptr;
}