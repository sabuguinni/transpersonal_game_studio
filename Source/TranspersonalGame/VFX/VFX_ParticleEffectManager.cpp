#include "VFX_ParticleEffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"

UVFX_ParticleEffectManager::UVFX_ParticleEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for performance

    // Initialize default values
    MaxActiveEffects = 50;
    EffectCleanupInterval = 1.0f;
    DefaultEffectDuration = 5.0f;
    CleanupTimer = 0.0f;

    // Initialize effect arrays
    ActiveEffects.Reserve(MaxActiveEffects);
    EffectTimers.Reserve(MaxActiveEffects);
}

void UVFX_ParticleEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    LoadVFXAssets();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Particle Effect Manager initialized with %d max effects"), MaxActiveEffects);
}

void UVFX_ParticleEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update effect timers
    for (int32 i = EffectTimers.Num() - 1; i >= 0; i--)
    {
        EffectTimers[i] -= DeltaTime;
        if (EffectTimers[i] <= 0.0f)
        {
            RemoveExpiredEffect(i);
        }
    }

    // Periodic cleanup
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= EffectCleanupInterval)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

UNiagaraComponent* UVFX_ParticleEffectManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Intensity)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max VFX effects reached (%d), skipping new effect"), MaxActiveEffects);
        return nullptr;
    }

    UNiagaraSystem* EffectAsset = nullptr;
    float Duration = DefaultEffectDuration;

    switch (EffectType)
    {
        case EVFX_EffectType::FootstepDust:
            EffectAsset = FootstepDustEffect;
            Duration = 2.0f;
            break;
        case EVFX_EffectType::CampfireFire:
            EffectAsset = CampfireFireEffect;
            Duration = -1.0f; // Persistent effect
            break;
        case EVFX_EffectType::CampfireSmoke:
            EffectAsset = CampfireSmokeEffect;
            Duration = -1.0f; // Persistent effect
            break;
        case EVFX_EffectType::BloodSplatter:
            EffectAsset = BloodSplatterEffect;
            Duration = 3.0f;
            break;
        case EVFX_EffectType::RockDebris:
            EffectAsset = RockDebrisEffect;
            Duration = 4.0f;
            break;
        case EVFX_EffectType::WaterSplash:
            EffectAsset = WaterSplashEffect;
            Duration = 2.5f;
            break;
        case EVFX_EffectType::DinosaurBreath:
            EffectAsset = DinosaurBreathEffect;
            Duration = 1.5f;
            break;
        case EVFX_EffectType::WeatherRain:
            EffectAsset = RainEffect;
            Duration = -1.0f; // Persistent effect
            break;
        case EVFX_EffectType::WeatherFog:
            EffectAsset = FogEffect;
            Duration = -1.0f; // Persistent effect
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown VFX effect type requested"));
            return nullptr;
    }

    return CreateNiagaraEffect(EffectAsset, Location, Rotation, Duration);
}

void UVFX_ParticleEffectManager::SpawnFootstepDust(FVector FootLocation, float DinosaurSize)
{
    UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::FootstepDust, FootLocation, FRotator::ZeroRotator, DinosaurSize);
    if (Effect)
    {
        // Scale effect based on dinosaur size
        Effect->SetFloatParameter(TEXT("Size"), DinosaurSize);
        Effect->SetFloatParameter(TEXT("Intensity"), FMath::Clamp(DinosaurSize, 0.5f, 3.0f));
        UE_LOG(LogTemp, Log, TEXT("Spawned footstep dust at %s with size %f"), *FootLocation.ToString(), DinosaurSize);
    }
}

void UVFX_ParticleEffectManager::SpawnCampfire(FVector FireLocation, bool bIncludeSmoke)
{
    // Spawn fire effect
    UNiagaraComponent* FireEffect = SpawnVFXEffect(EVFX_EffectType::CampfireFire, FireLocation, FRotator::ZeroRotator, 1.0f);
    
    // Spawn smoke effect slightly above fire
    if (bIncludeSmoke)
    {
        FVector SmokeLocation = FireLocation + FVector(0, 0, 50);
        UNiagaraComponent* SmokeEffect = SpawnVFXEffect(EVFX_EffectType::CampfireSmoke, SmokeLocation, FRotator::ZeroRotator, 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned campfire at %s (smoke: %s)"), *FireLocation.ToString(), bIncludeSmoke ? TEXT("Yes") : TEXT("No"));
}

void UVFX_ParticleEffectManager::SpawnBloodEffect(FVector ImpactLocation, FVector ImpactDirection, float BloodAmount)
{
    FRotator BloodRotation = ImpactDirection.Rotation();
    UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::BloodSplatter, ImpactLocation, BloodRotation, BloodAmount);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("BloodAmount"), BloodAmount);
        Effect->SetVectorParameter(TEXT("Direction"), ImpactDirection);
        UE_LOG(LogTemp, Log, TEXT("Spawned blood effect at %s with amount %f"), *ImpactLocation.ToString(), BloodAmount);
    }
}

void UVFX_ParticleEffectManager::SpawnRockDebris(FVector ImpactLocation, float DebrisAmount)
{
    UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::RockDebris, ImpactLocation, FRotator::ZeroRotator, DebrisAmount);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("DebrisAmount"), DebrisAmount);
        Effect->SetFloatParameter(TEXT("Spread"), FMath::Clamp(DebrisAmount * 100.0f, 50.0f, 300.0f));
        UE_LOG(LogTemp, Log, TEXT("Spawned rock debris at %s with amount %f"), *ImpactLocation.ToString(), DebrisAmount);
    }
}

