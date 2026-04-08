#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeVFXRegistry();
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Performance management
    CleanupFinishedEffects();
    UpdateVFXLOD();
}

void AVFXManager::InitializeVFXRegistry()
{
    // Environment Effects
    FVFXDefinition FootstepDust;
    FootstepDust.EffectName = FName("FootstepDust");
    FootstepDust.Category = EVFXCategory::Environment;
    FootstepDust.Priority = EVFXPriority::Low;
    FootstepDust.MaxDistance = 1000.0f;
    FootstepDust.MaxInstances = 20;
    FootstepDust.Duration = 2.0f;
    VFXRegistry.Add(FootstepDust.EffectName, FootstepDust);

    FVFXDefinition LeafFall;
    LeafFall.EffectName = FName("LeafFall");
    LeafFall.Category = EVFXCategory::Atmosphere;
    LeafFall.Priority = EVFXPriority::Background;
    LeafFall.MaxDistance = 3000.0f;
    LeafFall.MaxInstances = 50;
    LeafFall.Duration = 10.0f;
    VFXRegistry.Add(LeafFall.EffectName, LeafFall);

    // Combat Effects
    FVFXDefinition BloodSplatter;
    BloodSplatter.EffectName = FName("BloodSplatter");
    BloodSplatter.Category = EVFXCategory::Combat;
    BloodSplatter.Priority = EVFXPriority::Critical;
    BloodSplatter.MaxDistance = 2000.0f;
    BloodSplatter.MaxInstances = 15;
    BloodSplatter.Duration = 5.0f;
    VFXRegistry.Add(BloodSplatter.EffectName, BloodSplatter);

    FVFXDefinition DinosaurBreath;
    DinosaurBreath.EffectName = FName("DinosaurBreath");
    DinosaurBreath.Category = EVFXCategory::Atmosphere;
    DinosaurBreath.Priority = EVFXPriority::High;
    DinosaurBreath.MaxDistance = 1500.0f;
    DinosaurBreath.MaxInstances = 10;
    DinosaurBreath.Duration = 3.0f;
    DinosaurBreath.bAttachToActor = true;
    DinosaurBreath.AttachSocket = FName("MouthSocket");
    VFXRegistry.Add(DinosaurBreath.EffectName, DinosaurBreath);

    // Weather Effects
    FVFXDefinition RainDrops;
    RainDrops.EffectName = FName("RainDrops");
    RainDrops.Category = EVFXCategory::Weather;
    RainDrops.Priority = EVFXPriority::Medium;
    RainDrops.MaxDistance = 5000.0f;
    RainDrops.MaxInstances = 100;
    RainDrops.Duration = -1.0f; // Persistent
    VFXRegistry.Add(RainDrops.EffectName, RainDrops);

    // Magic/Time Travel Effects
    FVFXDefinition GemGlow;
    GemGlow.EffectName = FName("GemGlow");
    GemGlow.Category = EVFXCategory::Magic;
    GemGlow.Priority = EVFXPriority::Critical;
    GemGlow.MaxDistance = 10000.0f;
    GemGlow.MaxInstances = 1;
    GemGlow.Duration = -1.0f; // Persistent
    VFXRegistry.Add(GemGlow.EffectName, GemGlow);

    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Initialized %d effects in registry"), VFXRegistry.Num());
}

UNiagaraComponent* AVFXManager::SpawnVFX(FName EffectName, FVector Location, FRotator Rotation, AActor* AttachActor)
{
    if (!CanSpawnEffect(EffectName, Location))
    {
        return nullptr;
    }

    FVFXDefinition* EffectDef = VFXRegistry.Find(EffectName);
    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Effect %s not found in registry"), *EffectName.ToString());
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = EffectDef->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Niagara system not loaded for effect %s"), *EffectName.ToString());
        return nullptr;
    }

    UNiagaraComponent* NewEffect = nullptr;

    if (AttachActor && EffectDef->bAttachToActor)
    {
        NewEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachActor->GetRootComponent(),
            EffectDef->AttachSocket,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            EffectDef->bAutoDestroy
        );
    }
    else
    {
        NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation,
            FVector::OneVector,
            EffectDef->bAutoDestroy
        );
    }

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Update instance count
        int32* CurrentCount = EffectInstanceCounts.Find(EffectName);
        if (CurrentCount)
        {
            (*CurrentCount)++;
        }
        else
        {
            EffectInstanceCounts.Add(EffectName, 1);
        }

        // Set initial LOD based on distance
        float Distance = GetDistanceToPlayer(Location);
        int32 LODLevel = GetLODLevel(Distance);
        UpdateEffectLOD(NewEffect, LODLevel);

        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned effect %s at distance %.2f (LOD %d)"), 
               *EffectName.ToString(), Distance, LODLevel);
    }

    return NewEffect;
}

