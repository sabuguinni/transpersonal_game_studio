#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for cleanup
    
    MaxConcurrentEffects = 20;
    EffectCullDistance = 5000.0f;

    // Initialize default footstep effect
    FootstepEffect.EffectScale = FVector(1.5f, 1.5f, 1.0f);
    FootstepEffect.EffectDuration = 2.0f;
    FootstepEffect.bAttachToSurface = false;

    // Initialize default blood effect
    BloodSplatterEffect.EffectScale = FVector(1.0f, 1.0f, 1.0f);
    BloodSplatterEffect.EffectDuration = 5.0f;
    BloodSplatterEffect.bAttachToSurface = true;

    // Initialize weapon impact effect
    WeaponImpactEffect.EffectScale = FVector(0.8f, 0.8f, 0.8f);
    WeaponImpactEffect.EffectDuration = 1.5f;
    WeaponImpactEffect.bAttachToSurface = false;

    // Initialize environmental effects
    RockFallEffect.EffectScale = FVector(2.0f, 2.0f, 2.0f);
    RockFallEffect.EffectDuration = 4.0f;
    RockFallEffect.bAttachToSurface = false;

    TreeFallEffect.EffectScale = FVector(3.0f, 3.0f, 1.5f);
    TreeFallEffect.EffectDuration = 6.0f;
    TreeFallEffect.bAttachToSurface = false;

    // Initialize bite and tail swipe effects
    BiteEffect.EffectScale = FVector(1.2f, 1.2f, 1.2f);
    BiteEffect.EffectDuration = 2.5f;
    BiteEffect.bAttachToSurface = false;

    TailSwipeEffect.EffectScale = FVector(2.5f, 2.5f, 1.0f);
    TailSwipeEffect.EffectDuration = 3.0f;
    TailSwipeEffect.bAttachToSurface = false;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Reserve space for active effects array
    ActiveEffects.Reserve(MaxConcurrentEffects);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager initialized for actor: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished effects periodically
    CleanupFinishedEffects();
}

void UVFX_ImpactManager::PlayImpactEffect(const FVector& Location, const FVector& Normal, EPhysicalSurface SurfaceType, float ImpactForce)
{
    if (ShouldCullEffect(Location))
    {
        return;
    }

    // Find appropriate effect for surface type
    FVFX_ImpactData* EffectData = SurfaceImpacts.Find(SurfaceType);
    if (!EffectData)
    {
        // Use default dirt/ground effect
        EffectData = &FootstepEffect;
    }

    FRotator EffectRotation = CalculateEffectRotation(Normal);
    float ScaleMultiplier = FMath::Clamp(ImpactForce, 0.5f, 3.0f);
    
    SpawnParticleEffect(*EffectData, Location, EffectRotation, ScaleMultiplier);
    PlayImpactSound(*EffectData, Location, ScaleMultiplier);
}

void UVFX_ImpactManager::PlayFootstepEffect(const FVector& Location, const FVector& Normal, float DinosaurSize)
{
    if (ShouldCullEffect(Location))
    {
        return;
    }

    FRotator EffectRotation = CalculateEffectRotation(Normal);
    float ScaleMultiplier = FMath::Clamp(DinosaurSize, 0.3f, 5.0f);
    
    SpawnParticleEffect(FootstepEffect, Location, EffectRotation, ScaleMultiplier);
    PlayImpactSound(FootstepEffect, Location, ScaleMultiplier * 0.8f);

    UE_LOG(LogTemp, VeryVerbose, TEXT("Played footstep effect at location: %s, size: %f"), 
           *Location.ToString(), DinosaurSize);
}

void UVFX_ImpactManager::PlayBloodEffect(const FVector& Location, const FVector& Direction, float BloodAmount)
{
    if (ShouldCullEffect(Location))
    {
        return;
    }

    FRotator EffectRotation = Direction.Rotation();
    float ScaleMultiplier = FMath::Clamp(BloodAmount, 0.5f, 2.0f);
    
    SpawnParticleEffect(BloodSplatterEffect, Location, EffectRotation, ScaleMultiplier);
    PlayImpactSound(BloodSplatterEffect, Location, ScaleMultiplier);
}

