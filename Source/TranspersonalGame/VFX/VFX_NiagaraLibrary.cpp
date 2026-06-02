#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectLibrary();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupFinishedEffects();
}

UNiagaraComponent* UVFX_NiagaraLibrary::PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (EffectType == EVFX_EffectType::None)
    {
        return nullptr;
    }

    FVFX_EffectData* EffectData = EffectLibrary.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found or invalid: %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraSystem* System = EffectData->NiagaraSystem.LoadSynchronous();
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara system for effect: %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        Scale * EffectData->Scale,
        EffectData->bAutoDestroy
    );

    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned: %d at location %s"), (int32)EffectType, *Location.ToString());
    }

    return EffectComponent;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->DeactivateImmediate();
        ActiveEffects.Remove(EffectComponent);
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
}

void UVFX_NiagaraLibrary::InitializeEffectLibrary()
{
    LoadDefaultEffects();
    UE_LOG(LogTemp, Log, TEXT("VFX Library initialized with %d effects"), EffectLibrary.Num());
}

bool UVFX_NiagaraLibrary::IsEffectActive(EVFX_EffectType EffectType) const
{
    for (const UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect) && Effect->IsActive())
        {
            return true;
        }
    }
    return false;
}

int32 UVFX_NiagaraLibrary::GetActiveEffectCount() const
{
    int32 Count = 0;
    for (const UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect) && Effect->IsActive())
        {
            Count++;
        }
    }
    return Count;
}

void UVFX_NiagaraLibrary::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || !Effect->IsActive();
    });
}

void UVFX_NiagaraLibrary::LoadDefaultEffects()
{
    // Initialize default effect data
    FVFX_EffectData CampfireData;
    CampfireData.Scale = FVector(1.5f);
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.Scale = FVector(2.0f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Dust_FootstepImpact, FootstepData);

    FVFX_EffectData BloodData;
    BloodData.Scale = FVector(1.0f);
    BloodData.Duration = 5.0f;
    BloodData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Blood_Splatter, BloodData);

    FVFX_EffectData RainData;
    RainData.Scale = FVector(10.0f);
    RainData.Duration = -1.0f; // Infinite
    RainData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Weather_Rain, RainData);

    FVFX_EffectData AshData;
    AshData.Scale = FVector(5.0f);
    AshData.Duration = -1.0f; // Infinite
    AshData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Volcanic_Ash, AshData);
}