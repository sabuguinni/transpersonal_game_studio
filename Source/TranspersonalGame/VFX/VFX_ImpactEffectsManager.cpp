#include "VFX_ImpactEffectsManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DecalActor.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AVFX_ImpactEffectsManager::AVFX_ImpactEffectsManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    MaxSimultaneousEffects = 20.0f;
    EffectCullDistance = 5000.0f;
    bEnableImpactDecals = true;
    DecalLifetime = 30.0f;
    GlobalEffectIntensity = 1.0f;
}

void AVFX_ImpactEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEffects();
    LoadEffectAssets();
    
    // Set up cleanup timer
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &AVFX_ImpactEffectsManager::CleanupExpiredEffects,
        2.0f,
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Effects Manager initialized"));
}

void AVFX_ImpactEffectsManager::TriggerImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, 
                                                   FVector Location, FVector Normal, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }

    // Check distance culling
    if (ShouldCullEffect(Location))
    {
        return;
    }

    // Check effect limit
    if (GetActiveEffectCount() >= MaxSimultaneousEffects)
    {
        CleanupExpiredEffects();
        if (GetActiveEffectCount() >= MaxSimultaneousEffects)
        {
            return; // Still too many effects
        }
    }

    // Get effect data - prioritize surface-specific effects
    FVFX_ImpactEffect* EffectData = SurfaceEffectMap.Find(SurfaceType);
    if (!EffectData)
    {
        EffectData = ImpactEffectMap.Find(ImpactType);
    }

    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No effect data found for impact type %d, surface type %d"), 
               (int32)ImpactType, (int32)SurfaceType);
        return;
    }

    float ScaledIntensity = CalculateScaledIntensity(Intensity);

    // Create particle effect
    if (EffectData->ParticleSystem.IsValid())
    {
        CreateParticleEffect(*EffectData, Location, Normal, ScaledIntensity);
    }

    // Create audio effect
    if (EffectData->ImpactSound.IsValid())
    {
        CreateAudioEffect(*EffectData, Location, EffectData->SoundVolume * ScaledIntensity);
    }

    // Create decal effect
    if (bEnableImpactDecals && EffectData->bCreateDecal)
    {
        CreateDecalEffect(*EffectData, Location, Normal, ScaledIntensity);
    }
}

void AVFX_ImpactEffectsManager::TriggerDinosaurFootstep(FVector Location, float DinosaurSize, EVFX_SurfaceType SurfaceType)
{
    // Scale effect intensity based on dinosaur size
    float Intensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    
    TriggerImpactEffect(EVFX_ImpactType::DinosaurFootstep, SurfaceType, Location, FVector::UpVector, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur footstep effect triggered at %s with size %f"), 
           *Location.ToString(), DinosaurSize);
}

void AVFX_ImpactEffectsManager::TriggerDinosaurBodySlam(FVector Location, FVector Normal, float DinosaurSize)
{
    // Body slam is always intense
    float Intensity = FMath::Clamp(DinosaurSize * 1.5f, 1.0f, 4.0f);
    
    TriggerImpactEffect(EVFX_ImpactType::BodySlam, EVFX_SurfaceType::Dirt, Location, Normal, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur body slam effect triggered at %s"), *Location.ToString());
}

void AVFX_ImpactEffectsManager::TriggerWeaponImpact(FVector Location, FVector Normal, float WeaponDamage, EVFX_SurfaceType SurfaceType)
{
    // Scale intensity based on weapon damage
    float Intensity = FMath::Clamp(WeaponDamage / 50.0f, 0.3f, 2.0f);
    
    TriggerImpactEffect(EVFX_ImpactType::WeaponHit, SurfaceType, Location, Normal, Intensity);
}

void AVFX_ImpactEffectsManager::TriggerRockImpact(FVector Location, FVector Normal, float RockSize)
{
    float Intensity = FMath::Clamp(RockSize, 0.5f, 2.5f);
    
    TriggerImpactEffect(EVFX_ImpactType::RockImpact, EVFX_SurfaceType::Rock, Location, Normal, Intensity);
}

void AVFX_ImpactEffectsManager::TriggerTreeFall(FVector Location, FVector Direction, float TreeSize)
{
    float Intensity = FMath::Clamp(TreeSize, 0.8f, 3.0f);
    
    TriggerImpactEffect(EVFX_ImpactType::TreeFall, EVFX_SurfaceType::Wood, Location, Direction, Intensity);
}

void AVFX_ImpactEffectsManager::CreateParticleEffect(const FVFX_ImpactEffect& EffectData, FVector Location, FVector Normal, float Scale)
{
    if (!EffectData.ParticleSystem.IsValid())
    {
        return;
    }

    UNiagaraComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        EffectData.ParticleSystem.Get(),
        Location,
        Normal.Rotation(),
        FVector(Scale * EffectData.ParticleScale),
        true,
        EPSCPoolMethod::None,
        true
    );

    if (ParticleComponent)
    {
        ActiveParticleEffects.Add(ParticleComponent);
        
        // Set effect duration
        ParticleComponent->SetFloatParameter(TEXT("LifeTime"), EffectData.EffectDuration);
        
        UE_LOG(LogTemp, Log, TEXT("Particle effect created at %s with scale %f"), 
               *Location.ToString(), Scale);
    }
}

