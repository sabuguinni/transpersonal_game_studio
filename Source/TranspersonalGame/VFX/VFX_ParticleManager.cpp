#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bUseNiagaraSystem = true;
    GlobalEffectScale = 1.0f;
    MaxActiveEffects = 50;
    
    // Initialize default effect database
    FVFX_EffectData CampfireEffect;
    CampfireEffect.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireEffect.Duration = -1.0f; // Looping
    CampfireEffect.Scale = FVector(1.0f, 1.0f, 1.0f);
    EffectDatabase.Add(CampfireEffect);
    
    FVFX_EffectData FootstepEffect;
    FootstepEffect.EffectType = EVFX_EffectType::Dino_Footstep;
    FootstepEffect.Duration = 2.0f;
    FootstepEffect.Scale = FVector(1.5f, 1.5f, 1.0f);
    EffectDatabase.Add(FootstepEffect);
    
    FVFX_EffectData RainEffect;
    RainEffect.EffectType = EVFX_EffectType::Weather_Rain;
    RainEffect.Duration = -1.0f; // Looping
    RainEffect.Scale = FVector(10.0f, 10.0f, 5.0f);
    EffectDatabase.Add(RainEffect);
    
    FVFX_EffectData BloodEffect;
    BloodEffect.EffectType = EVFX_EffectType::Combat_Blood;
    BloodEffect.Duration = 3.0f;
    BloodEffect.Scale = FVector(0.8f, 0.8f, 0.8f);
    EffectDatabase.Add(BloodEffect);
    
    FVFX_EffectData DustEffect;
    DustEffect.EffectType = EVFX_EffectType::Environment_Dust;
    DustEffect.Duration = 4.0f;
    DustEffect.Scale = FVector(2.0f, 2.0f, 1.0f);
    EffectDatabase.Add(DustEffect);
    
    FVFX_EffectData SplashEffect;
    SplashEffect.EffectType = EVFX_EffectType::Water_Splash;
    SplashEffect.Duration = 1.5f;
    SplashEffect.Scale = FVector(1.2f, 1.2f, 1.2f);
    EffectDatabase.Add(SplashEffect);
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX ParticleManager initialized with %d effect types"), EffectDatabase.Num());
}

void UVFX_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupFinishedEffects();
}

void UVFX_ParticleManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    if (ActiveNiagaraComponents.Num() + ActiveParticleComponents.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX ParticleManager: Max active effects reached (%d)"), MaxActiveEffects);
        return;
    }
    
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX ParticleManager: Effect type not found"));
        return;
    }
    
    float FinalScale = Scale * GlobalEffectScale;
    
    if (bUseNiagaraSystem && EffectData->NiagaraSystem.IsValid())
    {
        SpawnNiagaraEffect(*EffectData, Location, Rotation, FinalScale);
    }
    else if (EffectData->LegacyParticleSystem.IsValid())
    {
        SpawnLegacyEffect(*EffectData, Location, Rotation, FinalScale);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX ParticleManager: No valid particle system found for effect type"));
    }
}

void UVFX_ParticleManager::SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX ParticleManager: Target actor is null"));
        return;
    }
    
    FVector SpawnLocation = TargetActor->GetActorLocation() + Offset;
    FRotator SpawnRotation = TargetActor->GetActorRotation();
    
    SpawnEffect(EffectType, SpawnLocation, SpawnRotation);
}

void UVFX_ParticleManager::StopAllEffects()
{
    for (UNiagaraComponent* NiagaraComp : ActiveNiagaraComponents)
    {
        if (IsValid(NiagaraComp))
        {
            NiagaraComp->Deactivate();
        }
    }
    
    for (UParticleSystemComponent* ParticleComp : ActiveParticleComponents)
    {
        if (IsValid(ParticleComp))
        {
            ParticleComp->Deactivate();
        }
    }
    
    ActiveNiagaraComponents.Empty();
    ActiveParticleComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX ParticleManager: All effects stopped"));
}

void UVFX_ParticleManager::SetEffectQuality(int32 QualityLevel)
{
    // Quality levels: 0=Low, 1=Medium, 2=High, 3=Ultra
    switch (QualityLevel)
    {
        case 0: // Low
            GlobalEffectScale = 0.5f;
            MaxActiveEffects = 20;
            bUseNiagaraSystem = false;
            break;
        case 1: // Medium
            GlobalEffectScale = 0.75f;
            MaxActiveEffects = 35;
            bUseNiagaraSystem = true;
            break;
        case 2: // High
            GlobalEffectScale = 1.0f;
            MaxActiveEffects = 50;
            bUseNiagaraSystem = true;
            break;
        case 3: // Ultra
            GlobalEffectScale = 1.5f;
            MaxActiveEffects = 75;
            bUseNiagaraSystem = true;
            break;
        default:
            QualityLevel = 2; // Default to High
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX ParticleManager: Quality set to %d, Scale: %f, MaxEffects: %d"), 
           QualityLevel, GlobalEffectScale, MaxActiveEffects);
}

void UVFX_ParticleManager::CleanupFinishedEffects()
{
    // Clean up finished Niagara components
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* NiagaraComp = ActiveNiagaraComponents[i];
        if (!IsValid(NiagaraComp) || !NiagaraComp->IsActive())
        {
            ActiveNiagaraComponents.RemoveAt(i);
        }
    }
    
    // Clean up finished particle components
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* ParticleComp = ActiveParticleComponents[i];
        if (!IsValid(ParticleComp) || !ParticleComp->IsActive())
        {
            ActiveParticleComponents.RemoveAt(i);
        }
    }
}

FVFX_EffectData* UVFX_ParticleManager::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& Effect : EffectDatabase)
    {
        if (Effect.EffectType == EffectType)
        {
            return &Effect;
        }
    }
    return nullptr;
}

void UVFX_ParticleManager::SpawnNiagaraEffect(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, float Scale)
{
    if (!EffectData.NiagaraSystem.IsValid())
    {
        return;
    }
    
    UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        return;
    }
    
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        EffectData.Scale * Scale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (NiagaraComp)
    {
        ActiveNiagaraComponents.Add(NiagaraComp);
        
        if (EffectData.Duration > 0.0f)
        {
            // Set auto-destroy timer for non-looping effects
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->Deactivate();
                }
            }, EffectData.Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX ParticleManager: Spawned Niagara effect at %s"), *Location.ToString());
    }
}

void UVFX_ParticleManager::SpawnLegacyEffect(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation, float Scale)
{
    if (!EffectData.LegacyParticleSystem.IsValid())
    {
        return;
    }
    
    UParticleSystem* ParticleSystem = EffectData.LegacyParticleSystem.LoadSynchronous();
    if (!ParticleSystem)
    {
        return;
    }
    
    UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        ParticleSystem,
        Location,
        Rotation,
        EffectData.Scale * Scale,
        true,
        EPSCPoolMethod::None,
        true
    );
    
    if (ParticleComp)
    {
        ActiveParticleComponents.Add(ParticleComp);
        UE_LOG(LogTemp, Log, TEXT("VFX ParticleManager: Spawned legacy particle effect at %s"), *Location.ToString());
    }
}