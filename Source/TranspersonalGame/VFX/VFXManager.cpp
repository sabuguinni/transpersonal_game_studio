#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXLibrary();
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupFinishedEffects();
    
    // Update LOD based on player location
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
    {
        UpdateLODLevels(Player->GetActorLocation());
    }
}

void AVFXManager::InitializeVFXLibrary()
{
    // Initialize VFX definitions with placeholder paths
    // These will be replaced with actual Niagara systems once created
    
    FVFXDefinition AtmosphericDust;
    AtmosphericDust.VFXType = EVFXType::AtmosphericDust;
    AtmosphericDust.Duration = -1.0f; // Looping
    AtmosphericDust.BaseScale = 1.0f;
    AtmosphericDust.bAutoDestroy = false;
    VFXLibrary.Add(EVFXType::AtmosphericDust, AtmosphericDust);

    FVFXDefinition LeafFall;
    LeafFall.VFXType = EVFXType::LeafFall;
    LeafFall.Duration = -1.0f; // Looping
    LeafFall.BaseScale = 1.0f;
    LeafFall.bAutoDestroy = false;
    VFXLibrary.Add(EVFXType::LeafFall, LeafFall);

    FVFXDefinition DinosaurBreathing;
    DinosaurBreathing.VFXType = EVFXType::Breathing;
    DinosaurBreathing.Duration = 3.0f;
    DinosaurBreathing.BaseScale = 1.0f;
    DinosaurBreathing.bAutoDestroy = true;
    VFXLibrary.Add(EVFXType::Breathing, DinosaurBreathing);

    FVFXDefinition Footsteps;
    Footsteps.VFXType = EVFXType::Footsteps;
    Footsteps.Duration = 2.0f;
    Footsteps.BaseScale = 1.0f;
    Footsteps.bAutoDestroy = true;
    VFXLibrary.Add(EVFXType::Footsteps, Footsteps);

    FVFXDefinition GemGlow;
    GemGlow.VFXType = EVFXType::GemGlow;
    GemGlow.Duration = -1.0f; // Looping
    GemGlow.BaseScale = 1.0f;
    GemGlow.bAutoDestroy = false;
    VFXLibrary.Add(EVFXType::GemGlow, GemGlow);
}

UNiagaraComponent* AVFXManager::SpawnVFX(EVFXType VFXType, FVector Location, FRotator Rotation, float Scale)
{
    if (!VFXLibrary.Contains(VFXType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Type not found in library: %d"), (int32)VFXType);
        return nullptr;
    }

    // Check if we've hit the max active effects limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max active effects reached, skipping VFX spawn"));
        return nullptr;
    }

    FVFXDefinition& VFXDef = VFXLibrary[VFXType];
    
    // Try to get from pool first
    UNiagaraComponent* Effect = GetPooledEffect(VFXType);
    
    if (!Effect && VFXDef.NiagaraSystem.LoadSynchronous())
    {
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXDef.NiagaraSystem.LoadSynchronous(),
            Location,
            Rotation
        );
    }

    if (Effect)
    {
        // Apply scale
        Effect->SetWorldScale3D(FVector(Scale * VFXDef.BaseScale));
        
        // Add to active effects
        ActiveEffects.Add(Effect);
        
        // Set auto-destroy if needed
        if (VFXDef.bAutoDestroy && VFXDef.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, 
                [this, Effect, VFXType]()
                {
                    StopVFX(Effect);
                }, 
                VFXDef.Duration, false);
        }
    }

    return Effect;
}

UNiagaraComponent* AVFXManager::AttachVFX(EVFXType VFXType, USceneComponent* AttachTo, FName SocketName)
{
    if (!AttachTo || !VFXLibrary.Contains(VFXType))
    {
        return nullptr;
    }

    FVFXDefinition& VFXDef = VFXLibrary[VFXType];
    
    if (!VFXDef.NiagaraSystem.LoadSynchronous())
    {
        return nullptr;
    }

    UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFXDef.NiagaraSystem.LoadSynchronous(),
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        VFXDef.bAutoDestroy
    );

    if (Effect)
    {
        ActiveEffects.Add(Effect);
    }

    return Effect;
}

void AVFXManager::StopVFX(UNiagaraComponent* Effect)
{
    if (!Effect)
    {
        return;
    }

    Effect->DeactivateImmediate();
    ActiveEffects.Remove(Effect);
    
    // Return to pool if possible
    // Note: Pool management would need the VFXType, which we'd need to store
    // For now, just destroy
    Effect->DestroyComponent();
}

void AVFXManager::StopAllVFXOfType(EVFXType VFXType)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (Effect)
        {
            // Would need to check VFXType if we stored it
            // For now, this is a placeholder
            StopVFX(Effect);
        }
    }
}

void AVFXManager::SetAtmosphericIntensity(float Intensity)
{
    // Find and update all atmospheric effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && Effect->GetAsset())
        {
            // Set intensity parameter on atmospheric effects
            Effect->SetFloatParameter(TEXT("Intensity"), Intensity);
        }
    }
}

void AVFXManager::TriggerEnvironmentalEvent(EVFXType EventType, FVector Location, float Radius)
{
    // Spawn environmental effect at location
    UNiagaraComponent* Effect = SpawnVFX(EventType, Location);
    
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Radius"), Radius);
    }
}

void AVFXManager::PlayDinosaurBreathing(AActor* Dinosaur, float IntensityMultiplier)
{
    if (!Dinosaur)
    {
        return;
    }

    USceneComponent* RootComp = Dinosaur->GetRootComponent();
    if (!RootComp)
    {
        return;
    }

    UNiagaraComponent* Effect = AttachVFX(EVFXType::Breathing, RootComp, TEXT("head"));
    
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Intensity"), IntensityMultiplier);
    }
}

void AVFXManager::PlayFootstepEffect(FVector Location, float DinosaurSize)
{
    UNiagaraComponent* Effect = SpawnVFX(EVFXType::Footsteps, Location);
    
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Size"), DinosaurSize);
    }
}

void AVFXManager::UpdateLODLevels(FVector PlayerLocation)
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (!Effect)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Effect->GetComponentLocation());
        int32 LODLevel = CalculateLODLevel(Distance);
        
        // Update effect LOD
        Effect->SetIntParameter(TEXT("LODLevel"), LODLevel);
        
        // Disable effects beyond cull distance
        if (Distance > CullDistance)
        {
            Effect->SetVisibility(false);
        }
        else
        {
            Effect->SetVisibility(true);
        }
    }
}

void AVFXManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        
        if (!Effect || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

UNiagaraComponent* AVFXManager::GetPooledEffect(EVFXType VFXType)
{
    if (EffectPool.Contains(VFXType) && EffectPool[VFXType].Num() > 0)
    {
        UNiagaraComponent* Effect = EffectPool[VFXType].Pop();
        return Effect;
    }
    
    return nullptr;
}

void AVFXManager::ReturnToPool(UNiagaraComponent* Effect, EVFXType VFXType)
{
    if (!Effect)
    {
        return;
    }

    Effect->DeactivateImmediate();
    Effect->SetVisibility(false);
    
    if (!EffectPool.Contains(VFXType))
    {
        EffectPool.Add(VFXType, TArray<UNiagaraComponent*>());
    }
    
    EffectPool[VFXType].Add(Effect);
}

int32 AVFXManager::CalculateLODLevel(float Distance) const
{
    if (Distance < LODDistance1)
    {
        return 0; // High quality
    }
    else if (Distance < LODDistance2)
    {
        return 1; // Medium quality
    }
    else
    {
        return 2; // Low quality
    }
}