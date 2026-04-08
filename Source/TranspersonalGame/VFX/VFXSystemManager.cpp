#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance profile based on platform
    SetPerformanceProfile(2); // Default to High
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized with %d registered effects"), 
           RegisteredEffects.Num());
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupFinishedEffects();
    UpdatePerformanceLOD();
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXEffect(const FString& EffectName, 
                                                    const FVector& Location, 
                                                    const FRotator& Rotation, 
                                                    AActor* AttachToActor)
{
    if (!CanSpawnEffect(EffectName))
    {
        return nullptr;
    }

    FVFXDefinition* EffectDef = FindEffectDefinition(EffectName);
    if (!EffectDef || !EffectDef->NiagaraSystem.LoadSynchronous())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found or failed to load: %s"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* NewEffect = nullptr;
    
    if (AttachToActor)
    {
        NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            EffectDef->NiagaraSystem.LoadSynchronous(),
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            EffectDef->NiagaraSystem.LoadSynchronous(),
            Location,
            Rotation,
            FVector(1.0f),
            true
        );
    }

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        EffectCounts.FindOrAdd(EffectName)++;
        
        // Calculate and add performance cost
        float Distance = FVector::Dist(Location, GetActorLocation());
        CurrentPerformanceCost += CalculateEffectCost(*EffectDef, Distance);
        
        // Apply immediate LOD based on distance
        ApplyLODToEffect(NewEffect, Distance);
    }

    return NewEffect;
}

void AVFXSystemManager::StopVFXEffect(UNiagaraComponent* Effect)
{
    if (Effect && IsValid(Effect))
    {
        Effect->Deactivate();
        ActiveEffects.Remove(Effect);
        
        // Update performance cost (simplified - should track individual costs)
        CurrentPerformanceCost = FMath::Max(0.0f, CurrentPerformanceCost - 1.0f);
    }
}

void AVFXSystemManager::StopAllEffectsOfCategory(EVFXCategory Category)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (IsValid(ActiveEffects[i]))
        {
            // Would need to store category with each effect for proper filtering
            // For now, stop all
            ActiveEffects[i]->Deactivate();
        }
    }
    
    CleanupFinishedEffects();
}

void AVFXSystemManager::UpdatePerformanceLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
        return;

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            float Distance = FVector::Dist(Effect->GetComponentLocation(), PlayerLocation);
            ApplyLODToEffect(Effect, Distance);
        }
    }
}

bool AVFXSystemManager::CanSpawnEffect(const FString& EffectName) const
{
    // Check performance budget
    if (CurrentPerformanceCost >= PerformanceBudget)
    {
        return false;
    }

    // Check max active effects
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        return false;
    }

    // Check if effect exists
    FVFXDefinition* EffectDef = const_cast<AVFXSystemManager*>(this)->FindEffectDefinition(EffectName);
    return EffectDef != nullptr;
}

void AVFXSystemManager::SetPerformanceProfile(int32 ProfileLevel)
{
    switch (ProfileLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            PerformanceBudget = 30.0f;
            LODDistance_High = 500.0f;
            LODDistance_Medium = 1500.0f;
            LODDistance_Low = 2500.0f;
            break;
            
        case 1: // Medium
            MaxActiveEffects = 35;
            PerformanceBudget = 60.0f;
            LODDistance_High = 750.0f;
            LODDistance_Medium = 2000.0f;
            LODDistance_Low = 3500.0f;
            break;
            
        case 2: // High
            MaxActiveEffects = 50;
            PerformanceBudget = 100.0f;
            LODDistance_High = 1000.0f;
            LODDistance_Medium = 3000.0f;
            LODDistance_Low = 5000.0f;
            break;
            
        case 3: // Ultra
            MaxActiveEffects = 75;
            PerformanceBudget = 150.0f;
            LODDistance_High = 1500.0f;
            LODDistance_Medium = 4000.0f;
            LODDistance_Low = 7000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Performance Profile set to level %d"), ProfileLevel);
}

void AVFXSystemManager::TriggerNarrativeVFX(const FString& MomentName, const FVector& Location)
{
    // Map narrative moments to specific VFX
    if (MomentName == "GemstonePulse")
    {
        SpawnVFXEffect("Narrative_GemstonePulse", Location);
    }
    else if (MomentName == "DinosaurFirstSight")
    {
        SpawnVFXEffect("Creature_PresenceMajor", Location);
        SpawnVFXEffect("Environmental_TensionFog", Location);
    }
    else if (MomentName == "DomesticationSuccess")
    {
        SpawnVFXEffect("Interaction_TrustBond", Location);
    }
}

void AVFXSystemManager::SetAtmosphericIntensity(float Intensity)
{
    // Modulate all environmental effects based on narrative tension
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->SetFloatParameter("AtmosphericIntensity", Intensity);
        }
    }
}

