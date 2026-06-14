#include "VFX_ImpactSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/Engine.h"

UVFX_ImpactSystem::UVFX_ImpactSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoCleanupEffects = true;
    CleanupDelay = 5.0f;
    MaxSimultaneousEffects = 20;

    // Initialize default impact effects data
    FVFX_ImpactData FootstepLightData;
    FootstepLightData.EffectScale = 0.5f;
    FootstepLightData.SoundVolume = 0.3f;
    FootstepLightData.Duration = 1.0f;
    FootstepLightData.ParticleColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Dust color
    ImpactEffects.Add(EVFX_ImpactType::FootstepLight, FootstepLightData);

    FVFX_ImpactData FootstepHeavyData;
    FootstepHeavyData.EffectScale = 1.0f;
    FootstepHeavyData.SoundVolume = 0.6f;
    FootstepHeavyData.Duration = 1.5f;
    FootstepHeavyData.ParticleColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
    ImpactEffects.Add(EVFX_ImpactType::FootstepHeavy, FootstepHeavyData);

    FVFX_ImpactData DinosaurStompData;
    DinosaurStompData.EffectScale = 2.0f;
    DinosaurStompData.SoundVolume = 1.0f;
    DinosaurStompData.Duration = 3.0f;
    DinosaurStompData.ParticleColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
    ImpactEffects.Add(EVFX_ImpactType::DinosaurStomp, DinosaurStompData);

    FVFX_ImpactData WeaponHitData;
    WeaponHitData.EffectScale = 0.8f;
    WeaponHitData.SoundVolume = 0.7f;
    WeaponHitData.Duration = 1.0f;
    WeaponHitData.ParticleColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Sparks color
    ImpactEffects.Add(EVFX_ImpactType::WeaponHit, WeaponHitData);

    FVFX_ImpactData BloodSplatterData;
    BloodSplatterData.EffectScale = 1.0f;
    BloodSplatterData.SoundVolume = 0.4f;
    BloodSplatterData.Duration = 5.0f;
    BloodSplatterData.ParticleColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Blood red
    ImpactEffects.Add(EVFX_ImpactType::BloodSplatter, BloodSplatterData);

    FVFX_ImpactData WaterSplashData;
    WaterSplashData.EffectScale = 1.2f;
    WaterSplashData.SoundVolume = 0.5f;
    WaterSplashData.Duration = 2.0f;
    WaterSplashData.ParticleColor = FLinearColor(0.6f, 0.8f, 1.0f, 0.8f); // Water blue
    ImpactEffects.Add(EVFX_ImpactType::WaterSplash, WaterSplashData);
}

void UVFX_ImpactSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Reserve space for active effects array
    ActiveEffects.Reserve(MaxSimultaneousEffects);
}

void UVFX_ImpactSystem::TriggerImpact(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation, float ScaleMultiplier)
{
    if (!ImpactEffects.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactSystem: Impact type not found in effects map"));
        return;
    }

    const FVFX_ImpactData& ImpactData = ImpactEffects[ImpactType];
    
    // Cleanup old effects if we're at the limit
    if (ActiveEffects.Num() >= MaxSimultaneousEffects)
    {
        CleanupOldEffects();
    }

    // Spawn the impact effect
    SpawnImpactEffect(ImpactData, Location, Rotation, ScaleMultiplier);
    
    // Play impact sound
    if (ImpactData.ImpactSound)
    {
        PlayImpactSound(ImpactData.ImpactSound, Location, ImpactData.SoundVolume);
    }
}

