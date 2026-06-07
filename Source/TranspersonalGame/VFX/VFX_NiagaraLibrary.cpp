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
    InitializeDefaultEffects();
    
    // Setup cleanup timer for finished effects
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UVFX_NiagaraLibrary::CleanupFinishedEffects,
            2.0f,
            true
        );
    }
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Effect type not found in library"));
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Niagara system is null for effect type"));
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem.Get(),
        Location,
        Rotation,
        EffectData.Scale,
        EffectData.bAutoDestroy
    );

    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        
        // Set duration if specified
        if (EffectData.Duration > 0.0f && EffectData.bAutoDestroy)
        {
            EffectComponent->SetFloatParameter(TEXT("LifeTime"), EffectData.Duration);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned effect at location %s"), *Location.ToString());
    }

    return EffectComponent;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        ActiveEffects.Remove(EffectComponent);
    }
}

void UVFX_NiagaraLibrary::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    ActiveEffects.Empty();
}

void UVFX_NiagaraLibrary::RegisterEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem, FVector Scale, float Duration)
{
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Cannot register null Niagara system"));
        return;
    }

    FVFX_EffectData EffectData;
    EffectData.NiagaraSystem = NiagaraSystem;
    EffectData.Scale = Scale;
    EffectData.Duration = Duration;
    EffectData.bAutoDestroy = Duration > 0.0f;

    EffectLibrary.Add(EffectType, EffectData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Registered effect type %d"), (int32)EffectType);
}

bool UVFX_NiagaraLibrary::HasEffect(EVFX_EffectType EffectType) const
{
    return EffectLibrary.Contains(EffectType);
}

int32 UVFX_NiagaraLibrary::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_NiagaraLibrary::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect) {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}

void UVFX_NiagaraLibrary::InitializeDefaultEffects()
{
    // Initialize default effect data structures
    // Note: Actual Niagara systems will be assigned via Blueprint or C++ setup
    
    FVFX_EffectData CampfireData;
    CampfireData.Scale = FVector(1.0f, 1.0f, 1.5f);
    CampfireData.Duration = -1.0f; // Persistent
    CampfireData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.Scale = FVector(0.5f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    FVFX_EffectData DinoStompData;
    DinoStompData.Scale = FVector(2.0f);
    DinoStompData.Duration = 3.0f;
    DinoStompData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Dust_DinoStomp, DinoStompData);

    FVFX_EffectData BloodData;
    BloodData.Scale = FVector(1.0f);
    BloodData.Duration = 5.0f;
    BloodData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Blood_Impact, BloodData);

    FVFX_EffectData RainData;
    RainData.Scale = FVector(10.0f, 10.0f, 1.0f);
    RainData.Duration = -1.0f; // Persistent weather
    RainData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Rain_Heavy, RainData);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized %d default effects"), EffectLibrary.Num());
}