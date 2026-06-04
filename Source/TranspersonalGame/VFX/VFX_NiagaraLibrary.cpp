#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    SetupDefaultEffects();
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectLibrary();
}

void UVFX_NiagaraLibrary::SetupDefaultEffects()
{
    EffectLibrary.Empty();

    // Campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Scale = FVector(1.0f, 1.0f, 1.5f);
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    EffectLibrary.Add(CampfireData);

    // Dinosaur footstep impact
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Impact_Footstep;
    FootstepData.Scale = FVector(2.0f, 2.0f, 1.0f);
    FootstepData.Duration = 3.0f;
    FootstepData.bAutoDestroy = true;
    EffectLibrary.Add(FootstepData);

    // Rain weather
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Scale = FVector(10.0f, 10.0f, 5.0f);
    RainData.Duration = -1.0f; // Infinite
    RainData.bAutoDestroy = false;
    EffectLibrary.Add(RainData);

    // Blood splash combat
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Combat_BloodSplash;
    BloodData.Scale = FVector(0.8f, 0.8f, 0.8f);
    BloodData.Duration = 2.5f;
    BloodData.bAutoDestroy = true;
    EffectLibrary.Add(BloodData);

    // Crafting sparks
    FVFX_EffectData SparksData;
    SparksData.EffectType = EVFX_EffectType::Craft_Sparks;
    SparksData.Scale = FVector(0.5f, 0.5f, 0.5f);
    SparksData.Duration = 1.5f;
    SparksData.bAutoDestroy = true;
    EffectLibrary.Add(SparksData);

    // Environment dust
    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::Environment_Dust;
    DustData.Scale = FVector(1.5f, 1.5f, 1.0f);
    DustData.Duration = 4.0f;
    DustData.bAutoDestroy = true;
    EffectLibrary.Add(DustData);

    // Water splash
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Splash;
    WaterData.Scale = FVector(1.2f, 1.2f, 1.2f);
    WaterData.Duration = 2.0f;
    WaterData.bAutoDestroy = true;
    EffectLibrary.Add(WaterData);

    // Snow weather
    FVFX_EffectData SnowData;
    SnowData.EffectType = EVFX_EffectType::Weather_Snow;
    SnowData.Scale = FVector(8.0f, 8.0f, 3.0f);
    SnowData.Duration = -1.0f; // Infinite
    SnowData.bAutoDestroy = false;
    EffectLibrary.Add(SnowData);
}

void UVFX_NiagaraLibrary::InitializeEffectLibrary()
{
    // Initialize Niagara system references
    // In a real implementation, these would load actual Niagara assets
    for (FVFX_EffectData& EffectData : EffectLibrary)
    {
        // Placeholder - would load actual Niagara systems from content browser
        // EffectData.NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_Fire_Campfire"));
    }
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Effect type not found"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Create Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        EffectData->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        EffectData->Scale,
        EffectData->bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        
        // Set duration for auto-destroy effects
        if (EffectData->bAutoDestroy && EffectData->Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                    ActiveEffects.Remove(NiagaraComp);
                }
            }, EffectData->Duration, false);
        }
    }

    CleanupDestroyedEffects();
    return NiagaraComp;
}

void UVFX_NiagaraLibrary::SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset)
{
    if (!TargetActor)
    {
        return;
    }

    FVector SpawnLocation = TargetActor->GetActorLocation() + Offset;
    FRotator SpawnRotation = TargetActor->GetActorRotation();
    
    SpawnEffect(EffectType, SpawnLocation, SpawnRotation);
}

void UVFX_NiagaraLibrary::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
}

void UVFX_NiagaraLibrary::StopEffectsByType(EVFX_EffectType EffectType)
{
    // Note: This is a simplified implementation
    // In practice, you'd track effect types per component
    CleanupDestroyedEffects();
}

int32 UVFX_NiagaraLibrary::GetActiveEffectCount() const
{
    int32 ValidCount = 0;
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            ValidCount++;
        }
    }
    return ValidCount;
}

FVFX_EffectData* UVFX_NiagaraLibrary::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& EffectData : EffectLibrary)
    {
        if (EffectData.EffectType == EffectType)
        {
            return &EffectData;
        }
    }
    return nullptr;
}

void UVFX_NiagaraLibrary::CleanupDestroyedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect);
    });
}