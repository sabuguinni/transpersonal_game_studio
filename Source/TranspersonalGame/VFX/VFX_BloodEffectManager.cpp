#include "VFX_BloodEffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVFX_BloodEffectManager::AVFX_BloodEffectManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MaxBloodDecals = 50.0f;
    DecalLifetime = 30.0f;
    bEnableBloodEffects = true;
}

void AVFX_BloodEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Blood effect system initialized"));
}

void AVFX_BloodEffectManager::CreateBloodSplatter(const FVFX_BloodSplatterData& SplatterData)
{
    if (!bEnableBloodEffects)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Creating blood splatter at %s"), 
           *SplatterData.ImpactLocation.ToString());

    // Clean up old decals if we have too many
    if (ActiveBloodDecals.Num() >= MaxBloodDecals)
    {
        CleanupOldDecals();
    }

    // Spawn blood particles
    SpawnBloodParticles(SplatterData);

    // Create blood decal (placeholder - would use UDecalComponent in full implementation)
    FVector SpawnLocation = SplatterData.ImpactLocation + (SplatterData.ImpactNormal * 10.0f);
    
    AActor* BloodDecal = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    if (BloodDecal)
    {
        BloodDecal->SetActorLabel(TEXT("BloodDecal"));
        ActiveBloodDecals.Add(BloodDecal);

        // Set timer to destroy decal after lifetime
        FTimerHandle DecalTimer;
        GetWorld()->GetTimerManager().SetTimer(DecalTimer, [this, BloodDecal]()
        {
            if (IsValid(BloodDecal))
            {
                ActiveBloodDecals.Remove(BloodDecal);
                BloodDecal->Destroy();
            }
        }, DecalLifetime, false);
    }
}

void AVFX_BloodEffectManager::CreateDinosaurBloodHit(FVector HitLocation, FVector HitNormal, float Damage)
{
    FVFX_BloodSplatterData SplatterData;
    SplatterData.ImpactLocation = HitLocation;
    SplatterData.ImpactNormal = HitNormal;
    SplatterData.SplatterRadius = FMath::Clamp(Damage * 5.0f, 50.0f, 200.0f);
    SplatterData.BloodAmount = FMath::Clamp(Damage / 100.0f, 0.1f, 2.0f);
    SplatterData.BloodColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red for dinosaurs

    CreateBloodSplatter(SplatterData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Dinosaur blood hit - Damage: %f, Radius: %f"), 
           Damage, SplatterData.SplatterRadius);
}

void AVFX_BloodEffectManager::CreatePlayerBloodHit(FVector HitLocation, FVector HitNormal, float Damage)
{
    FVFX_BloodSplatterData SplatterData;
    SplatterData.ImpactLocation = HitLocation;
    SplatterData.ImpactNormal = HitNormal;
    SplatterData.SplatterRadius = FMath::Clamp(Damage * 3.0f, 30.0f, 120.0f);
    SplatterData.BloodAmount = FMath::Clamp(Damage / 50.0f, 0.2f, 1.5f);
    SplatterData.BloodColor = FLinearColor(0.9f, 0.2f, 0.2f, 1.0f); // Bright red for player

    CreateBloodSplatter(SplatterData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Player blood hit - Damage: %f, Radius: %f"), 
           Damage, SplatterData.SplatterRadius);
}

void AVFX_BloodEffectManager::ClearAllBloodDecals()
{
    for (AActor* Decal : ActiveBloodDecals)
    {
        if (IsValid(Decal))
        {
            Decal->Destroy();
        }
    }
    ActiveBloodDecals.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: All blood decals cleared"));
}

void AVFX_BloodEffectManager::CleanupOldDecals()
{
    // Remove first 10 decals (oldest ones)
    int32 DecalsToRemove = FMath::Min(10, ActiveBloodDecals.Num());
    
    for (int32 i = 0; i < DecalsToRemove; i++)
    {
        if (IsValid(ActiveBloodDecals[i]))
        {
            ActiveBloodDecals[i]->Destroy();
        }
    }
    
    ActiveBloodDecals.RemoveAt(0, DecalsToRemove);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Cleaned up %d old blood decals"), DecalsToRemove);
}

void AVFX_BloodEffectManager::SpawnBloodParticles(const FVFX_BloodSplatterData& SplatterData)
{
    // Placeholder for particle system spawning
    // In full implementation, this would spawn Niagara or legacy particle systems
    
    UE_LOG(LogTemp, Log, TEXT("VFX_BloodEffectManager: Spawning blood particles - Amount: %f, Color: %s"), 
           SplatterData.BloodAmount, *SplatterData.BloodColor.ToString());
}