void UVFX_ParticleEffectManager::SpawnWaterSplash(FVector WaterLocation, float SplashSize)
{
    UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::WaterSplash, WaterLocation, FRotator::ZeroRotator, SplashSize);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("SplashSize"), SplashSize);
        Effect->SetFloatParameter(TEXT("Height"), SplashSize * 200.0f);
        UE_LOG(LogTemp, Log, TEXT("Spawned water splash at %s with size %f"), *WaterLocation.ToString(), SplashSize);
    }
}

void UVFX_ParticleEffectManager::SpawnDinosaurBreath(FVector MouthLocation, FRotator BreathDirection, float BreathIntensity)
{
    UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::DinosaurBreath, MouthLocation, BreathDirection, BreathIntensity);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Intensity"), BreathIntensity);
        Effect->SetVectorParameter(TEXT("Direction"), BreathDirection.Vector());
        UE_LOG(LogTemp, Log, TEXT("Spawned dinosaur breath at %s with intensity %f"), *MouthLocation.ToString(), BreathIntensity);
    }
}

void UVFX_ParticleEffectManager::StartRainEffect(float RainIntensity)
{
    if (GetOwner())
    {
        FVector PlayerLocation = GetOwner()->GetActorLocation();
        FVector RainLocation = PlayerLocation + FVector(0, 0, 1000); // High above player
        
        UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::WeatherRain, RainLocation, FRotator::ZeroRotator, RainIntensity);
        if (Effect)
        {
            Effect->SetFloatParameter(TEXT("RainIntensity"), RainIntensity);
            Effect->SetFloatParameter(TEXT("Coverage"), 5000.0f); // Large coverage area
            UE_LOG(LogTemp, Log, TEXT("Started rain effect with intensity %f"), RainIntensity);
        }
    }
}

void UVFX_ParticleEffectManager::StopRainEffect()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && ActiveEffects[i]->GetAsset() == RainEffect)
        {
            ActiveEffects[i]->DestroyComponent();
            RemoveExpiredEffect(i);
            UE_LOG(LogTemp, Log, TEXT("Stopped rain effect"));
            break;
        }
    }
}

void UVFX_ParticleEffectManager::StartFogEffect(float FogDensity)
{
    if (GetOwner())
    {
        FVector PlayerLocation = GetOwner()->GetActorLocation();
        UNiagaraComponent* Effect = SpawnVFXEffect(EVFX_EffectType::WeatherFog, PlayerLocation, FRotator::ZeroRotator, FogDensity);
        if (Effect)
        {
            Effect->SetFloatParameter(TEXT("Density"), FogDensity);
            Effect->SetFloatParameter(TEXT("Coverage"), 3000.0f);
            UE_LOG(LogTemp, Log, TEXT("Started fog effect with density %f"), FogDensity);
        }
    }
}

void UVFX_ParticleEffectManager::StopFogEffect()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && ActiveEffects[i]->GetAsset() == FogEffect)
        {
            ActiveEffects[i]->DestroyComponent();
            RemoveExpiredEffect(i);
            UE_LOG(LogTemp, Log, TEXT("Stopped fog effect"));
            break;
        }
    }
}

void UVFX_ParticleEffectManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEffects[i] || !IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            RemoveExpiredEffect(i);
        }
    }
}

void UVFX_ParticleEffectManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    EffectTimers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Stopped all VFX effects"));
}

int32 UVFX_ParticleEffectManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

UNiagaraComponent* UVFX_ParticleEffectManager::CreateNiagaraEffect(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, float Duration)
{
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create VFX effect - Niagara system is null"));
        return nullptr;
    }

    if (!GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create VFX effect - Owner is null"));
        return nullptr;
    }

    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NewEffect)
    {
        RegisterActiveEffect(NewEffect, Duration);
        UE_LOG(LogTemp, Log, TEXT("Created Niagara effect at %s with duration %f"), *Location.ToString(), Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create Niagara effect"));
    }

    return NewEffect;
}

void UVFX_ParticleEffectManager::LoadVFXAssets()
{
    // Note: In a real implementation, these would load actual Niagara system assets
    // For now, we'll set them to nullptr and handle the null case in CreateNiagaraEffect
    
    FootstepDustEffect = nullptr;
    CampfireFireEffect = nullptr;
    CampfireSmokeEffect = nullptr;
    BloodSplatterEffect = nullptr;
    RockDebrisEffect = nullptr;
    WaterSplashEffect = nullptr;
    DinosaurBreathEffect = nullptr;
    RainEffect = nullptr;
    FogEffect = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("VFX assets loading completed (placeholders for now)"));
}

void UVFX_ParticleEffectManager::RegisterActiveEffect(UNiagaraComponent* Effect, float Duration)
{
    if (!Effect)
    {
        return;
    }

    ActiveEffects.Add(Effect);
    EffectTimers.Add(Duration);

    // Ensure we don't exceed max effects
    while (ActiveEffects.Num() > MaxActiveEffects)
    {
        RemoveExpiredEffect(0); // Remove oldest effect
    }
}

void UVFX_ParticleEffectManager::RemoveExpiredEffect(int32 Index)
{
    if (Index >= 0 && Index < ActiveEffects.Num())
    {
        if (ActiveEffects[Index] && IsValid(ActiveEffects[Index]))
        {
            ActiveEffects[Index]->DestroyComponent();
        }
        
        ActiveEffects.RemoveAt(Index);
        
        if (Index < EffectTimers.Num())
        {
            EffectTimers.RemoveAt(Index);
        }
    }
}