void AVFXSystemManager::PlayCreaturePresenceVFX(AActor* Creature, float IntensityMultiplier)
{
    if (!Creature)
        return;

    FVector CreatureLocation = Creature->GetActorLocation();
    
    // Subtle breath/presence effect
    SpawnVFXEffect("Creature_PresenceSubtle", CreatureLocation, FRotator::ZeroRotator, Creature);
    
    // Footstep dust based on movement
    if (IntensityMultiplier > 0.5f)
    {
        SpawnVFXEffect("Creature_FootstepDust", CreatureLocation);
    }
}

void AVFXSystemManager::PlayDomesticationFeedbackVFX(AActor* Creature, float TrustLevel)
{
    if (!Creature)
        return;

    FVector CreatureLocation = Creature->GetActorLocation();
    
    if (TrustLevel > 0.8f)
    {
        SpawnVFXEffect("Interaction_HighTrust", CreatureLocation, FRotator::ZeroRotator, Creature);
    }
    else if (TrustLevel > 0.5f)
    {
        SpawnVFXEffect("Interaction_MediumTrust", CreatureLocation, FRotator::ZeroRotator, Creature);
    }
    else if (TrustLevel > 0.2f)
    {
        SpawnVFXEffect("Interaction_LowTrust", CreatureLocation, FRotator::ZeroRotator, Creature);
    }
}

void AVFXSystemManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
            CurrentPerformanceCost = FMath::Max(0.0f, CurrentPerformanceCost - 1.0f);
        }
    }
}

void AVFXSystemManager::ApplyLODToEffect(UNiagaraComponent* Effect, float Distance)
{
    if (!IsValid(Effect))
        return;

    float LODLevel = 1.0f;
    
    if (Distance > LODDistance_Low)
    {
        // Deactivate very distant effects
        Effect->Deactivate();
        return;
    }
    else if (Distance > LODDistance_Medium)
    {
        LODLevel = 0.3f; // Low quality
    }
    else if (Distance > LODDistance_High)
    {
        LODLevel = 0.6f; // Medium quality
    }
    else
    {
        LODLevel = 1.0f; // High quality
    }
    
    // Apply LOD parameters to Niagara system
    Effect->SetFloatParameter("LODMultiplier", LODLevel);
    Effect->SetFloatParameter("ParticleCountMultiplier", LODLevel);
}

FVFXDefinition* AVFXSystemManager::FindEffectDefinition(const FString& EffectName)
{
    for (FVFXDefinition& Effect : RegisteredEffects)
    {
        if (Effect.EffectName == EffectName)
        {
            return &Effect;
        }
    }
    return nullptr;
}

float AVFXSystemManager::CalculateEffectCost(const FVFXDefinition& Definition, float Distance) const
{
    float BaseCost = Definition.BaseCost;
    
    // Reduce cost based on distance
    if (Distance > LODDistance_High)
    {
        BaseCost *= 0.5f;
    }
    
    if (Distance > LODDistance_Medium)
    {
        BaseCost *= 0.3f;
    }
    
    return BaseCost;
}