#include "VFX_CretaceousImpactController.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/PlayerCameraManager.h"

UVFX_CretaceousImpactController::UVFX_CretaceousImpactController()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    MaxSimultaneousEffects = 20;
    CullingDistance = 3000.0f;
    ParticleDensityScale = 1.0f;
}

void UVFX_CretaceousImpactController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start cleanup timer for finished effects
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_CretaceousImpactController::CleanupFinishedEffects,
            2.0f,
            true
        );
    }
}

void UVFX_CretaceousImpactController::TriggerFootstepImpact(FVector Location, float CreatureWeight, EVFX_SurfaceType SurfaceType, float Velocity)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }
    
    // Limit simultaneous effects for performance
    if (ActiveComponents.Num() >= MaxSimultaneousEffects)
    {
        return;
    }
    
    // Get appropriate particle system
    UNiagaraSystem* ParticleSystem = nullptr;
    if (FootstepParticleSystems.Contains(SurfaceType))
    {
        ParticleSystem = FootstepParticleSystems[SurfaceType].LoadSynchronous();
    }
    
    if (ParticleSystem)
    {
        // Calculate effect scale based on creature weight and velocity
        float EffectScale = CalculateParticleScale(CreatureWeight, Velocity);
        EffectScale *= ParticleDensityScale;
        
        // Spawn particle effect
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParticleSystem,
            Location,
            FRotator::ZeroRotator,
            FVector(EffectScale),
            true,
            true,
            ENCPoolMethod::None
        );
        
        if (NiagaraComp)
        {
            // Set surface-specific parameters
            switch (SurfaceType)
            {
                case EVFX_SurfaceType::Mud:
                    NiagaraComp->SetFloatParameter(TEXT("ParticleLifetime"), 3.0f);
                    NiagaraComp->SetVectorParameter(TEXT("ParticleColor"), FVector(0.4f, 0.3f, 0.2f));
                    break;
                case EVFX_SurfaceType::Sand:
                    NiagaraComp->SetFloatParameter(TEXT("ParticleLifetime"), 2.0f);
                    NiagaraComp->SetVectorParameter(TEXT("ParticleColor"), FVector(0.8f, 0.7f, 0.5f));
                    break;
                case EVFX_SurfaceType::Rock:
                    NiagaraComp->SetFloatParameter(TEXT("ParticleLifetime"), 1.5f);
                    NiagaraComp->SetVectorParameter(TEXT("ParticleColor"), FVector(0.6f, 0.6f, 0.6f));
                    break;
                case EVFX_SurfaceType::Vegetation:
                    NiagaraComp->SetFloatParameter(TEXT("ParticleLifetime"), 1.0f);
                    NiagaraComp->SetVectorParameter(TEXT("ParticleColor"), FVector(0.3f, 0.6f, 0.2f));
                    break;
                default:
                    break;
            }
            
            ActiveComponents.Add(NiagaraComp);
        }
    }
    
    // Play footstep sound
    if (FootstepSounds.Contains(SurfaceType))
    {
        USoundCue* FootstepSound = FootstepSounds[SurfaceType].LoadSynchronous();
        if (FootstepSound)
        {
            float VolumeScale = FMath::Clamp(CreatureWeight / 1000.0f, 0.1f, 2.0f);
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                FootstepSound,
                Location,
                VolumeScale,
                1.0f + (Velocity - 1.0f) * 0.2f
            );
        }
    }
}

void UVFX_CretaceousImpactController::TriggerWeaponImpact(FVector Location, FVector Normal, EVFX_WeaponType WeaponType, EVFX_SurfaceType SurfaceType)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }
    
    if (ActiveComponents.Num() >= MaxSimultaneousEffects)
    {
        return;
    }
    
    // Get weapon-specific particle system
    UNiagaraSystem* ParticleSystem = nullptr;
    if (WeaponImpactSystems.Contains(WeaponType))
    {
        ParticleSystem = WeaponImpactSystems[WeaponType].LoadSynchronous();
    }
    
    if (ParticleSystem)
    {
        // Calculate rotation from surface normal
        FRotator ImpactRotation = Normal.Rotation();
        
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParticleSystem,
            Location,
            ImpactRotation,
            FVector(ParticleDensityScale),
            true,
            true,
            ENCPoolMethod::None
        );
        
        if (NiagaraComp)
        {
            // Set weapon-specific parameters
            switch (WeaponType)
            {
                case EVFX_WeaponType::Spear:
                    NiagaraComp->SetFloatParameter(TEXT("ImpactForce"), 0.8f);
                    NiagaraComp->SetFloatParameter(TEXT("SparkIntensity"), 0.3f);
                    break;
                case EVFX_WeaponType::Club:
                    NiagaraComp->SetFloatParameter(TEXT("ImpactForce"), 1.2f);
                    NiagaraComp->SetFloatParameter(TEXT("SparkIntensity"), 0.1f);
                    break;
                case EVFX_WeaponType::Stone:
                    NiagaraComp->SetFloatParameter(TEXT("ImpactForce"), 0.6f);
                    NiagaraComp->SetFloatParameter(TEXT("SparkIntensity"), 0.5f);
                    break;
                case EVFX_WeaponType::Arrow:
                    NiagaraComp->SetFloatParameter(TEXT("ImpactForce"), 0.4f);
                    NiagaraComp->SetFloatParameter(TEXT("SparkIntensity"), 0.2f);
                    break;
                default:
                    break;
            }
            
            ActiveComponents.Add(NiagaraComp);
        }
    }
    
    // Play weapon impact sound
    if (WeaponImpactSounds.Contains(WeaponType))
    {
        USoundCue* ImpactSound = WeaponImpactSounds[WeaponType].LoadSynchronous();
        if (ImpactSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                ImpactSound,
                Location,
                1.0f,
                1.0f
            );
        }
    }
}

