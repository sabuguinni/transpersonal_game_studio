#include "VFX_ImpactEffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UVFX_ImpactEffectManager::UVFX_ImpactEffectManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxEffectDistance = 5000.0f;
    MaxActiveEffects = 20;
}

void UVFX_ImpactEffectManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultEffects();
    
    // Setup cleanup timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UVFX_ImpactEffectManager::CleanupExpiredEffects,
            2.0f,
            true
        );
    }
}

void UVFX_ImpactEffectManager::InitializeDefaultEffects()
{
    // Initialize default impact effects data
    FVFX_ImpactData FootstepLight;
    FootstepLight.EffectScale = 0.5f;
    FootstepLight.SoundVolume = 0.3f;
    FootstepLight.EffectDuration = 1.5f;
    ImpactEffects.Add(EVFX_ImpactType::FootstepLight, FootstepLight);

    FVFX_ImpactData FootstepHeavy;
    FootstepHeavy.EffectScale = 1.0f;
    FootstepHeavy.SoundVolume = 0.7f;
    FootstepHeavy.EffectDuration = 2.5f;
    ImpactEffects.Add(EVFX_ImpactType::FootstepHeavy, FootstepHeavy);

    FVFX_ImpactData FootstepMassive;
    FootstepMassive.EffectScale = 2.0f;
    FootstepMassive.SoundVolume = 1.0f;
    FootstepMassive.EffectDuration = 4.0f;
    ImpactEffects.Add(EVFX_ImpactType::FootstepMassive, FootstepMassive);

    FVFX_ImpactData WeaponHit;
    WeaponHit.EffectScale = 0.8f;
    WeaponHit.SoundVolume = 0.8f;
    WeaponHit.EffectDuration = 1.0f;
    ImpactEffects.Add(EVFX_ImpactType::WeaponHit, WeaponHit);

    FVFX_ImpactData RockImpact;
    RockImpact.EffectScale = 1.2f;
    RockImpact.SoundVolume = 0.9f;
    RockImpact.EffectDuration = 2.0f;
    ImpactEffects.Add(EVFX_ImpactType::RockImpact, RockImpact);
}

void UVFX_ImpactEffectManager::PlayImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal, float ScaleMultiplier)
{
    if (!GetWorld()) return;

    // Check distance to player
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (Player)
    {
        float Distance = FVector::Dist(Player->GetActorLocation(), Location);
        if (Distance > MaxEffectDistance)
        {
            return;
        }
    }

    // Get impact data
    const FVFX_ImpactData* ImpactData = ImpactEffects.Find(ImpactType);
    if (!ImpactData) return;

    // Calculate rotation from normal
    FRotator Rotation = FRotationMatrix::MakeFromZ(Normal).Rotator();

    // Spawn particle effect
    if (ImpactData->ParticleEffect.LoadSynchronous())
    {
        SpawnParticleEffect(
            ImpactData->ParticleEffect.Get(),
            Location,
            Rotation,
            ImpactData->EffectScale * ScaleMultiplier
        );
    }

    // Play sound
    if (ImpactData->ImpactSound.LoadSynchronous())
    {
        PlayImpactSound(
            ImpactData->ImpactSound.Get(),
            Location,
            ImpactData->SoundVolume
        );
    }
}

void UVFX_ImpactEffectManager::PlayFootstepEffect(FVector Location, float DinosaurMass, ESurfaceType SurfaceType)
{
    EVFX_ImpactType FootstepType = GetFootstepTypeFromMass(DinosaurMass);
    
    // Adjust effect based on surface type
    float ScaleMultiplier = 1.0f;
    switch (SurfaceType)
    {
        case ESurfaceType::Sand:
            ScaleMultiplier = 1.5f; // More dust in sand
            break;
        case ESurfaceType::Mud:
            ScaleMultiplier = 0.7f; // Less dust, more splash
            break;
        case ESurfaceType::Rock:
            ScaleMultiplier = 0.5f; // Minimal dust
            break;
        default:
            ScaleMultiplier = 1.0f;
            break;
    }

    PlayImpactEffect(FootstepType, Location, FVector::UpVector, ScaleMultiplier);
}

void UVFX_ImpactEffectManager::PlayWeaponImpact(FVector Location, FVector Normal, EWeaponType WeaponType, ESurfaceType SurfaceType)
{
    EVFX_ImpactType ImpactType = EVFX_ImpactType::WeaponHit;
    
    // Adjust based on weapon type
    float ScaleMultiplier = 1.0f;
    switch (WeaponType)
    {
        case EWeaponType::Spear:
            ScaleMultiplier = 0.8f;
            break;
        case EWeaponType::Club:
            ScaleMultiplier = 1.2f;
            break;
        case EWeaponType::Rock:
            ImpactType = EVFX_ImpactType::RockImpact;
            ScaleMultiplier = 1.0f;
            break;
        default:
            break;
    }

    PlayImpactEffect(ImpactType, Location, Normal, ScaleMultiplier);
}

EVFX_ImpactType UVFX_ImpactEffectManager::GetFootstepTypeFromMass(float Mass)
{
    if (Mass < 500.0f)
    {
        return EVFX_ImpactType::FootstepLight; // Human, small dinosaurs
    }
    else if (Mass < 5000.0f)
    {
        return EVFX_ImpactType::FootstepHeavy; // Medium dinosaurs
    }
    else
    {
        return EVFX_ImpactType::FootstepMassive; // Large dinosaurs (T-Rex, Brachiosaurus)
    }
}

void UVFX_ImpactEffectManager::SpawnParticleEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, float Scale)
{
    if (!Effect || !GetWorld()) return;

    // Remove oldest effect if at limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        RemoveOldestEffect();
    }

    // Spawn new effect
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        Effect,
        Location,
        Rotation,
        FVector(Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
    }
}

void UVFX_ImpactEffectManager::PlayImpactSound(USoundCue* Sound, FVector Location, float Volume)
{
    if (!Sound || !GetWorld()) return;

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        Volume,
        1.0f,
        0.0f,
        nullptr,
        nullptr,
        true
    );

    if (AudioComp)
    {
        ActiveSounds.Add(AudioComp);
    }
}

void UVFX_ImpactEffectManager::RemoveOldestEffect()
{
    if (ActiveEffects.Num() > 0)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        if (OldestEffect && IsValid(OldestEffect))
        {
            OldestEffect->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

void UVFX_ImpactEffectManager::CleanupExpiredEffects()
{
    // Clean up finished particle effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }

    // Clean up finished audio components
    for (int32 i = ActiveSounds.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* Audio = ActiveSounds[i];
        if (!Audio || !IsValid(Audio) || !Audio->IsPlaying())
        {
            ActiveSounds.RemoveAt(i);
        }
    }
}