#include "VFX_ImpactEffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "DrawDebugHelpers.h"

UVFX_ImpactEffectManager::UVFX_ImpactEffectManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    DefaultDustEffect = nullptr;
    DefaultWaterSplash = nullptr;
    DefaultRockImpact = nullptr;
    DefaultFootstepSound = nullptr;
    DefaultImpactSound = nullptr;
}

void UVFX_ImpactEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default effects when component starts
    InitializeDefaultEffects();
}

void UVFX_ImpactEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVFX_ImpactEffectManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location, FVector Normal)
{
    // Get effect data for this combination
    FVFX_ImpactEffectData EffectData = GetEffectData(ImpactType, SurfaceType);
    
    // Spawn particle effect if available
    if (EffectData.ParticleEffect)
    {
        FRotator EffectRotation = Normal.Rotation();
        UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            EffectData.ParticleEffect,
            Location,
            EffectRotation,
            FVector(EffectData.EffectScale)
        );
        
        if (SpawnedEffect)
        {
            // Configure effect parameters based on surface type
            switch (SurfaceType)
            {
                case EVFX_SurfaceType::Dirt:
                    SpawnedEffect->SetNiagaraVariableLinearColor(TEXT("ParticleColor"), FLinearColor(0.6f, 0.4f, 0.2f, 1.0f));
                    break;
                case EVFX_SurfaceType::Sand:
                    SpawnedEffect->SetNiagaraVariableLinearColor(TEXT("ParticleColor"), FLinearColor(0.9f, 0.8f, 0.6f, 1.0f));
                    break;
                case EVFX_SurfaceType::Mud:
                    SpawnedEffect->SetNiagaraVariableLinearColor(TEXT("ParticleColor"), FLinearColor(0.4f, 0.3f, 0.2f, 1.0f));
                    break;
                case EVFX_SurfaceType::Rock:
                    SpawnedEffect->SetNiagaraVariableLinearColor(TEXT("ParticleColor"), FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
                    break;
                case EVFX_SurfaceType::Snow:
                    SpawnedEffect->SetNiagaraVariableLinearColor(TEXT("ParticleColor"), FLinearColor(0.9f, 0.9f, 1.0f, 1.0f));
                    break;
                default:
                    break;
            }
            
            UE_LOG(LogTemp, Log, TEXT("VFX: Spawned impact effect at %s"), *Location.ToString());
        }
    }
    
    // Play impact sound if available
    if (EffectData.ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            EffectData.ImpactSound,
            Location,
            EffectData.VolumeMultiplier
        );
        
        UE_LOG(LogTemp, Log, TEXT("VFX: Played impact sound at %s"), *Location.ToString());
    }
}

void UVFX_ImpactEffectManager::SpawnFootstepEffect(FVector Location, float DinosaurSize, EVFX_SurfaceType SurfaceType)
{
    // Detect surface type if not specified
    if (SurfaceType == EVFX_SurfaceType::Dirt)
    {
        SurfaceType = DetectSurfaceType(Location);
    }
    
    // Get footstep effect data
    FVFX_ImpactEffectData EffectData = GetEffectData(EVFX_ImpactType::Footstep, SurfaceType);
    
    // Scale effect based on dinosaur size
    EffectData.EffectScale *= DinosaurSize;
    EffectData.VolumeMultiplier *= FMath::Clamp(DinosaurSize, 0.5f, 2.0f);
    
    // Spawn the effect
    SpawnImpactEffect(EVFX_ImpactType::Footstep, SurfaceType, Location, FVector::UpVector);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Spawned footstep effect for size %.2f dinosaur"), DinosaurSize);
}

void UVFX_ImpactEffectManager::SetEffectData(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, const FVFX_ImpactEffectData& EffectData)
{
    if (!ImpactEffects.Contains(ImpactType))
    {
        ImpactEffects.Add(ImpactType, TMap<EVFX_SurfaceType, FVFX_ImpactEffectData>());
    }
    
    ImpactEffects[ImpactType].Add(SurfaceType, EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Configured effect data for impact type %d, surface type %d"), 
           (int32)ImpactType, (int32)SurfaceType);
}