void UVFX_CretaceousImpactController::TriggerCreatureImpact(FVector Location, float ImpactForce, EVFX_CreatureSize CreatureSize)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }
    
    if (ActiveComponents.Num() >= MaxSimultaneousEffects)
    {
        return;
    }
    
    UNiagaraSystem* ParticleSystem = CreatureCollisionSystem.LoadSynchronous();
    if (ParticleSystem)
    {
        // Scale effect based on creature size
        float SizeMultiplier = 1.0f;
        switch (CreatureSize)
        {
            case EVFX_CreatureSize::Small:
                SizeMultiplier = 0.5f;
                break;
            case EVFX_CreatureSize::Medium:
                SizeMultiplier = 1.0f;
                break;
            case EVFX_CreatureSize::Large:
                SizeMultiplier = 2.0f;
                break;
            case EVFX_CreatureSize::Massive:
                SizeMultiplier = 3.0f;
                break;
            default:
                break;
        }
        
        float EffectScale = SizeMultiplier * ParticleDensityScale;
        
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParticleSystem,
            Location,
            FRotator::ZeroRotator,
            FVector(EffectScale),
            true,
            true,
            ENCPoolMethod::None
        );
        
        if (NiagaraComp)
        {
            NiagaraComp->SetFloatParameter(TEXT("ImpactForce"), ImpactForce);
            NiagaraComp->SetFloatParameter(TEXT("CreatureScale"), SizeMultiplier);
            ActiveComponents.Add(NiagaraComp);
        }
    }
}

void UVFX_CretaceousImpactController::TriggerEnvironmentalDestruction(FVector Location, EVFX_DestructionType DestructionType, float Scale)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }
    
    if (ActiveComponents.Num() >= MaxSimultaneousEffects)
    {
        return;
    }
    
    UNiagaraSystem* ParticleSystem = nullptr;
    if (DestructionSystems.Contains(DestructionType))
    {
        ParticleSystem = DestructionSystems[DestructionType].LoadSynchronous();
    }
    
    if (ParticleSystem)
    {
        float EffectScale = Scale * ParticleDensityScale;
        
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParticleSystem,
            Location,
            FRotator::ZeroRotator,
            FVector(EffectScale),
            true,
            true,
            ENCPoolMethod::None
        );
        
        if (NiagaraComp)
        {
            ActiveComponents.Add(NiagaraComp);
        }
    }
}

void UVFX_CretaceousImpactController::CleanupFinishedEffects()
{
    for (int32 i = ActiveComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveComponents[i];
        if (!Component || !Component->IsActive() || Component->IsBeingDestroyed())
        {
            ActiveComponents.RemoveAt(i);
        }
    }
}

UNiagaraSystem* UVFX_CretaceousImpactController::GetParticleSystemForImpact(EVFX_SurfaceType SurfaceType, EVFX_ImpactType ImpactType)
{
    // Implementation for getting appropriate particle system
    // This would be expanded based on specific game requirements
    return nullptr;
}

float UVFX_CretaceousImpactController::CalculateParticleScale(float Weight, float Force)
{
    // Scale particles based on weight and force
    float WeightFactor = FMath::Clamp(Weight / 1000.0f, 0.1f, 5.0f);
    float ForceFactor = FMath::Clamp(Force, 0.1f, 3.0f);
    return WeightFactor * ForceFactor;
}

bool UVFX_CretaceousImpactController::IsWithinCullingDistance(FVector Location)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Get player camera location for distance calculation
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
    if (CameraManager)
    {
        FVector CameraLocation = CameraManager->GetCameraLocation();
        float Distance = FVector::Dist(CameraLocation, Location);
        return Distance <= CullingDistance;
    }
    
    return true; // If no camera, don't cull
}