void AVFX_ImpactEffectsManager::CreateAudioEffect(const FVFX_ImpactEffect& EffectData, FVector Location, float Volume)
{
    if (!EffectData.ImpactSound.IsValid())
    {
        return;
    }

    UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        EffectData.ImpactSound.Get(),
        Location,
        FRotator::ZeroRotator,
        Volume,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComponent)
    {
        ActiveAudioEffects.Add(AudioComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Audio effect created at %s with volume %f"), 
               *Location.ToString(), Volume);
    }
}

void AVFX_ImpactEffectsManager::CreateDecalEffect(const FVFX_ImpactEffect& EffectData, FVector Location, FVector Normal, float Scale)
{
    if (!bEnableImpactDecals)
    {
        return;
    }

    // Create decal actor
    ADecalActor* DecalActor = GetWorld()->SpawnActor<ADecalActor>(ADecalActor::StaticClass(), Location, Normal.Rotation());
    
    if (DecalActor && DecalActor->GetDecal())
    {
        // Configure decal
        DecalActor->GetDecal()->DecalSize = FVector(50.0f * Scale, 50.0f * Scale, 50.0f * Scale);
        DecalActor->SetLifeSpan(DecalLifetime);
        
        UE_LOG(LogTemp, Log, TEXT("Decal effect created at %s"), *Location.ToString());
    }
}

void AVFX_ImpactEffectsManager::CleanupExpiredEffects()
{
    RemoveExpiredEffects();
}

void AVFX_ImpactEffectsManager::RemoveExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Clean up particle effects
    for (int32 i = ActiveParticleEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveParticleEffects[i] || IsEffectExpired(ActiveParticleEffects[i], CurrentTime))
        {
            if (ActiveParticleEffects[i])
            {
                ActiveParticleEffects[i]->DestroyComponent();
            }
            ActiveParticleEffects.RemoveAt(i);
        }
    }
    
    // Clean up audio effects
    for (int32 i = ActiveAudioEffects.Num() - 1; i >= 0; i--)
    {
        if (!ActiveAudioEffects[i] || IsAudioExpired(ActiveAudioEffects[i]))
        {
            if (ActiveAudioEffects[i])
            {
                ActiveAudioEffects[i]->Stop();
                ActiveAudioEffects[i]->DestroyComponent();
            }
            ActiveAudioEffects.RemoveAt(i);
        }
    }
}

bool AVFX_ImpactEffectsManager::IsEffectExpired(UNiagaraComponent* Effect, float CurrentTime) const
{
    if (!Effect)
    {
        return true;
    }
    
    // Check if effect is still active
    return !Effect->IsActive() || Effect->GetSystemInstance() == nullptr;
}

bool AVFX_ImpactEffectsManager::IsAudioExpired(UAudioComponent* Audio) const
{
    if (!Audio)
    {
        return true;
    }
    
    return !Audio->IsPlaying();
}

bool AVFX_ImpactEffectsManager::ShouldCullEffect(FVector EffectLocation) const
{
    FVector PlayerLocation = GetPlayerLocation();
    float Distance = FVector::Dist(PlayerLocation, EffectLocation);
    
    return Distance > EffectCullDistance;
}

