#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectDatabase();
}

void UVFX_NiagaraLibrary::InitializeEffectDatabase()
{
    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Scale = FVector(1.5f, 1.5f, 2.0f);
    CampfireData.Duration = -1.0f; // Persistent
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    // Initialize footstep dust effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Scale = FVector(0.8f, 0.8f, 0.5f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    // Initialize blood impact effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood_Impact;
    BloodData.Scale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 3.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Blood_Impact, BloodData);

    // Initialize water splash effect
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Splash;
    WaterData.Scale = FVector(1.2f, 1.2f, 1.0f);
    WaterData.Duration = 2.5f;
    WaterData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Water_Splash, WaterData);

    // Initialize cooking smoke effect
    FVFX_EffectData SmokeData;
    SmokeData.EffectType = EVFX_EffectType::Smoke_Cooking;
    SmokeData.Scale = FVector(1.0f, 1.0f, 1.5f);
    SmokeData.Duration = -1.0f; // Persistent while cooking
    SmokeData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Smoke_Cooking, SmokeData);

    // Initialize rain effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Scale = FVector(10.0f, 10.0f, 5.0f);
    RainData.Duration = -1.0f; // Weather controlled
    RainData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Weather_Rain, RainData);

    // Initialize wind particles effect
    FVFX_EffectData WindData;
    WindData.EffectType = EVFX_EffectType::Wind_Particles;
    WindData.Scale = FVector(5.0f, 5.0f, 2.0f);
    WindData.Duration = -1.0f; // Environmental
    WindData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Wind_Particles, WindData);

    // Initialize crafting sparks effect
    FVFX_EffectData CraftData;
    CraftData.EffectType = EVFX_EffectType::Craft_Sparks;
    CraftData.Scale = FVector(0.5f, 0.5f, 0.5f);
    CraftData.Duration = 1.5f;
    CraftData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Craft_Sparks, CraftData);

    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Initialized %d effect types"), EffectDatabase.Num());
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_NiagaraLibrary: Effect type %d not found in database"), (int32)EffectType);
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system assigned for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        EffectData.Scale,
        EffectData.bAutoDestroy
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        // Set up auto-destroy timer if needed
        if (EffectData.bAutoDestroy && EffectData.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
            {
                if (IsValid(NewEffect))
                {
                    NewEffect->DestroyComponent();
                    ActiveEffects.Remove(NewEffect);
                }
            }, EffectData.Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned effect %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }

    return NewEffect;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (IsValid(EffectComponent))
    {
        EffectComponent->DeactivateImmediate();
        ActiveEffects.Remove(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped effect component"));
    }
}

void UVFX_NiagaraLibrary::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DeactivateImmediate();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped all active effects"));
}

bool UVFX_NiagaraLibrary::IsEffectActive(EVFX_EffectType EffectType) const
{
    for (const UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect) && Effect->IsActive())
        {
            // Note: In a full implementation, you'd store effect type with the component
            return true;
        }
    }
    return false;
}

int32 UVFX_NiagaraLibrary::GetActiveEffectCount() const
{
    CleanupInactiveEffects();
    return ActiveEffects.Num();
}

void UVFX_NiagaraLibrary::SetEffectScale(EVFX_EffectType EffectType, FVector NewScale)
{
    if (EffectDatabase.Contains(EffectType))
    {
        EffectDatabase[EffectType].Scale = NewScale;
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Updated scale for effect type %d"), (int32)EffectType);
    }
}

void UVFX_NiagaraLibrary::CleanupInactiveEffects()
{
    ActiveEffects.RemoveAll([](const UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
}

void UVFX_NiagaraLibrary::OnEffectFinished(UNiagaraComponent* FinishedEffect)
{
    if (IsValid(FinishedEffect))
    {
        ActiveEffects.Remove(FinishedEffect);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Effect finished and removed from active list"));
    }
}