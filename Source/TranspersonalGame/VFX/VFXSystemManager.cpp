#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update VFX every 100ms
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized"));
    
    // Reserve space for active effects
    ActiveEffects.Reserve(MaxActiveEffects);
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= EffectCleanupInterval)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
    
    // Update LOD for all active effects
    if (bEnableVFXLOD)
    {
        for (UParticleSystemComponent* Effect : ActiveEffects)
        {
            if (Effect && IsValid(Effect))
            {
                UpdateEffectLOD(Effect, GetDistanceToPlayer(Effect->GetComponentLocation()));
            }
        }
    }
}

void UVFX_SystemManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System: Max effects reached, skipping spawn"));
        return;
    }
    
    FVFX_EffectData EffectData;
    EffectData.EffectType = EffectType;
    EffectData.Location = Location;
    EffectData.Rotation = Rotation;
    EffectData.Scale = Scale;
    EffectData.Duration = 3.0f;
    EffectData.bAutoDestroy = true;
    
    UParticleSystemComponent* NewEffect = CreateParticleEffect(EffectType, EffectData);
    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned: %s at %s"), 
               *UEnum::GetValueAsString(EffectType), *Location.ToString());
    }
}

void UVFX_SystemManager::SpawnDinosaurFootstepEffect(FVector ImpactLocation, float DinosaurSize)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::DinosaurFootstep;
    EffectData.Location = ImpactLocation;
    EffectData.Scale = DinosaurSize;
    EffectData.Duration = 2.0f;
    
    UParticleSystemComponent* FootstepEffect = CreateParticleEffect(EVFX_EffectType::DinosaurFootstep, EffectData);
    if (FootstepEffect)
    {
        ActiveEffects.Add(FootstepEffect);
        UE_LOG(LogTemp, Log, TEXT("Dinosaur footstep effect spawned at %s, size: %f"), 
               *ImpactLocation.ToString(), DinosaurSize);
    }
}

void UVFX_SystemManager::SpawnWaterSplashEffect(FVector WaterLocation, float SplashIntensity)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::WaterSplash;
    EffectData.Location = WaterLocation;
    EffectData.Scale = SplashIntensity;
    EffectData.Duration = 1.5f;
    
    UParticleSystemComponent* SplashEffect = CreateParticleEffect(EVFX_EffectType::WaterSplash, EffectData);
    if (SplashEffect)
    {
        ActiveEffects.Add(SplashEffect);
        UE_LOG(LogTemp, Log, TEXT("Water splash effect spawned at %s"), *WaterLocation.ToString());
    }
}

void UVFX_SystemManager::SpawnBloodEffect(FVector HitLocation, FVector HitDirection)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EVFX_EffectType::BloodSpray;
    EffectData.Location = HitLocation;
    EffectData.Rotation = HitDirection.Rotation();
    EffectData.Duration = 1.0f;
    
    UParticleSystemComponent* BloodEffect = CreateParticleEffect(EVFX_EffectType::BloodSpray, EffectData);
    if (BloodEffect)
    {
        ActiveEffects.Add(BloodEffect);
        UE_LOG(LogTemp, Log, TEXT("Blood effect spawned at %s"), *HitLocation.ToString());
    }
}

void UVFX_SystemManager::SpawnCampfireEffect(FVector FireLocation)
{
    // Spawn fire effect
    FVFX_EffectData FireData;
    FireData.EffectType = EVFX_EffectType::CampfireFire;
    FireData.Location = FireLocation;
    FireData.Duration = 60.0f; // Long duration for campfire
    FireData.bAutoDestroy = false; // Manual cleanup for persistent effects
    
    UParticleSystemComponent* FireEffect = CreateParticleEffect(EVFX_EffectType::CampfireFire, FireData);
    if (FireEffect)
    {
        ActiveEffects.Add(FireEffect);
    }
    
    // Spawn smoke effect
    FVFX_EffectData SmokeData;
    SmokeData.EffectType = EVFX_EffectType::CampfireSmoke;
    SmokeData.Location = FireLocation + FVector(0, 0, 50);
    SmokeData.Duration = 60.0f;
    SmokeData.bAutoDestroy = false;
    
    UParticleSystemComponent* SmokeEffect = CreateParticleEffect(EVFX_EffectType::CampfireSmoke, SmokeData);
    if (SmokeEffect)
    {
        ActiveEffects.Add(SmokeEffect);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Campfire effects spawned at %s"), *FireLocation.ToString());
}

void UVFX_SystemManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Cleanup: %d active effects remaining"), ActiveEffects.Num());
}

