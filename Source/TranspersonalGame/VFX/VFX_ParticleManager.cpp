#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    InitializeParticleLibrary();
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: BeginPlay - System initialized"));
}

void AVFX_ParticleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateActiveEffects(DeltaTime);
}

void AVFX_ParticleManager::InitializeParticleLibrary()
{
    // Initialize default particle effects
    FVFX_ParticleEffect FireEffect;
    FireEffect.DefaultScale = FVector(1.0f, 1.0f, 1.5f);
    FireEffect.DefaultLifetime = 10.0f;
    FireEffect.bAutoDestroy = false;
    ParticleLibrary.Add(EVFX_EffectType::Fire, FireEffect);

    FVFX_ParticleEffect DustEffect;
    DustEffect.DefaultScale = FVector(2.0f, 2.0f, 1.0f);
    DustEffect.DefaultLifetime = 3.0f;
    DustEffect.bAutoDestroy = true;
    ParticleLibrary.Add(EVFX_EffectType::Dust, DustEffect);

    FVFX_ParticleEffect BloodEffect;
    BloodEffect.DefaultScale = FVector(0.8f, 0.8f, 0.8f);
    BloodEffect.DefaultLifetime = 2.0f;
    BloodEffect.bAutoDestroy = true;
    ParticleLibrary.Add(EVFX_EffectType::Blood, BloodEffect);

    FVFX_ParticleEffect WaterEffect;
    WaterEffect.DefaultScale = FVector(1.5f, 1.5f, 1.0f);
    WaterEffect.DefaultLifetime = 8.0f;
    WaterEffect.bAutoDestroy = false;
    ParticleLibrary.Add(EVFX_EffectType::Water, WaterEffect);

    FVFX_ParticleEffect SmokeEffect;
    SmokeEffect.DefaultScale = FVector(2.0f, 2.0f, 3.0f);
    SmokeEffect.DefaultLifetime = 15.0f;
    SmokeEffect.bAutoDestroy = false;
    ParticleLibrary.Add(EVFX_EffectType::Smoke, SmokeEffect);

    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Particle library initialized with %d effects"), ParticleLibrary.Num());
}

UNiagaraComponent* AVFX_ParticleManager::SpawnParticleEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!ParticleLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Effect type not found in library"));
        return nullptr;
    }

    const FVFX_ParticleEffect& EffectData = ParticleLibrary[EffectType];
    
    if (!EffectData.ParticleSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Particle system not loaded for effect type"));
        return nullptr;
    }

    UNiagaraComponent* NewEffect = NewObject<UNiagaraComponent>(this);
    if (NewEffect)
    {
        NewEffect->SetAsset(EffectData.ParticleSystem.Get());
        NewEffect->SetWorldLocation(Location);
        NewEffect->SetWorldRotation(Rotation);
        NewEffect->SetWorldScale3D(Scale * EffectData.DefaultScale);
        
        NewEffect->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        NewEffect->Activate();

        ActiveEffects.Add(NewEffect);

        if (EffectData.bAutoDestroy && EffectData.DefaultLifetime > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewEffect]()
            {
                StopParticleEffect(NewEffect);
            }, EffectData.DefaultLifetime, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned effect at location %s"), *Location.ToString());
        return NewEffect;
    }

    return nullptr;
}

void AVFX_ParticleManager::StopParticleEffect(UNiagaraComponent* Effect)
{
    if (Effect && ActiveEffects.Contains(Effect))
    {
        Effect->Deactivate();
        ActiveEffects.Remove(Effect);
        Effect->DestroyComponent();
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Stopped and removed particle effect"));
    }
}

void AVFX_ParticleManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Stopped all active effects"));
}

void AVFX_ParticleManager::RegisterParticleSystem(EVFX_EffectType EffectType, UNiagaraSystem* ParticleSystem)
{
    if (ParticleSystem && ParticleLibrary.Contains(EffectType))
    {
        ParticleLibrary[EffectType].ParticleSystem = ParticleSystem;
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Registered particle system for effect type"));
    }
}

int32 AVFX_ParticleManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_ParticleManager::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !Effect->IsActive();
    });
}

void AVFX_ParticleManager::UpdateActiveEffects(float DeltaTime)
{
    // Clean up finished effects every few seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    
    if (CleanupTimer >= 5.0f)
    {
        CleanupFinishedEffects();
        CleanupTimer = 0.0f;
    }
}