void AVFXManager::StopVFX(UNiagaraComponent* Effect)
{
    if (Effect && IsValid(Effect))
    {
        Effect->DeactivateImmediate();
        ActiveEffects.Remove(Effect);
    }
}

void AVFXManager::StopAllVFXByCategory(EVFXCategory Category)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }

        // Find effect definition to check category
        for (const auto& RegistryPair : VFXRegistry)
        {
            if (RegistryPair.Value.Category == Category)
            {
                // This is a simplified check - in practice you'd need to track which effect is which
                Effect->DeactivateImmediate();
                ActiveEffects.RemoveAt(i);
                break;
            }
        }
    }
}

void AVFXManager::RegisterVFX(FName EffectName, FVFXDefinition Definition)
{
    VFXRegistry.Add(EffectName, Definition);
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Registered new effect %s"), *EffectName.ToString());
}

bool AVFXManager::IsVFXRegistered(FName EffectName)
{
    return VFXRegistry.Contains(EffectName);
}

void AVFXManager::UpdateVFXLOD()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }

        FVector EffectLocation = Effect->GetComponentLocation();
        float Distance = GetDistanceToPlayer(EffectLocation);
        int32 LODLevel = GetLODLevel(Distance);
        
        UpdateEffectLOD(Effect, LODLevel);
    }
}

void AVFXManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }

    // Limit total active effects for performance
    while (ActiveEffects.Num() > MaxActiveEffects)
    {
        // Remove lowest priority effects first
        UNiagaraComponent* LowestPriorityEffect = nullptr;
        int32 LowestPriorityIndex = -1;
        
        for (int32 i = 0; i < ActiveEffects.Num(); i++)
        {
            UNiagaraComponent* Effect = ActiveEffects[i];
            if (IsValid(Effect))
            {
                // Simple priority check - in practice you'd track effect priorities
                LowestPriorityEffect = Effect;
                LowestPriorityIndex = i;
                break;
            }
        }

        if (LowestPriorityEffect)
        {
            StopVFX(LowestPriorityEffect);
            ActiveEffects.RemoveAt(LowestPriorityIndex);
        }
        else
        {
            break;
        }
    }
}

float AVFXManager::GetDistanceToPlayer(FVector Location)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        return FVector::Dist(Location, PlayerLocation);
    }
    return 0.0f;
}

int32 AVFXManager::GetLODLevel(float Distance)
{
    if (Distance <= LODDistance1)
        return 0; // Highest quality
    else if (Distance <= LODDistance2)
        return 1; // Medium quality
    else if (Distance <= LODDistance3)
        return 2; // Low quality
    else
        return 3; // Minimal/disabled
}

bool AVFXManager::CanSpawnEffect(FName EffectName, FVector Location)
{
    FVFXDefinition* EffectDef = VFXRegistry.Find(EffectName);
    if (!EffectDef)
        return false;

    // Check distance limit
    float Distance = GetDistanceToPlayer(Location);
    if (Distance > EffectDef->MaxDistance)
        return false;

    // Check instance limit
    int32* CurrentCount = EffectInstanceCounts.Find(EffectName);
    if (CurrentCount && *CurrentCount >= EffectDef->MaxInstances)
        return false;

    // Check total active effects limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
        return false;

    return true;
}

void AVFXManager::UpdateEffectLOD(UNiagaraComponent* Effect, int32 LODLevel)
{
    if (!Effect || !IsValid(Effect))
        return;

    // Set Niagara LOD parameters based on level
    switch (LODLevel)
    {
        case 0: // High Quality
            Effect->SetFloatParameter(FName("LOD_ParticleCount"), 1.0f);
            Effect->SetFloatParameter(FName("LOD_UpdateRate"), 1.0f);
            break;
        case 1: // Medium Quality
            Effect->SetFloatParameter(FName("LOD_ParticleCount"), 0.7f);
            Effect->SetFloatParameter(FName("LOD_UpdateRate"), 0.8f);
            break;
        case 2: // Low Quality
            Effect->SetFloatParameter(FName("LOD_ParticleCount"), 0.4f);
            Effect->SetFloatParameter(FName("LOD_UpdateRate"), 0.5f);
            break;
        case 3: // Minimal/Disabled
            Effect->SetFloatParameter(FName("LOD_ParticleCount"), 0.1f);
            Effect->SetFloatParameter(FName("LOD_UpdateRate"), 0.2f);
            break;
    }
}