void UVFX_ImpactManager::PlayWeaponImpact(const FVector& Location, const FVector& Normal, EPhysicalSurface SurfaceType)
{
    if (ShouldCullEffect(Location))
    {
        return;
    }

    FRotator EffectRotation = CalculateEffectRotation(Normal);
    
    SpawnParticleEffect(WeaponImpactEffect, Location, EffectRotation);
    PlayImpactSound(WeaponImpactEffect, Location);

    // Also play surface-specific effect if available
    PlayImpactEffect(Location, Normal, SurfaceType, 0.8f);
}

void UVFX_ImpactManager::PlayEnvironmentalEffect(const FVector& Location, bool bIsRockFall)
{
    if (ShouldCullEffect(Location))
    {
        return;
    }

    const FVFX_ImpactData& EffectData = bIsRockFall ? RockFallEffect : TreeFallEffect;
    FRotator EffectRotation = FRotator::ZeroRotator;
    
    SpawnParticleEffect(EffectData, Location, EffectRotation);
    PlayImpactSound(EffectData, Location, 1.5f);
}

void UVFX_ImpactManager::SpawnParticleEffect(const FVFX_ImpactData& EffectData, const FVector& Location, const FRotator& Rotation, float ScaleMultiplier)
{
    if (!EffectData.ParticleEffect.IsValid())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Load the Niagara system if needed
    UNiagaraSystem* NiagaraSystem = EffectData.ParticleEffect.LoadSynchronous();
    if (!NiagaraSystem)
    {
        return;
    }

    // Calculate final scale
    FVector FinalScale = EffectData.EffectScale * ScaleMultiplier;

    // Spawn the effect
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Location,
        Rotation,
        FinalScale,
        true, // Auto destroy
        true, // Auto activate
        ENCPoolMethod::None
    );

    if (SpawnedEffect)
    {
        // Track the effect for cleanup
        ActiveEffects.Add(SpawnedEffect);
        
        // Set custom duration if specified
        if (EffectData.EffectDuration > 0.0f)
        {
            SpawnedEffect->SetNiagaraVariableFloat(TEXT("User.LifeTime"), EffectData.EffectDuration);
        }

        UE_LOG(LogTemp, VeryVerbose, TEXT("Spawned VFX effect at: %s"), *Location.ToString());
    }
}

void UVFX_ImpactManager::PlayImpactSound(const FVFX_ImpactData& EffectData, const FVector& Location, float VolumeMultiplier)
{
    if (!EffectData.ImpactSound.IsValid())
    {
        return;
    }

    USoundCue* SoundCue = EffectData.ImpactSound.LoadSynchronous();
    if (!SoundCue)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        SoundCue,
        Location,
        VolumeMultiplier,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation override
        nullptr, // Concurrency override
        GetOwner() // Owner
    );
}

FRotator UVFX_ImpactManager::CalculateEffectRotation(const FVector& Normal)
{
    // Calculate rotation to align effect with surface normal
    FVector UpVector = FVector::UpVector;
    FVector ForwardVector = FVector::CrossProduct(Normal, UpVector);
    
    if (ForwardVector.IsNearlyZero())
    {
        // Normal is parallel to up vector, use a different reference
        ForwardVector = FVector::CrossProduct(Normal, FVector::ForwardVector);
    }
    
    ForwardVector.Normalize();
    FVector RightVector = FVector::CrossProduct(Normal, ForwardVector);
    
    return FRotationMatrix::MakeFromXZ(ForwardVector, Normal).Rotator();
}

void UVFX_ImpactManager::CleanupFinishedEffects()
{
    // Remove null or finished effects
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });

    // If we have too many effects, remove the oldest ones
    while (ActiveEffects.Num() > MaxConcurrentEffects)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        if (IsValid(OldestEffect))
        {
            OldestEffect->DestroyComponent();
        }
        ActiveEffects.RemoveAt(0);
    }
}

bool UVFX_ImpactManager::ShouldCullEffect(const FVector& EffectLocation) const
{
    if (!GetOwner())
    {
        return true;
    }

    // Get player camera location for distance culling
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return false; // Don't cull if we can't find player
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float DistanceSquared = FVector::DistSquared(PlayerLocation, EffectLocation);
    
    return DistanceSquared > (EffectCullDistance * EffectCullDistance);
}