#include "VFXNiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultEffects();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVFX_NiagaraLibrary::InitializeDefaultEffects()
{
    // Initialize default effect paths - these would be set up in Blueprint or config
    FVFX_EffectData CampfireData;
    CampfireData.DefaultScale = FVector(1.0f, 1.0f, 1.2f);
    CampfireData.DefaultLifetime = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.DefaultScale = FVector(0.8f, 0.8f, 0.5f);
    FootstepData.DefaultLifetime = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Dino_Footstep, FootstepData);

    FVFX_EffectData RainData;
    RainData.DefaultScale = FVector(5.0f, 5.0f, 3.0f);
    RainData.DefaultLifetime = -1.0f; // Infinite
    RainData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::Weather_Rain, RainData);

    FVFX_EffectData BloodData;
    BloodData.DefaultScale = FVector(0.6f, 0.6f, 0.6f);
    BloodData.DefaultLifetime = 3.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Blood_Impact, BloodData);

    FVFX_EffectData DustData;
    DustData.DefaultScale = FVector(1.2f, 1.2f, 0.8f);
    DustData.DefaultLifetime = 4.0f;
    DustData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::Dust_Cloud, DustData);
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not registered: %d"), (int32)EffectType);
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    FVector FinalScale = Scale * EffectData.DefaultScale;
    
    return CreateNiagaraComponent(EffectData, Location, Rotation, FinalScale);
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName, FVector LocationOffset, FRotator RotationOffset, FVector Scale)
{
    if (!EffectDatabase.Contains(EffectType) || !AttachToComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not registered or invalid attach component: %d"), (int32)EffectType);
        return nullptr;
    }

    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    FVector FinalScale = Scale * EffectData.DefaultScale;
    
    UNiagaraComponent* NiagaraComp = CreateNiagaraComponent(EffectData, LocationOffset, RotationOffset, FinalScale);
    
    if (NiagaraComp && AttachToComponent)
    {
        NiagaraComp->AttachToComponent(AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachPointName);
    }
    
    return NiagaraComp;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent)
    {
        EffectComponent->Deactivate();
        
        // Auto-destroy after a short delay to allow particles to fade out
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [EffectComponent]()
        {
            if (IsValid(EffectComponent))
            {
                EffectComponent->DestroyComponent();
            }
        }, 2.0f, false);
    }
}

void UVFX_NiagaraLibrary::RegisterEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData)
{
    EffectDatabase.Add(EffectType, EffectData);
    UE_LOG(LogTemp, Log, TEXT("VFX Effect registered: %d"), (int32)EffectType);
}

bool UVFX_NiagaraLibrary::IsEffectRegistered(EVFX_EffectType EffectType) const
{
    return EffectDatabase.Contains(EffectType);
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateNiagaraComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Niagara system is not valid"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        EffectData.NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Scale,
        EffectData.bAutoDestroy
    );

    if (NiagaraComp && EffectData.DefaultLifetime > 0.0f)
    {
        FTimerHandle LifetimeTimer;
        World->GetTimerManager().SetTimer(LifetimeTimer, [NiagaraComp]()
        {
            if (IsValid(NiagaraComp))
            {
                NiagaraComp->Deactivate();
            }
        }, EffectData.DefaultLifetime, false);
    }

    return NiagaraComp;
}