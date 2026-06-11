#include "VFX_EffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize effect database
    InitializeEffectDatabase();
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start periodic cleanup timer
    GetWorldTimerManager().SetTimer(CleanupTimerHandle, this, &AVFX_EffectManager::PerformPeriodicCleanup, 5.0f, true);
}

void AVFX_EffectManager::InitializeEffectDatabase()
{
    // Initialize default effect data
    FVFX_EffectData CampfireData;
    CampfireData.Duration = 0.0f; // Looping
    CampfireData.Scale = 1.0f;
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.Duration = 3.0f;
    FootstepData.Scale = 1.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    FVFX_EffectData RainData;
    RainData.Duration = 0.0f; // Looping
    RainData.Scale = 1.0f;
    RainData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Weather_Rain, RainData);

    FVFX_EffectData BloodData;
    BloodData.Duration = 2.0f;
    BloodData.Scale = 1.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Combat_BloodImpact, BloodData);

    FVFX_EffectData SplashData;
    SplashData.Duration = 1.5f;
    SplashData.Scale = 1.0f;
    SplashData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Water_Splash, SplashData);

    FVFX_EffectData SparksData;
    SparksData.Duration = 2.0f;
    SparksData.Scale = 1.0f;
    SparksData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Sparks_Crafting, SparksData);

    FVFX_EffectData SmokeData;
    SmokeData.Duration = 0.0f; // Looping
    SmokeData.Scale = 1.0f;
    SmokeData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Smoke_Cooking, SmokeData);
}

UNiagaraComponent* AVFX_EffectManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float CustomScale)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Effect type not found in database"));
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    
    if (!EffectData.NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: No Niagara system assigned for effect type"));
        return nullptr;
    }

    // Spawn Niagara effect at world location
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem,
        Location,
        Rotation,
        FVector(EffectData.Scale * CustomScale),
        EffectData.bAutoDestroy
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Set custom duration if specified
        if (EffectData.Duration > 0.0f && EffectData.bAutoDestroy)
        {
            FTimerHandle DestroyTimer;
            GetWorldTimerManager().SetTimer(DestroyTimer, [this, NewEffect]()
            {
                CleanupEffect(NewEffect);
            }, EffectData.Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Spawned effect at location %s"), *Location.ToString());
    }

    return NewEffect;
}

UNiagaraComponent* AVFX_EffectManager::SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Target actor is null"));
        return nullptr;
    }

    FVector SpawnLocation = TargetActor->GetActorLocation() + Offset;
    FRotator SpawnRotation = TargetActor->GetActorRotation();

    return SpawnEffect(EffectType, SpawnLocation, SpawnRotation);
}

void AVFX_EffectManager::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (!EffectComponent)
        return;

    EffectComponent->Deactivate();
    CleanupEffect(EffectComponent);
}

void AVFX_EffectManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: All effects stopped"));
}

void AVFX_EffectManager::CreateCampfire(FVector Location)
{
    SpawnEffect(EVFX_EffectType::Fire_Campfire, Location);
}

void AVFX_EffectManager::CreateDinosaurFootstep(FVector Location, float DinosaurSize)
{
    SpawnEffect(EVFX_EffectType::Dust_Footstep, Location, FRotator::ZeroRotator, DinosaurSize);
}

void AVFX_EffectManager::CreateRainEffect(FVector Location, float Intensity)
{
    SpawnEffect(EVFX_EffectType::Weather_Rain, Location, FRotator::ZeroRotator, Intensity);
}

void AVFX_EffectManager::CreateBloodImpact(FVector Location, FVector ImpactDirection)
{
    FRotator ImpactRotation = ImpactDirection.Rotation();
    SpawnEffect(EVFX_EffectType::Combat_BloodImpact, Location, ImpactRotation);
}

void AVFX_EffectManager::CreateWaterSplash(FVector Location, float SplashSize)
{
    SpawnEffect(EVFX_EffectType::Water_Splash, Location, FRotator::ZeroRotator, SplashSize);
}

void AVFX_EffectManager::CreateCraftingSparks(FVector Location)
{
    SpawnEffect(EVFX_EffectType::Sparks_Crafting, Location);
}

void AVFX_EffectManager::CreateCookingSmoke(FVector Location)
{
    SpawnEffect(EVFX_EffectType::Smoke_Cooking, Location);
}

int32 AVFX_EffectManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_EffectManager::CleanupFinishedEffects()
{
    PerformPeriodicCleanup();
}

void AVFX_EffectManager::CleanupEffect(UNiagaraComponent* Effect)
{
    if (Effect)
    {
        ActiveEffects.Remove(Effect);
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
}

void AVFX_EffectManager::PerformPeriodicCleanup()
{
    // Remove null or invalid effects from tracking
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (!ActiveEffects[i] || !IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}