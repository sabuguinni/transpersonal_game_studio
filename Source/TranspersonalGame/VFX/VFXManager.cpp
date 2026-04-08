// VFX Manager Implementation - Jurassic Survival Game
// Transpersonal Game Studio - VFX Agent #17
// CYCLE_ID: PROD_JURASSIC_001

#include "VFXManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize effect pools
    EffectPool.Reserve(MaxSimultaneousEffects);
    ActiveEffects.Reserve(MaxSimultaneousEffects);
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player reference for distance calculations
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Pre-populate effect pool
    for (int32 i = 0; i < MaxSimultaneousEffects / 2; i++)
    {
        UNiagaraComponent* PooledEffect = CreateDefaultSubobject<UNiagaraComponent>(
            *FString::Printf(TEXT("PooledEffect_%d"), i));
        PooledEffect->SetAutoDestroy(false);
        PooledEffect->SetVisibility(false);
        EffectPool.Add(PooledEffect);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFXManager initialized with %d pooled effects"), EffectPool.Num());
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Performance management
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdateLODLevels();
        ManageEffectPool();
        LastPerformanceCheck = 0.0f;
    }
}

UNiagaraComponent* AVFXManager::SpawnEnvironmentalEffect(const FString& EffectName, 
                                                       const FVector& Location, 
                                                       const FRotator& Rotation,
                                                       const FVector& Scale)
{
    UNiagaraSystem** FoundEffect = EnvironmentalEffects.Find(EffectName);
    if (!FoundEffect || !*FoundEffect)
    {
        UE_LOG(LogTemp, Error, TEXT("Environmental effect '%s' not found"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* Effect = GetPooledEffect();
    if (!Effect)
    {
        UE_LOG(LogTemp, Warning, TEXT("No pooled effects available, creating new one"));
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), *FoundEffect, Location, Rotation, Scale);
    }
    else
    {
        Effect->SetAsset(*FoundEffect);
        Effect->SetWorldLocationAndRotation(Location, Rotation);
        Effect->SetWorldScale3D(Scale);
        Effect->SetVisibility(true);
        Effect->Activate();
    }

    if (Effect)
    {
        ActiveEffects.Add(Effect);
        // Environmental effects are Tier 1 - always high quality
        ApplyLODToEffect(Effect, 0);
    }

    return Effect;
}

UNiagaraComponent* AVFXManager::SpawnCreatureEffect(const FString& EffectName, 
                                                  const FVector& Location, 
                                                  const FRotator& Rotation,
                                                  AActor* AttachToActor)
{
    UNiagaraSystem** FoundEffect = CreatureEffects.Find(EffectName);
    if (!FoundEffect || !*FoundEffect)
    {
        UE_LOG(LogTemp, Error, TEXT("Creature effect '%s' not found"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* Effect = GetPooledEffect();
    if (!Effect)
    {
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), *FoundEffect, Location, Rotation);
    }
    else
    {
        Effect->SetAsset(*FoundEffect);
        Effect->SetWorldLocationAndRotation(Location, Rotation);
        Effect->SetVisibility(true);
        Effect->Activate();
    }

    if (Effect && AttachToActor)
    {
        Effect->AttachToComponent(AttachToActor->GetRootComponent(), 
                                FAttachmentTransformRules::KeepWorldTransform);
    }

    if (Effect)
    {
        ActiveEffects.Add(Effect);
        // Creature effects use distance-based LOD
        int32 LODLevel = CalculateLODLevel(Location);
        ApplyLODToEffect(Effect, LODLevel);
    }

    return Effect;
}

UNiagaraComponent* AVFXManager::SpawnSurvivalEffect(const FString& EffectName, 
                                                  const FVector& Location, 
                                                  const FRotator& Rotation,
                                                  float Intensity)
{
    UNiagaraSystem** FoundEffect = SurvivalEffects.Find(EffectName);
    if (!FoundEffect || !*FoundEffect)
    {
        UE_LOG(LogTemp, Error, TEXT("Survival effect '%s' not found"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* Effect = GetPooledEffect();
    if (!Effect)
    {
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), *FoundEffect, Location, Rotation);
    }
    else
    {
        Effect->SetAsset(*FoundEffect);
        Effect->SetWorldLocationAndRotation(Location, Rotation);
        Effect->SetVisibility(true);
        Effect->Activate();
    }

    if (Effect)
    {
        // Set intensity parameter if available
        Effect->SetFloatParameter(TEXT("Intensity"), Intensity);
        ActiveEffects.Add(Effect);
        // Survival effects are gameplay critical - always high quality
        ApplyLODToEffect(Effect, 0);
    }

    return Effect;
}

UNiagaraComponent* AVFXManager::SpawnTemporalEffect(const FString& EffectName, 
                                                  const FVector& Location, 
                                                  const FRotator& Rotation,
                                                  float PowerLevel)
{
    UNiagaraSystem** FoundEffect = TemporalEffects.Find(EffectName);
    if (!FoundEffect || !*FoundEffect)
    {
        UE_LOG(LogTemp, Error, TEXT("Temporal effect '%s' not found"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* Effect = GetPooledEffect();
    if (!Effect)
    {
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), *FoundEffect, Location, Rotation);
    }
    else
    {
        Effect->SetAsset(*FoundEffect);
        Effect->SetWorldLocationAndRotation(Location, Rotation);
        Effect->SetVisibility(true);
        Effect->Activate();
    }

    if (Effect)
    {
        // Set power level parameter
        Effect->SetFloatParameter(TEXT("PowerLevel"), PowerLevel);
        ActiveEffects.Add(Effect);
        // Temporal effects are story moments - can use high quality
        ApplyLODToEffect(Effect, 0);
    }

    return Effect;
}

UNiagaraComponent* AVFXManager::SpawnDestructionEffect(const FString& EffectName, 
                                                     const FVector& Location, 
                                                     const FVector& ImpactDirection,
                                                     float Force)
{
    UNiagaraSystem** FoundEffect = DestructionEffects.Find(EffectName);
    if (!FoundEffect || !*FoundEffect)
    {
        UE_LOG(LogTemp, Error, TEXT("Destruction effect '%s' not found"), *EffectName);
        return nullptr;
    }

    FRotator Rotation = ImpactDirection.Rotation();
    UNiagaraComponent* Effect = GetPooledEffect();
    if (!Effect)
    {
        Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), *FoundEffect, Location, Rotation);
    }
    else
    {
        Effect->SetAsset(*FoundEffect);
        Effect->SetWorldLocationAndRotation(Location, Rotation);
        Effect->SetVisibility(true);
        Effect->Activate();
    }

    if (Effect)
    {
        // Set impact parameters
        Effect->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection);
        Effect->SetFloatParameter(TEXT("Force"), Force);
        ActiveEffects.Add(Effect);
        // Destruction effects use distance-based LOD
        int32 LODLevel = CalculateLODLevel(Location);
        ApplyLODToEffect(Effect, LODLevel);
    }

    return Effect;
}

void AVFXManager::StopEffect(UNiagaraComponent* Effect)
{
    if (!Effect) return;

    Effect->Deactivate();
    ActiveEffects.Remove(Effect);
    ReturnEffectToPool(Effect);
}

void AVFXManager::ClearAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            Effect->Deactivate();
            ReturnEffectToPool(Effect);
        }
    }
    ActiveEffects.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("VFXManager: All effects cleared for performance"));
}