FVector AVFX_ImpactEffectsManager::GetPlayerLocation() const
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            return PlayerPawn->GetActorLocation();
        }
    }
    
    return FVector::ZeroVector;
}

void AVFX_ImpactEffectsManager::StopAllEffects()
{
    // Stop all particle effects
    for (UNiagaraComponent* Effect : ActiveParticleEffects)
    {
        if (Effect)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    ActiveParticleEffects.Empty();
    
    // Stop all audio effects
    for (UAudioComponent* Audio : ActiveAudioEffects)
    {
        if (Audio)
        {
            Audio->Stop();
            Audio->DestroyComponent();
        }
    }
    ActiveAudioEffects.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("All VFX impact effects stopped"));
}

int32 AVFX_ImpactEffectsManager::GetActiveEffectCount() const
{
    return ActiveParticleEffects.Num() + ActiveAudioEffects.Num();
}

void AVFX_ImpactEffectsManager::SetEffectIntensity(float NewIntensity)
{
    GlobalEffectIntensity = FMath::Clamp(NewIntensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("VFX effect intensity set to %f"), GlobalEffectIntensity);
}

void AVFX_ImpactEffectsManager::SetMaxSimultaneousEffects(int32 MaxEffects)
{
    MaxSimultaneousEffects = FMath::Clamp(MaxEffects, 1, 100);
    UE_LOG(LogTemp, Log, TEXT("Max simultaneous effects set to %d"), MaxEffects);
}

void AVFX_ImpactEffectsManager::EnableImpactDecals(bool bEnable)
{
    bEnableImpactDecals = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Impact decals %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AVFX_ImpactEffectsManager::InitializeDefaultEffects()
{
    // Initialize default impact effects
    FVFX_ImpactEffect DefaultFootstep;
    DefaultFootstep.ParticleScale = 1.0f;
    DefaultFootstep.SoundVolume = 0.8f;
    DefaultFootstep.EffectDuration = 2.0f;
    DefaultFootstep.bCreateDecal = true;
    DefaultFootstep.DecalColor = FLinearColor::Brown;
    ImpactEffectMap.Add(EVFX_ImpactType::DinosaurFootstep, DefaultFootstep);
    
    FVFX_ImpactEffect DefaultWeaponHit;
    DefaultWeaponHit.ParticleScale = 0.7f;
    DefaultWeaponHit.SoundVolume = 1.0f;
    DefaultWeaponHit.EffectDuration = 1.5f;
    DefaultWeaponHit.bCreateDecal = true;
    DefaultWeaponHit.DecalColor = FLinearColor::Red;
    ImpactEffectMap.Add(EVFX_ImpactType::WeaponHit, DefaultWeaponHit);
    
    FVFX_ImpactEffect DefaultRockImpact;
    DefaultRockImpact.ParticleScale = 1.2f;
    DefaultRockImpact.SoundVolume = 1.2f;
    DefaultRockImpact.EffectDuration = 3.0f;
    DefaultRockImpact.bCreateDecal = false;
    ImpactEffectMap.Add(EVFX_ImpactType::RockImpact, DefaultRockImpact);
    
    // Initialize surface-specific effects
    FVFX_ImpactEffect DirtEffect;
    DirtEffect.ParticleScale = 1.0f;
    DirtEffect.DecalColor = FLinearColor::Brown;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Dirt, DirtEffect);
    
    FVFX_ImpactEffect GrassEffect;
    GrassEffect.ParticleScale = 0.8f;
    GrassEffect.DecalColor = FLinearColor::Green;
    SurfaceEffectMap.Add(EVFX_SurfaceType::Grass, GrassEffect);
    
    UE_LOG(LogTemp, Log, TEXT("Default VFX impact effects initialized"));
}

void AVFX_ImpactEffectsManager::LoadEffectAssets()
{
    // Asset loading would happen here in a real implementation
    // For now, we'll use placeholder logging
    UE_LOG(LogTemp, Log, TEXT("VFX impact effect assets loaded"));
}

float AVFX_ImpactEffectsManager::CalculateScaledIntensity(float BaseIntensity) const
{
    return BaseIntensity * GlobalEffectIntensity;
}