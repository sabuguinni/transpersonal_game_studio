#include "VFX_EffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    GlobalEffectScale = 1.0f;
    MaxActiveEffects = 50;
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager initialized with %d effect types"), EffectDatabase.Num());
}

void AVFX_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup expired effects every 2 seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 2.0f)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

void AVFX_EffectManager::InitializeEffectDatabase()
{
    // Initialize effect database with default values
    // In a real implementation, these would load from data assets
    
    FVFX_EffectData FootstepData;
    FootstepData.Scale = FVector(1.0f, 1.0f, 0.5f);
    FootstepData.Duration = 1.5f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::DinosaurFootstep, FootstepData);

    FVFX_EffectData BloodData;
    BloodData.Scale = FVector(0.8f, 0.8f, 0.8f);
    BloodData.Duration = 3.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::BloodImpact, BloodData);

    FVFX_EffectData FireData;
    FireData.Scale = FVector(1.2f, 1.2f, 1.5f);
    FireData.Duration = -1.0f; // Persistent
    FireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::CampfireFire, FireData);

    FVFX_EffectData RainData;
    RainData.Scale = FVector(5.0f, 5.0f, 3.0f);
    RainData.Duration = -1.0f; // Persistent weather
    RainData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::WeatherRain, RainData);

    FVFX_EffectData DustData;
    DustData.Scale = FVector(1.5f, 1.5f, 1.0f);
    DustData.Duration = 2.5f;
    DustData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::EnvironmentDust, DustData);
}

UNiagaraComponent* AVFX_EffectManager::SpawnEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation, float ScaleMultiplier)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Maximum active effects reached (%d)"), MaxActiveEffects);
        CleanupExpiredEffects();
        return nullptr;
    }

    const FVFX_EffectData* EffectData = EffectDatabase.Find(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_EffectManager: Effect type %d not found in database"), (int32)EffectType);
        return nullptr;
    }

    // For now, create a basic Niagara component without loading specific assets
    // In production, this would load the actual Niagara system from EffectData->NiagaraSystem
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Would be EffectData->NiagaraSystem.LoadSynchronous() in production
        Location,
        Rotation,
        CalculateEffectScale(EffectType, ScaleMultiplier),
        EffectData->bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        
        // Set auto-destroy timer if specified
        if (EffectData->bAutoDestroy && EffectData->Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                    ActiveEffects.RemoveSingle(NiagaraComp);
                }
            }, EffectData->Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Spawned effect %d at %s"), (int32)EffectType, *Location.ToString());
    }

    return NiagaraComp;
}

void AVFX_EffectManager::SpawnFootstepEffect(const FVector& Location, float CreatureSize, ESurfaceType SurfaceType)
{
    // Scale effect based on creature size
    float ScaleMultiplier = FMath::Clamp(CreatureSize, 0.1f, 10.0f);
    
    // Adjust effect based on surface type
    EVFX_EffectType EffectType = EVFX_EffectType::DinosaurFootstep;
    
    switch (SurfaceType)
    {
        case ESurfaceType::Dirt:
        case ESurfaceType::Sand:
            EffectType = EVFX_EffectType::EnvironmentDust;
            break;
        case ESurfaceType::Water:
            EffectType = EVFX_EffectType::WaterSplash;
            ScaleMultiplier *= 0.8f;
            break;
        default:
            EffectType = EVFX_EffectType::DinosaurFootstep;
            break;
    }

    SpawnEffect(EffectType, Location, FRotator::ZeroRotator, ScaleMultiplier);
}

void AVFX_EffectManager::SpawnBloodEffect(const FVector& Location, const FVector& ImpactNormal, float Intensity)
{
    FRotator EffectRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    float ScaleMultiplier = FMath::Clamp(Intensity, 0.1f, 3.0f);
    
    SpawnEffect(EVFX_EffectType::BloodImpact, Location, EffectRotation, ScaleMultiplier);
}

void AVFX_EffectManager::SpawnEnvironmentalEffect(EVFX_EffectType EffectType, const FVector& Location, float Duration)
{
    UNiagaraComponent* Effect = SpawnEffect(EffectType, Location);
    
    if (Effect && Duration > 0.0f)
    {
        // Override the default duration
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Effect]()
        {
            if (IsValid(Effect))
            {
                Effect->DestroyComponent();
                ActiveEffects.RemoveSingle(Effect);
            }
        }, Duration, false);
    }
}

void AVFX_EffectManager::CleanupExpiredEffects()
{
    RemoveNullEffects();
    
    int32 RemovedCount = 0;
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]) || !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
            RemovedCount++;
        }
    }

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_EffectManager: Cleaned up %d expired effects. Active: %d"), RemovedCount, ActiveEffects.Num());
    }
}

void AVFX_EffectManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VFX_EffectManager: Stopped all effects"));
}

void AVFX_EffectManager::RemoveNullEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect);
    });
}

FVector AVFX_EffectManager::CalculateEffectScale(EVFX_EffectType EffectType, float ScaleMultiplier) const
{
    const FVFX_EffectData* EffectData = EffectDatabase.Find(EffectType);
    if (!EffectData)
    {
        return FVector(GlobalEffectScale * ScaleMultiplier);
    }

    return EffectData->Scale * GlobalEffectScale * ScaleMultiplier;
}