void UVFX_ImpactEffectManager::InitializeDefaultEffects()
{
    UE_LOG(LogTemp, Log, TEXT("VFX: Initializing default impact effects"));
    
    // Initialize footstep effects for different surfaces
    for (int32 SurfaceIndex = 0; SurfaceIndex < (int32)EVFX_SurfaceType::Snow + 1; ++SurfaceIndex)
    {
        EVFX_SurfaceType Surface = (EVFX_SurfaceType)SurfaceIndex;
        
        FVFX_ImpactEffectData FootstepData;
        FootstepData.ParticleEffect = DefaultDustEffect;
        FootstepData.ImpactSound = DefaultFootstepSound;
        FootstepData.EffectScale = 1.0f;
        FootstepData.VolumeMultiplier = 0.7f;
        FootstepData.bSpawnDecal = true;
        
        // Adjust settings per surface
        switch (Surface)
        {
            case EVFX_SurfaceType::Water:
                FootstepData.ParticleEffect = DefaultWaterSplash;
                FootstepData.VolumeMultiplier = 0.5f;
                FootstepData.bSpawnDecal = false;
                break;
            case EVFX_SurfaceType::Rock:
                FootstepData.ParticleEffect = DefaultRockImpact;
                FootstepData.VolumeMultiplier = 0.8f;
                FootstepData.EffectScale = 0.7f;
                break;
            case EVFX_SurfaceType::Snow:
                FootstepData.VolumeMultiplier = 0.3f;
                FootstepData.EffectScale = 1.2f;
                break;
            default:
                break;
        }
        
        SetEffectData(EVFX_ImpactType::Footstep, Surface, FootstepData);
    }
    
    // Initialize weapon hit effects
    FVFX_ImpactEffectData WeaponHitData;
    WeaponHitData.ParticleEffect = DefaultRockImpact;
    WeaponHitData.ImpactSound = DefaultImpactSound;
    WeaponHitData.EffectScale = 0.8f;
    WeaponHitData.VolumeMultiplier = 1.0f;
    WeaponHitData.bSpawnDecal = true;
    
    for (int32 SurfaceIndex = 0; SurfaceIndex < (int32)EVFX_SurfaceType::Snow + 1; ++SurfaceIndex)
    {
        SetEffectData(EVFX_ImpactType::WeaponHit, (EVFX_SurfaceType)SurfaceIndex, WeaponHitData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Default effects initialization complete"));
}

FVFX_ImpactEffectData UVFX_ImpactEffectManager::GetEffectData(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType)
{
    if (ImpactEffects.Contains(ImpactType) && ImpactEffects[ImpactType].Contains(SurfaceType))
    {
        return ImpactEffects[ImpactType][SurfaceType];
    }
    
    // Return default effect data if specific combination not found
    FVFX_ImpactEffectData DefaultData;
    DefaultData.ParticleEffect = DefaultDustEffect;
    DefaultData.ImpactSound = DefaultFootstepSound;
    DefaultData.EffectScale = 1.0f;
    DefaultData.VolumeMultiplier = 0.5f;
    DefaultData.bSpawnDecal = false;
    
    UE_LOG(LogTemp, Warning, TEXT("VFX: Using default effect data for impact %d, surface %d"), 
           (int32)ImpactType, (int32)SurfaceType);
    
    return DefaultData;
}

EVFX_SurfaceType UVFX_ImpactEffectManager::DetectSurfaceType(FVector Location)
{
    // Simple biome-based surface detection using coordinates from Hugo's memories
    float X = Location.X;
    float Y = Location.Y;
    float Z = Location.Z;
    
    // Pantano (sudoeste) - X(-77500 a -25000), Y(-76500 a -15000)
    if (X >= -77500.0f && X <= -25000.0f && Y >= -76500.0f && Y <= -15000.0f)
    {
        return (Z < 50.0f) ? EVFX_SurfaceType::Mud : EVFX_SurfaceType::Water;
    }
    
    // Floresta (noroeste) - X(-77500 a -15000), Y(15000 a 76500)
    if (X >= -77500.0f && X <= -15000.0f && Y >= 15000.0f && Y <= 76500.0f)
    {
        return EVFX_SurfaceType::Grass;
    }
    
    // Deserto (leste) - X(25000 a 79500), Y(-30000 a 30000)
    if (X >= 25000.0f && X <= 79500.0f && Y >= -30000.0f && Y <= 30000.0f)
    {
        return EVFX_SurfaceType::Sand;
    }
    
    // Montanha Nevada (nordeste) - X(15000 a 79500), Y(20000 a 76500)
    if (X >= 15000.0f && X <= 79500.0f && Y >= 20000.0f && Y <= 76500.0f && Z > 300.0f)
    {
        return EVFX_SurfaceType::Snow;
    }
    
    // Savana (centro) - default
    return EVFX_SurfaceType::Dirt;
}