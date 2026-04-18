#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    MaxActiveEffects = 50;
    EffectCullDistance = 5000.0f;
    bEnableAudioSync = true;
    GlobalVFXScale = 1.0f;
    CurrentActiveEffectCount = 0;
    LastCleanupTime = 0.0f;
    RainEffectComponent = nullptr;
}

void UVFX_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized"));
    
    // Initialize effect templates with default paths
    // These will be set up in Blueprint or via data assets
    EffectTemplates.Empty();
    EffectAudioTemplates.Empty();
    
    // Clear active effects array
    ActiveEffects.Empty();
    CurrentActiveEffectCount = 0;
}

void UVFX_SystemManager::Deinitialize()
{
    // Clean up all active effects
    StopAllEffects();
    
    // Clear arrays
    ActiveEffects.Empty();
    EffectTemplates.Empty();
    EffectAudioTemplates.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager deinitialized"));
    
    Super::Deinitialize();
}

UNiagaraComponent* UVFX_SystemManager::SpawnEffect(const FVFX_EffectData& EffectData, AActor* AttachToActor)
{
    // Check if we're at the effect limit
    if (CurrentActiveEffectCount >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (CurrentActiveEffectCount >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX: Maximum active effects reached, skipping spawn"));
            return nullptr;
        }
    }
    
    // Check distance culling
    if (ShouldCullEffect(EffectData.Location))
    {
        return nullptr;
    }
    
    // Load the Niagara system
    UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        // Try to get from templates
        if (EffectTemplates.Contains(EffectData.EffectType))
        {
            NiagaraSystem = EffectTemplates[EffectData.EffectType].LoadSynchronous();
        }
        
        if (!NiagaraSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX: No Niagara system found for effect type"));
            return nullptr;
        }
    }
    
    // Create the effect component
    UNiagaraComponent* EffectComponent = CreateEffectComponent(NiagaraSystem, EffectData.Location, EffectData.Rotation);
    if (!EffectComponent)
    {
        return nullptr;
    }
    
    // Apply scale
    FVector FinalScale = EffectData.Scale * GlobalVFXScale;
    EffectComponent->SetWorldScale3D(FinalScale);
    
    // Attach to actor if specified
    if (AttachToActor)
    {
        EffectComponent->AttachToComponent(AttachToActor->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
    }
    
    // Set auto-destroy if specified
    if (EffectData.bAutoDestroy && EffectData.Duration > 0.0f)
    {
        // Schedule destruction
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, 
            [this, EffectComponent]()
            {
                if (IsValid(EffectComponent))
                {
                    StopEffect(EffectComponent);
                }
            }, 
            EffectData.Duration, false);
    }
    
    // Play associated audio
    if (bEnableAudioSync)
    {
        PlayEffectAudio(EffectData.EffectType, EffectData.Location);
    }
    
    // Register the effect
    RegisterActiveEffect(EffectComponent);
    
    return EffectComponent;
}

void UVFX_SystemManager::SpawnFireEffect(const FVector& Location, EVFX_IntensityLevel Intensity)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Fire;
    EffectData.Location = Location;
    EffectData.Intensity = Intensity;
    EffectData.Duration = 0.0f; // Persistent fire
    EffectData.bAutoDestroy = false;
    
    // Scale based on intensity
    switch (Intensity)
    {
        case EVFX_IntensityLevel::Low:
            EffectData.Scale = FVector(0.5f);
            break;
        case EVFX_IntensityLevel::Medium:
            EffectData.Scale = FVector(1.0f);
            break;
        case EVFX_IntensityLevel::High:
            EffectData.Scale = FVector(1.5f);
            break;
        case EVFX_IntensityLevel::Extreme:
            EffectData.Scale = FVector(2.0f);
            break;
    }
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::SpawnDustImpact(const FVector& Location, const FVector& ImpactNormal, float ImpactForce)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Dust;
    EffectData.Location = Location;
    EffectData.Rotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    EffectData.Scale = FVector(FMath::Clamp(ImpactForce, 0.5f, 3.0f));
    EffectData.Duration = 3.0f;
    EffectData.bAutoDestroy = true;
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::SpawnFootstepEffect(const FVector& Location, float CreatureSize, bool bIsWet)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Footstep;
    EffectData.Location = Location;
    EffectData.Scale = FVector(CreatureSize);
    EffectData.Duration = 2.0f;
    EffectData.bAutoDestroy = true;
    
    // Different intensity for wet vs dry ground
    EffectData.Intensity = bIsWet ? EVFX_IntensityLevel::High : EVFX_IntensityLevel::Medium;
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::SpawnBloodSplatter(const FVector& Location, const FVector& Direction, EVFX_IntensityLevel Intensity)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Blood;
    EffectData.Location = Location;
    EffectData.Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    EffectData.Intensity = Intensity;
    EffectData.Duration = 5.0f;
    EffectData.bAutoDestroy = true;
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::SpawnWaterSplash(const FVector& Location, float SplashRadius)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Water;
    EffectData.Location = Location;
    EffectData.Scale = FVector(SplashRadius / 100.0f); // Normalize to 100cm base
    EffectData.Duration = 2.0f;
    EffectData.bAutoDestroy = true;
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::StartRainEffect(EVFX_IntensityLevel Intensity)
{
    if (RainEffectComponent && IsValid(RainEffectComponent))
    {
        StopRainEffect();
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Weather;
    EffectData.Intensity = Intensity;
    EffectData.bAutoDestroy = false;
    EffectData.Duration = 0.0f; // Persistent
    
    // Position rain above player
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            EffectData.Location = PlayerPawn->GetActorLocation() + FVector(0, 0, 1000);
        }
    }
    
    RainEffectComponent = SpawnEffect(EffectData);
}

