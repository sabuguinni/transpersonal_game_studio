#include "VFX_EnvironmentalEffectManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SceneComponent.h"

UVFX_EnvironmentalEffectManager::UVFX_EnvironmentalEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second for performance
    
    MaxActiveEffects = 20;
    EffectCullingDistance = 5000.0f;
    bUseNiagaraWhenAvailable = true;
    LastWeatherUpdate = 0.0f;
    LastTimeUpdate = 0.0f;
}

void UVFX_EnvironmentalEffectManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectDatabase();
}

void UVFX_EnvironmentalEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up effects that are too far from player or finished
    CleanupInactiveEffects();
    
    // Update weather-dependent effects every 2 seconds
    if (GetWorld()->GetTimeSeconds() - LastWeatherUpdate > 2.0f)
    {
        LastWeatherUpdate = GetWorld()->GetTimeSeconds();
        // Weather update logic would go here when weather system is implemented
    }
}

void UVFX_EnvironmentalEffectManager::SpawnEnvironmentalEffect(EVFX_EnvironmentalType EffectType, FVector Location, float Intensity)
{
    // Check if we're at max capacity
    if (ActiveNiagaraComponents.Num() + ActiveParticleComponents.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffectManager: Max effects reached, skipping spawn"));
        return;
    }

    // Find effect configuration
    FVFX_EnvironmentalEffect* FoundEffect = nullptr;
    for (FVFX_EnvironmentalEffect& Effect : EnvironmentalEffects)
    {
        if (Effect.EffectType == EffectType)
        {
            FoundEffect = &Effect;
            break;
        }
    }

    if (!FoundEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EnvironmentalEffectManager: Effect type not configured: %d"), (int32)EffectType);
        return;
    }

    // Try Niagara first if available and preferred
    if (bUseNiagaraWhenAvailable && FoundEffect->NiagaraSystem.IsValid())
    {
        UNiagaraComponent* NiagaraComp = CreateNiagaraEffect(*FoundEffect, Location);
        if (NiagaraComp)
        {
            ActiveNiagaraComponents.Add(NiagaraComp);
            UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffectManager: Spawned Niagara effect at %s"), *Location.ToString());
            return;
        }
    }

    // Fall back to legacy particle system
    if (FoundEffect->LegacyParticleSystem.IsValid())
    {
        UParticleSystemComponent* ParticleComp = CreateParticleEffect(*FoundEffect, Location);
        if (ParticleComp)
        {
            ActiveParticleComponents.Add(ParticleComp);
            UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffectManager: Spawned particle effect at %s"), *Location.ToString());
        }
    }
}

void UVFX_EnvironmentalEffectManager::StopEnvironmentalEffect(EVFX_EnvironmentalType EffectType)
{
    // Stop all effects of this type
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveNiagaraComponents[i] && IsValid(ActiveNiagaraComponents[i]))
        {
            ActiveNiagaraComponents[i]->Deactivate();
            ActiveNiagaraComponents.RemoveAt(i);
        }
    }

    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveParticleComponents[i] && IsValid(ActiveParticleComponents[i]))
        {
            ActiveParticleComponents[i]->Deactivate();
            ActiveParticleComponents.RemoveAt(i);
        }
    }
}

void UVFX_EnvironmentalEffectManager::UpdateWeatherEffects(EWeatherType WeatherType, float Intensity)
{
    // Adjust environmental effects based on weather
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            // Reduce mist, increase dust devils
            break;
        case EWeatherType::Rainy:
            // Increase rain droplets, reduce dust
            SpawnEnvironmentalEffect(EVFX_EnvironmentalType::RainDroplets, GetOwner()->GetActorLocation(), Intensity);
            break;
        case EWeatherType::Stormy:
            // Increase wind particles, reduce visibility effects
            SpawnEnvironmentalEffect(EVFX_EnvironmentalType::WindParticles, GetOwner()->GetActorLocation(), Intensity * 1.5f);
            break;
        case EWeatherType::Foggy:
            // Increase mist effects
            SpawnEnvironmentalEffect(EVFX_EnvironmentalType::MorningMist, GetOwner()->GetActorLocation(), Intensity);
            break;
    }
}

void UVFX_EnvironmentalEffectManager::UpdateTimeOfDayEffects(float TimeOfDay)
{
    // 0.0 = midnight, 0.5 = noon, 1.0 = midnight again
    if (TimeOfDay >= 0.2f && TimeOfDay <= 0.4f) // Morning
    {
        SpawnEnvironmentalEffect(EVFX_EnvironmentalType::MorningMist, GetOwner()->GetActorLocation(), 0.8f);
    }
    else if (TimeOfDay >= 0.6f && TimeOfDay <= 0.8f) // Evening
    {
        SpawnEnvironmentalEffect(EVFX_EnvironmentalType::InsectSwarm, GetOwner()->GetActorLocation(), 0.6f);
    }
}