void UVFX_ImpactSystem::TriggerFootstepImpact(FVector Location, float DinosaurSize, ESurfaceType SurfaceType)
{
    EVFX_ImpactType ImpactType;
    
    // Determine impact type based on dinosaur size
    if (DinosaurSize >= 3.0f)
    {
        ImpactType = EVFX_ImpactType::DinosaurStomp;
    }
    else if (DinosaurSize >= 1.5f)
    {
        ImpactType = EVFX_ImpactType::FootstepHeavy;
    }
    else
    {
        ImpactType = EVFX_ImpactType::FootstepLight;
    }

    // Adjust scale based on surface type
    float ScaleMultiplier = 1.0f;
    switch (SurfaceType)
    {
        case ESurfaceType::Mud:
            ScaleMultiplier = 1.5f; // More splash in mud
            break;
        case ESurfaceType::Sand:
            ScaleMultiplier = 1.2f; // More dust in sand
            break;
        case ESurfaceType::Rock:
            ScaleMultiplier = 0.8f; // Less particles on rock
            break;
        case ESurfaceType::Water:
            ImpactType = EVFX_ImpactType::WaterSplash;
            ScaleMultiplier = DinosaurSize;
            break;
        default:
            break;
    }

    TriggerImpact(ImpactType, Location, FRotator::ZeroRotator, ScaleMultiplier * DinosaurSize);
}

void UVFX_ImpactSystem::TriggerWeaponImpact(FVector Location, FVector ImpactNormal, float Damage)
{
    // Calculate rotation from impact normal
    FRotator ImpactRotation = ImpactNormal.Rotation();
    
    // Scale effect based on damage
    float ScaleMultiplier = FMath::Clamp(Damage / 50.0f, 0.5f, 2.0f);
    
    TriggerImpact(EVFX_ImpactType::WeaponHit, Location, ImpactRotation, ScaleMultiplier);
}

void UVFX_ImpactSystem::TriggerBloodEffect(FVector Location, FVector Direction, float Intensity)
{
    FRotator BloodRotation = Direction.Rotation();
    float ScaleMultiplier = FMath::Clamp(Intensity, 0.3f, 3.0f);
    
    TriggerImpact(EVFX_ImpactType::BloodSplatter, Location, BloodRotation, ScaleMultiplier);
}

void UVFX_ImpactSystem::SpawnImpactEffect(const FVFX_ImpactData& ImpactData, FVector Location, FRotator Rotation, float Scale)
{
    if (!ImpactData.ParticleEffect)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn Niagara effect
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        ImpactData.ParticleEffect,
        Location,
        Rotation,
        FVector(ImpactData.EffectScale * Scale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (NiagaraComp)
    {
        // Set particle color
        NiagaraComp->SetColorParameter(TEXT("ParticleColor"), ImpactData.ParticleColor);
        
        // Add to active effects for cleanup
        ActiveEffects.Add(NiagaraComp);

        // Schedule cleanup if auto cleanup is enabled
        if (bAutoCleanupEffects)
        {
            FTimerHandle CleanupTimer;
            World->GetTimerManager().SetTimer(CleanupTimer, [this, NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    ActiveEffects.Remove(NiagaraComp);
                    NiagaraComp->DestroyComponent();
                }
            }, ImpactData.Duration + CleanupDelay, false);
        }
    }
}

void UVFX_ImpactSystem::PlayImpactSound(USoundBase* Sound, FVector Location, float Volume)
{
    if (!Sound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        Volume,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation settings
        nullptr, // Concurrency settings
        GetOwner() // Owner
    );
}

void UVFX_ImpactSystem::CleanupOldEffects()
{
    // Remove null or invalid components
    ActiveEffects.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });

    // If still over limit, destroy oldest effects
    while (ActiveEffects.Num() >= MaxSimultaneousEffects)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        if (IsValid(OldestEffect))
        {
            OldestEffect->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

void UVFX_ImpactSystem::SetImpactData(EVFX_ImpactType ImpactType, const FVFX_ImpactData& NewData)
{
    ImpactEffects.Add(ImpactType, NewData);
}

FVFX_ImpactData UVFX_ImpactSystem::GetImpactData(EVFX_ImpactType ImpactType) const
{
    if (ImpactEffects.Contains(ImpactType))
    {
        return ImpactEffects[ImpactType];
    }
    
    return FVFX_ImpactData(); // Return default data
}

ESurfaceType UVFX_ImpactSystem::GetSurfaceTypeAtLocation(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return ESurfaceType::Default;
    }

    // Perform line trace to get surface material
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.PhysMaterial.IsValid())
        {
            // Return surface type based on physical material
            return HitResult.PhysMaterial->SurfaceType;
        }
    }
    
    return ESurfaceType::Default;
}