FString AVFXManager::GetPerformanceStats() const
{
    return FString::Printf(TEXT("Active Effects: %d/%d | Pooled: %d"), 
                          ActiveEffects.Num(), MaxSimultaneousEffects, EffectPool.Num());
}

void AVFXManager::UpdateLODLevels()
{
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (!Effect) continue;
        
        FVector EffectLocation = Effect->GetComponentLocation();
        int32 LODLevel = CalculateLODLevel(EffectLocation);
        ApplyLODToEffect(Effect, LODLevel);
    }
}

void AVFXManager::ManageEffectPool()
{
    // Remove finished effects from active list
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive())
        {
            if (Effect)
            {
                ReturnEffectToPool(Effect);
            }
            ActiveEffects.RemoveAt(i);
        }
    }
    
    // If we're over the limit, stop oldest effects
    while (ActiveEffects.Num() > MaxSimultaneousEffects)
    {
        UNiagaraComponent* OldestEffect = ActiveEffects[0];
        StopEffect(OldestEffect);
    }
}

UNiagaraComponent* AVFXManager::GetPooledEffect()
{
    if (EffectPool.Num() > 0)
    {
        UNiagaraComponent* Effect = EffectPool.Pop();
        return Effect;
    }
    return nullptr;
}

void AVFXManager::ReturnEffectToPool(UNiagaraComponent* Effect)
{
    if (!Effect) return;
    
    Effect->SetVisibility(false);
    Effect->SetAsset(nullptr);
    EffectPool.Add(Effect);
}

int32 AVFXManager::CalculateLODLevel(const FVector& EffectLocation) const
{
    if (!PlayerPawn) return 2; // Lowest quality if no player reference
    
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
    
    if (Distance <= LODDistance_Tier1)
        return 0; // High quality
    else if (Distance <= LODDistance_Tier2)
        return 1; // Medium quality
    else
        return 2; // Low quality
}

void AVFXManager::ApplyLODToEffect(UNiagaraComponent* Effect, int32 LODLevel)
{
    if (!Effect) return;
    
    // Apply LOD-specific parameters
    switch (LODLevel)
    {
        case 0: // High Quality
            Effect->SetFloatParameter(TEXT("LOD_ParticleMultiplier"), 1.0f);
            Effect->SetFloatParameter(TEXT("LOD_UpdateRate"), 1.0f);
            break;
            
        case 1: // Medium Quality
            Effect->SetFloatParameter(TEXT("LOD_ParticleMultiplier"), 0.6f);
            Effect->SetFloatParameter(TEXT("LOD_UpdateRate"), 0.5f);
            break;
            
        case 2: // Low Quality
            Effect->SetFloatParameter(TEXT("LOD_ParticleMultiplier"), 0.3f);
            Effect->SetFloatParameter(TEXT("LOD_UpdateRate"), 0.25f);
            break;
    }
}