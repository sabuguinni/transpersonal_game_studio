#include "VFX_ImpactManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for cleanup
    
    // Initialize default settings
    GlobalVFXScale = 1.0f;
    GlobalAudioVolume = 1.0f;
    bEnableVFX = true;
    bEnableAudio = true;
    MinTimeBetweenEffects = 0.05f; // 50ms minimum between effects
    MaxActiveEffects = 20;
    LastEffectTime = 0.0f;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEffects();
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupFinishedEffects();
}

void UVFX_ImpactManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, const FVector& Location, const FVector& Normal, float Scale)
{
    if (!CanSpawnNewEffect())
    {
        return;
    }

    const FVFX_ImpactData* ImpactData = ImpactEffects.Find(ImpactType);
    if (!ImpactData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No impact data found for type %d"), (int32)ImpactType);
        return;
    }

    float FinalScale = Scale * ImpactData->EffectScale * GlobalVFXScale;
    
    // Spawn particle effect
    if (bEnableVFX && ImpactData->ParticleEffect)
    {
        UNiagaraComponent* ParticleComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ImpactData->ParticleEffect,
            Location,
            Normal.Rotation(),
            FVector(FinalScale)
        );
        
        if (ParticleComp)
        {
            ActiveParticleComponents.Add(ParticleComp);
            
            // Remove from tracking when finished (auto-cleanup)
            ParticleComp->SetAutoDestroy(true);
        }
    }

    // Play impact sound
    if (bEnableAudio && ImpactData->ImpactSound)
    {
        float FinalVolume = ImpactData->VolumeMultiplier * GlobalAudioVolume;
        
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            ImpactData->ImpactSound,
            Location,
            FRotator::ZeroRotator,
            FinalVolume
        );
        
        if (AudioComp)
        {
            ActiveAudioComponents.Add(AudioComp);
        }
    }

    LastEffectTime = GetWorld()->GetTimeSeconds();
}

void UVFX_ImpactManager::SpawnDinosaurFootstep(const FVector& Location, float DinosaurSize, ESurfaceType SurfaceType)
{
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;
    
    // Adjust scale based on dinosaur size
    float Scale = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    
    // Different effects based on surface type
    switch (SurfaceType)
    {
        case ESurfaceType::Grass:
        case ESurfaceType::Dirt:
            ImpactType = EVFX_ImpactType::DinosaurFootstep;
            break;
        case ESurfaceType::Water:
            ImpactType = EVFX_ImpactType::WaterSplash;
            Scale *= 1.5f; // Bigger splash
            break;
        case ESurfaceType::Rock:
            ImpactType = EVFX_ImpactType::RockImpact;
            break;
        default:
            ImpactType = EVFX_ImpactType::DinosaurFootstep;
            break;
    }
    
    SpawnImpactEffect(ImpactType, Location, FVector::UpVector, Scale);
}

void UVFX_ImpactManager::SpawnWeaponImpact(const FVector& Location, const FVector& Normal, float Damage)
{
    float Scale = FMath::Clamp(Damage / 50.0f, 0.3f, 2.0f); // Scale based on damage
    SpawnImpactEffect(EVFX_ImpactType::WeaponHit, Location, Normal, Scale);
}

void UVFX_ImpactManager::SetImpactData(EVFX_ImpactType ImpactType, const FVFX_ImpactData& NewData)
{
    ImpactEffects.Add(ImpactType, NewData);
}

FVFX_ImpactData UVFX_ImpactManager::GetImpactData(EVFX_ImpactType ImpactType) const
{
    const FVFX_ImpactData* FoundData = ImpactEffects.Find(ImpactType);
    return FoundData ? *FoundData : FVFX_ImpactData();
}

void UVFX_ImpactManager::CleanupFinishedEffects()
{
    // Remove null or finished particle components
    ActiveParticleComponents.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });
    
    // Remove null or finished audio components
    ActiveAudioComponents.RemoveAll([](UAudioComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsPlaying();
    });
}

bool UVFX_ImpactManager::CanSpawnNewEffect() const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check time limit
    if (CurrentTime - LastEffectTime < MinTimeBetweenEffects)
    {
        return false;
    }
    
    // Check active effect limit
    if (ActiveParticleComponents.Num() + ActiveAudioComponents.Num() >= MaxActiveEffects)
    {
        return false;
    }
    
    return true;
}

void UVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize with placeholder data - these will be replaced with actual assets
    FVFX_ImpactData FootstepData;
    FootstepData.EffectScale = 1.0f;
    FootstepData.VolumeMultiplier = 0.8f;
    ImpactEffects.Add(EVFX_ImpactType::DinosaurFootstep, FootstepData);
    
    FVFX_ImpactData LandingData;
    LandingData.EffectScale = 2.0f;
    LandingData.VolumeMultiplier = 1.2f;
    ImpactEffects.Add(EVFX_ImpactType::DinosaurLanding, LandingData);
    
    FVFX_ImpactData WeaponData;
    WeaponData.EffectScale = 0.5f;
    WeaponData.VolumeMultiplier = 1.0f;
    ImpactEffects.Add(EVFX_ImpactType::WeaponHit, WeaponData);
    
    FVFX_ImpactData RockData;
    RockData.EffectScale = 0.8f;
    RockData.VolumeMultiplier = 0.9f;
    ImpactEffects.Add(EVFX_ImpactType::RockImpact, RockData);
    
    FVFX_ImpactData TreeData;
    TreeData.EffectScale = 3.0f;
    TreeData.VolumeMultiplier = 1.5f;
    ImpactEffects.Add(EVFX_ImpactType::TreeFall, TreeData);
    
    FVFX_ImpactData WaterData;
    WaterData.EffectScale = 1.5f;
    WaterData.VolumeMultiplier = 0.7f;
    ImpactEffects.Add(EVFX_ImpactType::WaterSplash, WaterData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Initialized %d default impact effects"), ImpactEffects.Num());
}