UParticleSystemComponent* UVFX_SystemManager::CreateParticleEffect(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create particle system component
    UParticleSystemComponent* ParticleComp = NewObject<UParticleSystemComponent>(GetOwner());
    if (!ParticleComp)
    {
        return nullptr;
    }
    
    // Set transform
    ParticleComp->SetWorldLocation(EffectData.Location);
    ParticleComp->SetWorldRotation(EffectData.Rotation);
    ParticleComp->SetWorldScale3D(FVector(EffectData.Scale));
    
    // Configure based on effect type
    switch (EffectType)
    {
        case EVFX_EffectType::DinosaurFootstep:
            // Configure dust/dirt particle system
            ParticleComp->SetFloatParameter(TEXT("SpawnRate"), 100.0f * EffectData.Scale);
            ParticleComp->SetFloatParameter(TEXT("Lifetime"), 2.0f);
            break;
            
        case EVFX_EffectType::WaterSplash:
            // Configure water splash particle system
            ParticleComp->SetFloatParameter(TEXT("SpawnRate"), 200.0f * EffectData.Scale);
            ParticleComp->SetFloatParameter(TEXT("Lifetime"), 1.5f);
            break;
            
        case EVFX_EffectType::BloodSpray:
            // Configure blood particle system
            ParticleComp->SetFloatParameter(TEXT("SpawnRate"), 50.0f);
            ParticleComp->SetFloatParameter(TEXT("Lifetime"), 1.0f);
            break;
            
        case EVFX_EffectType::CampfireFire:
            // Configure fire particle system
            ParticleComp->SetFloatParameter(TEXT("SpawnRate"), 30.0f);
            ParticleComp->SetFloatParameter(TEXT("Lifetime"), 60.0f);
            break;
            
        case EVFX_EffectType::CampfireSmoke:
            // Configure smoke particle system
            ParticleComp->SetFloatParameter(TEXT("SpawnRate"), 20.0f);
            ParticleComp->SetFloatParameter(TEXT("Lifetime"), 60.0f);
            break;
            
        default:
            break;
    }
    
    // Attach to owner and activate
    ParticleComp->AttachToComponent(GetOwner()->GetRootComponent(), 
                                   FAttachmentTransformRules::KeepWorldTransform);
    ParticleComp->Activate();
    
    return ParticleComp;
}

void UVFX_SystemManager::UpdateEffectLOD(UParticleSystemComponent* Effect, float DistanceToPlayer)
{
    if (!Effect || !IsValid(Effect))
    {
        return;
    }
    
    // LOD based on distance
    if (DistanceToPlayer > LODDistanceFar)
    {
        // Far LOD - disable or reduce quality
        Effect->SetActive(false);
    }
    else if (DistanceToPlayer > LODDistanceNear)
    {
        // Medium LOD - reduce particle count
        Effect->SetActive(true);
        Effect->SetFloatParameter(TEXT("LODDistanceCheckTime"), 0.5f);
    }
    else
    {
        // Near LOD - full quality
        Effect->SetActive(true);
        Effect->SetFloatParameter(TEXT("LODDistanceCheckTime"), 0.1f);
    }
}

float UVFX_SystemManager::GetDistanceToPlayer(FVector EffectLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    return FVector::Dist(PlayerLocation, EffectLocation);
}