void UVFX_EnvironmentalEffectManager::SetEffectQuality(int32 QualityLevel)
{
    // Adjust max effects and culling distance based on quality
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 10;
            EffectCullingDistance = 2000.0f;
            break;
        case 1: // Medium
            MaxActiveEffects = 20;
            EffectCullingDistance = 5000.0f;
            break;
        case 2: // High
            MaxActiveEffects = 40;
            EffectCullingDistance = 8000.0f;
            break;
        default:
            MaxActiveEffects = 20;
            EffectCullingDistance = 5000.0f;
            break;
    }
}

void UVFX_EnvironmentalEffectManager::InitializeEffectDatabase()
{
    EnvironmentalEffects.Empty();

    // Campfire smoke effect
    FVFX_EnvironmentalEffect CampfireEffect;
    CampfireEffect.EffectType = EVFX_EnvironmentalType::CampfireSmoke;
    CampfireEffect.EffectRadius = 500.0f;
    CampfireEffect.EffectIntensity = 1.0f;
    CampfireEffect.bIsWeatherDependent = true;
    CampfireEffect.bIsTimeOfDayDependent = false;
    EnvironmentalEffects.Add(CampfireEffect);

    // Morning mist effect
    FVFX_EnvironmentalEffect MistEffect;
    MistEffect.EffectType = EVFX_EnvironmentalType::MorningMist;
    MistEffect.EffectRadius = 2000.0f;
    MistEffect.EffectIntensity = 0.7f;
    MistEffect.bIsWeatherDependent = true;
    MistEffect.bIsTimeOfDayDependent = true;
    EnvironmentalEffects.Add(MistEffect);

    // Volcanic ash effect
    FVFX_EnvironmentalEffect AshEffect;
    AshEffect.EffectType = EVFX_EnvironmentalType::VolcanicAsh;
    AshEffect.EffectRadius = 5000.0f;
    AshEffect.EffectIntensity = 0.5f;
    AshEffect.bIsWeatherDependent = false;
    AshEffect.bIsTimeOfDayDependent = false;
    EnvironmentalEffects.Add(AshEffect);

    // Dust devil effect
    FVFX_EnvironmentalEffect DustEffect;
    DustEffect.EffectType = EVFX_EnvironmentalType::DustDevil;
    DustEffect.EffectRadius = 1000.0f;
    DustEffect.EffectIntensity = 1.2f;
    DustEffect.bIsWeatherDependent = true;
    DustEffect.bIsTimeOfDayDependent = false;
    EnvironmentalEffects.Add(DustEffect);

    UE_LOG(LogTemp, Log, TEXT("VFX_EnvironmentalEffectManager: Initialized %d environmental effects"), EnvironmentalEffects.Num());
}

void UVFX_EnvironmentalEffectManager::CleanupInactiveEffects()
{
    float PlayerDistance = GetDistanceToPlayer();

    // Clean up Niagara components
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Comp = ActiveNiagaraComponents[i];
        if (!Comp || !IsValid(Comp) || !Comp->IsActive() || PlayerDistance > EffectCullingDistance)
        {
            if (Comp && IsValid(Comp))
            {
                Comp->Deactivate();
            }
            ActiveNiagaraComponents.RemoveAt(i);
        }
    }

    // Clean up particle components
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* Comp = ActiveParticleComponents[i];
        if (!Comp || !IsValid(Comp) || !Comp->IsActive() || PlayerDistance > EffectCullingDistance)
        {
            if (Comp && IsValid(Comp))
            {
                Comp->Deactivate();
            }
            ActiveParticleComponents.RemoveAt(i);
        }
    }
}

UNiagaraComponent* UVFX_EnvironmentalEffectManager::CreateNiagaraEffect(const FVFX_EnvironmentalEffect& Effect, FVector Location)
{
    if (!Effect.NiagaraSystem.IsValid())
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        Effect.NiagaraSystem.Get(),
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NiagaraComp)
    {
        NiagaraComp->SetFloatParameter(TEXT("Intensity"), Effect.EffectIntensity);
        NiagaraComp->SetFloatParameter(TEXT("Radius"), Effect.EffectRadius);
    }

    return NiagaraComp;
}

UParticleSystemComponent* UVFX_EnvironmentalEffectManager::CreateParticleEffect(const FVFX_EnvironmentalEffect& Effect, FVector Location)
{
    if (!Effect.LegacyParticleSystem.IsValid())
    {
        return nullptr;
    }

    UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        Effect.LegacyParticleSystem.Get(),
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        EPSCPoolMethod::None,
        true
    );

    return ParticleComp;
}

bool UVFX_EnvironmentalEffectManager::ShouldEffectBeActive(const FVFX_EnvironmentalEffect& Effect) const
{
    // Check distance to player
    if (GetDistanceToPlayer() > EffectCullingDistance)
    {
        return false;
    }

    // Weather and time-of-day checks would go here when those systems are implemented
    return true;
}

float UVFX_EnvironmentalEffectManager::GetDistanceToPlayer() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && GetOwner())
    {
        return FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
    }
    return 0.0f;
}