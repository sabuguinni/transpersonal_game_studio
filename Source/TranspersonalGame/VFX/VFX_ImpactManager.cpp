#include "VFX_ImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    MaxImpactDistance = 5000.0f;
    MaxActiveEffects = 50;
    CleanupInterval = 5.0f;
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEffects();
    
    // Start cleanup timer
    GetWorldTimerManager().SetTimer(CleanupTimerHandle, this, &AVFX_ImpactManager::CleanupOldEffects, CleanupInterval, true);
}

void AVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize default impact effects data
    FVFX_ImpactData DirtFootstep;
    DirtFootstep.ParticleScale = FVector(1.0f, 1.0f, 0.8f);
    DirtFootstep.Duration = 1.5f;
    DirtFootstep.bAttachToSurface = false;
    ImpactEffects.Add(EVFX_ImpactType::FootstepDirt, DirtFootstep);

    FVFX_ImpactData RockFootstep;
    RockFootstep.ParticleScale = FVector(0.7f, 0.7f, 0.5f);
    RockFootstep.Duration = 1.0f;
    RockFootstep.bAttachToSurface = false;
    ImpactEffects.Add(EVFX_ImpactType::FootstepRock, RockFootstep);

    FVFX_ImpactData WaterFootstep;
    WaterFootstep.ParticleScale = FVector(1.2f, 1.2f, 0.6f);
    WaterFootstep.Duration = 2.0f;
    WaterFootstep.bAttachToSurface = false;
    ImpactEffects.Add(EVFX_ImpactType::FootstepWater, WaterFootstep);

    FVFX_ImpactData BloodHit;
    BloodHit.ParticleScale = FVector(0.8f, 0.8f, 0.8f);
    BloodHit.Duration = 3.0f;
    BloodHit.bAttachToSurface = true;
    ImpactEffects.Add(EVFX_ImpactType::BloodSplatter, BloodHit);

    FVFX_ImpactData WeaponFlesh;
    WeaponFlesh.ParticleScale = FVector(0.6f, 0.6f, 0.6f);
    WeaponFlesh.Duration = 1.5f;
    WeaponFlesh.bAttachToSurface = false;
    ImpactEffects.Add(EVFX_ImpactType::WeaponHitFlesh, WeaponFlesh);

    FVFX_ImpactData FallDamage;
    FallDamage.ParticleScale = FVector(1.5f, 1.5f, 1.0f);
    FallDamage.Duration = 2.5f;
    FallDamage.bAttachToSurface = false;
    ImpactEffects.Add(EVFX_ImpactType::FallImpact, FallDamage);
}

void AVFX_ImpactManager::PlayImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal, float Scale)
{
    if (!ImpactEffects.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No effect data for impact type"));
        return;
    }

    const FVFX_ImpactData& EffectData = ImpactEffects[ImpactType];
    
    // Check if we have a valid particle system
    if (!EffectData.ParticleSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No particle system assigned for impact type"));
        return;
    }

    // Calculate rotation from normal
    FRotator Rotation = FRotationMatrix::MakeFromZ(Normal).Rotator();
    
    // Apply scale
    FVector FinalScale = EffectData.ParticleScale * Scale;
    
    // Spawn the effect
    UNiagaraComponent* EffectComponent = SpawnEffectAtLocation(EffectData.ParticleSystem.Get(), Location, Rotation, FinalScale);
    
    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        
        // Clean up if we have too many effects
        if (ActiveEffects.Num() > MaxActiveEffects)
        {
            CleanupOldEffects();
        }
    }
}

void AVFX_ImpactManager::PlayFootstepEffect(EDinosaurSpecies Species, FVector Location, ESurfaceType SurfaceType)
{
    EVFX_ImpactType ImpactType = GetImpactTypeForSurface(SurfaceType);
    
    // Scale based on dinosaur species
    float Scale = 1.0f;
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            Scale = 3.0f;
            break;
        case EDinosaurSpecies::Brachiosaurus:
            Scale = 4.0f;
            break;
        case EDinosaurSpecies::Raptor:
            Scale = 0.8f;
            break;
        case EDinosaurSpecies::Triceratops:
            Scale = 2.5f;
            break;
        case EDinosaurSpecies::Pteranodon:
            Scale = 0.4f;
            break;
        default:
            Scale = 1.0f;
            break;
    }
    
    PlayImpactEffect(ImpactType, Location, FVector::UpVector, Scale);
}

void AVFX_ImpactManager::PlayCombatImpact(FVector Location, float Damage, bool bIsBloodHit)
{
    EVFX_ImpactType ImpactType = bIsBloodHit ? EVFX_ImpactType::BloodSplatter : EVFX_ImpactType::WeaponHitFlesh;
    
    // Scale effect based on damage
    float Scale = FMath::Clamp(Damage / 50.0f, 0.5f, 2.0f);
    
    PlayImpactEffect(ImpactType, Location, FVector::UpVector, Scale);
}

UNiagaraComponent* AVFX_ImpactManager::SpawnEffectAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        Scale,
        true,  // bAutoDestroy
        true,  // bAutoActivate
        ENCPoolMethod::None,
        true   // bPreCullCheck
    );

    return NiagaraComponent;
}

void AVFX_ImpactManager::CleanupOldEffects()
{
    RemoveFinishedEffects();
    
    // If we still have too many, remove oldest ones
    while (ActiveEffects.Num() > MaxActiveEffects)
    {
        if (ActiveEffects.Num() > 0 && IsValid(ActiveEffects[0]))
        {
            ActiveEffects[0]->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

void AVFX_ImpactManager::RemoveFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFX_ImpactManager::SetMaxActiveEffects(int32 NewMax)
{
    MaxActiveEffects = FMath::Max(NewMax, 10);
    CleanupOldEffects();
}

EVFX_ImpactType AVFX_ImpactManager::GetImpactTypeForSurface(ESurfaceType SurfaceType)
{
    switch (SurfaceType)
    {
        case ESurfaceType::Dirt:
        case ESurfaceType::Grass:
        case ESurfaceType::Sand:
            return EVFX_ImpactType::FootstepDirt;
            
        case ESurfaceType::Rock:
        case ESurfaceType::Stone:
            return EVFX_ImpactType::FootstepRock;
            
        case ESurfaceType::Water:
        case ESurfaceType::Mud:
            return EVFX_ImpactType::FootstepWater;
            
        default:
            return EVFX_ImpactType::FootstepDirt;
    }
}