void UVFX_SystemManager::StopRainEffect()
{
    if (RainEffectComponent && IsValid(RainEffectComponent))
    {
        StopEffect(RainEffectComponent);
        RainEffectComponent = nullptr;
    }
}

void UVFX_SystemManager::SpawnBreathVapor(const FVector& Location, const FRotator& Direction)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::Breath;
    EffectData.Location = Location;
    EffectData.Rotation = Direction;
    EffectData.Duration = 3.0f;
    EffectData.bAutoDestroy = true;
    EffectData.Scale = FVector(0.8f);
    
    SpawnEffect(EffectData);
}

void UVFX_SystemManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Only cleanup if enough time has passed
    if (CurrentTime - LastCleanupTime < CleanupInterval)
    {
        return;
    }
    
    LastCleanupTime = CurrentTime;
    
    // Remove invalid or completed effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
            CurrentActiveEffectCount--;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Cleanup completed, active effects: %d"), CurrentActiveEffectCount);
}

void UVFX_SystemManager::SetVFXQualityLevel(int32 QualityLevel)
{
    // Adjust max effects and cull distance based on quality
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            EffectCullDistance = 2000.0f;
            GlobalVFXScale = 0.7f;
            break;
        case 1: // Medium
            MaxActiveEffects = 35;
            EffectCullDistance = 3500.0f;
            GlobalVFXScale = 0.85f;
            break;
        case 2: // High
            MaxActiveEffects = 50;
            EffectCullDistance = 5000.0f;
            GlobalVFXScale = 1.0f;
            break;
        case 3: // Ultra
            MaxActiveEffects = 75;
            EffectCullDistance = 7500.0f;
            GlobalVFXScale = 1.2f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality set to level %d"), QualityLevel);
}

void UVFX_SystemManager::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (!IsValid(EffectComponent))
    {
        return;
    }
    
    EffectComponent->Deactivate();
    EffectComponent->DestroyComponent();
    
    UnregisterActiveEffect(EffectComponent);
}

void UVFX_SystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    CurrentActiveEffectCount = 0;
    RainEffectComponent = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("VFX: All effects stopped"));
}

UNiagaraComponent* UVFX_SystemManager::CreateEffectComponent(UNiagaraSystem* NiagaraSystem, const FVector& Location, const FRotator& Rotation)
{
    if (!NiagaraSystem)
    {
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn the Niagara component at world location
    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, NiagaraSystem, Location, Rotation);
    
    return Component;
}

void UVFX_SystemManager::PlayEffectAudio(EVFX_EffectType EffectType, const FVector& Location, float VolumeMultiplier)
{
    if (!bEnableAudioSync)
    {
        return;
    }
    
    if (!EffectAudioTemplates.Contains(EffectType))
    {
        return;
    }
    
    USoundCue* SoundCue = EffectAudioTemplates[EffectType].LoadSynchronous();
    if (!SoundCue)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (World)
    {
        UGameplayStatics::PlaySoundAtLocation(World, SoundCue, Location, VolumeMultiplier);
    }
}

void UVFX_SystemManager::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        ActiveEffects.Add(Effect);
        CurrentActiveEffectCount++;
    }
}

void UVFX_SystemManager::UnregisterActiveEffect(UNiagaraComponent* Effect)
{
    if (ActiveEffects.Remove(Effect) > 0)
    {
        CurrentActiveEffectCount--;
    }
}

bool UVFX_SystemManager::ShouldCullEffect(const FVector& EffectLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Get player location for distance check
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
        return Distance > EffectCullDistance;
    }
    
    return false;
}