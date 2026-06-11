#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    LoadEffectData();
    InitializeDefaultEffects();
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Effect type not found in library"));
        return nullptr;
    }

    FVFX_EffectData* EffectData = EffectLibrary.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Invalid effect data"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        EffectData->NiagaraSystem,
        Location,
        Rotation,
        EffectData->Scale,
        EffectData->bAutoDestroy
    );

    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        
        if (EffectData->Duration > 0.0f && EffectData->bAutoDestroy)
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
            {
                StopEffect(NewEffect);
            }, EffectData->Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned effect at location %s"), *Location.ToString());
    }

    return NewEffect;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (!EffectComponent)
    {
        return;
    }

    EffectComponent->Deactivate();
    ActiveEffects.Remove(EffectComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped effect component"));
}

void UVFX_NiagaraLibrary::LoadEffectData()
{
    // Load Niagara systems from content browser
    // This would typically load from data assets or configuration files
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Loading effect data"));
}

void UVFX_NiagaraLibrary::InitializeDefaultEffects()
{
    // Initialize default effect configurations
    FVFX_EffectData FireEffect;
    FireEffect.Scale = FVector(1.0f, 1.0f, 1.0f);
    FireEffect.Duration = 0.0f; // Continuous
    FireEffect.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Fire, FireEffect);

    FVFX_EffectData DustEffect;
    DustEffect.Scale = FVector(0.8f, 0.8f, 0.8f);
    DustEffect.Duration = 3.0f;
    DustEffect.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Dust, DustEffect);

    FVFX_EffectData BloodEffect;
    BloodEffect.Scale = FVector(0.5f, 0.5f, 0.5f);
    BloodEffect.Duration = 2.0f;
    BloodEffect.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Blood, BloodEffect);

    FVFX_EffectData WaterEffect;
    WaterEffect.Scale = FVector(1.2f, 1.2f, 1.2f);
    WaterEffect.Duration = 0.0f; // Continuous
    WaterEffect.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Water, WaterEffect);

    FVFX_EffectData WeatherEffect;
    WeatherEffect.Scale = FVector(5.0f, 5.0f, 5.0f);
    WeatherEffect.Duration = 0.0f; // Continuous
    WeatherEffect.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Weather, WeatherEffect);

    FVFX_EffectData ImpactEffect;
    ImpactEffect.Scale = FVector(0.7f, 0.7f, 0.7f);
    ImpactEffect.Duration = 1.5f;
    ImpactEffect.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Impact, ImpactEffect);

    FVFX_EffectData BreathEffect;
    BreathEffect.Scale = FVector(0.6f, 0.6f, 0.6f);
    BreathEffect.Duration = 2.0f;
    BreathEffect.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Breath, BreathEffect);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized %d default effects"), EffectLibrary.Num());
}