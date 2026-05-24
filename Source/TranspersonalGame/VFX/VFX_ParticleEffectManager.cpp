#include "VFX_ParticleEffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ParticleEffectManager::UVFX_ParticleEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    EffectCleanupTimer = 0.0f;
}

void UVFX_ParticleEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleEffectManager: System initialized with %d effect types"), EffectDatabase.Num());
}

void UVFX_ParticleEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    EffectCleanupTimer += DeltaTime;
    if (EffectCleanupTimer >= CLEANUP_INTERVAL)
    {
        CleanupFinishedEffects();
        EffectCleanupTimer = 0.0f;
    }
}

void UVFX_ParticleEffectManager::InitializeDefaultEffects()
{
    // Initialize footstep impact effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::FootstepImpact;
    FootstepData.Duration = 1.5f;
    FootstepData.Scale = 1.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::FootstepImpact, FootstepData);
    
    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::CampfireFlames;
    CampfireData.Duration = 0.0f; // Continuous
    CampfireData.Scale = 1.0f;
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(EVFX_EffectType::CampfireFlames, CampfireData);
    
    // Initialize blood splatter effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodSplatter;
    BloodData.Duration = 2.0f;
    BloodData.Scale = 1.0f;
    BloodData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::BloodSplatter, BloodData);
    
    // Initialize dust cloud effect
    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::DustCloud;
    DustData.Duration = 3.0f;
    DustData.Scale = 1.0f;
    DustData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::DustCloud, DustData);
    
    // Initialize weapon impact effect
    FVFX_EffectData WeaponData;
    WeaponData.EffectType = EVFX_EffectType::WeaponImpact;
    WeaponData.Duration = 1.0f;
    WeaponData.Scale = 1.0f;
    WeaponData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::WeaponImpact, WeaponData);
    
    // Initialize breath vapor effect
    FVFX_EffectData BreathData;
    BreathData.EffectType = EVFX_EffectType::BreathVapor;
    BreathData.Duration = 2.5f;
    BreathData.Scale = 1.0f;
    BreathData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::BreathVapor, BreathData);
    
    // Initialize water splash effect
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::WaterSplash;
    WaterData.Duration = 1.8f;
    WaterData.Scale = 1.0f;
    WaterData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::WaterSplash, WaterData);
    
    // Initialize rock debris effect
    FVFX_EffectData RockData;
    RockData.EffectType = EVFX_EffectType::RockDebris;
    RockData.Duration = 2.2f;
    RockData.Scale = 1.0f;
    RockData.bAutoDestroy = true;
    EffectDatabase.Add(EVFX_EffectType::RockDebris, RockData);
}

UNiagaraComponent* UVFX_ParticleEffectManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float CustomScale)
{
    if (ActiveEffects.Num() >= MAX_ACTIVE_EFFECTS)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleEffectManager: Max active effects reached (%d), skipping spawn"), MAX_ACTIVE_EFFECTS);
        return nullptr;
    }
    
    const FVFX_EffectData* EffectData = EffectDatabase.Find(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleEffectManager: Effect type %d not found in database"), (int32)EffectType);
        return nullptr;
    }
    
    UNiagaraComponent* NiagaraComp = nullptr;
    
    if (EffectData->NiagaraSystem.IsValid())
    {
        UNiagaraSystem* System = EffectData->NiagaraSystem.LoadSynchronous();
        if (System)
        {
            NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                System,
                Location,
                Rotation,
                FVector(EffectData->Scale * CustomScale),
                EffectData->bAutoDestroy
            );
            
            if (NiagaraComp)
            {
                ActiveEffects.Add(NiagaraComp);
                UE_LOG(LogTemp, Log, TEXT("VFX_ParticleEffectManager: Spawned effect %d at location %s"), (int32)EffectType, *Location.ToString());
            }
        }
    }
    
    // Play associated audio
    PlayEffectAudio(*EffectData, Location);
    
    return NiagaraComp;
}

void UVFX_ParticleEffectManager::SpawnEffectAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    SpawnEffect(EffectType, Location, Rotation);
}

void UVFX_ParticleEffectManager::SpawnEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachComponent, FName AttachPointName)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleEffectManager: Cannot attach effect - AttachComponent is null"));
        return;
    }
    
    const FVFX_EffectData* EffectData = EffectDatabase.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        return;
    }
    
    UNiagaraSystem* System = EffectData->NiagaraSystem.LoadSynchronous();
    if (System)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            System,
            AttachComponent,
            AttachPointName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            FVector(EffectData->Scale),
            EAttachLocation::KeepRelativeOffset,
            EffectData->bAutoDestroy
        );
        
        if (NiagaraComp)
        {
            ActiveEffects.Add(NiagaraComp);
        }
    }
}

void UVFX_ParticleEffectManager::SpawnFootstepEffect(FVector FootLocation, float DinosaurSize)
{
    SpawnEffect(EVFX_EffectType::FootstepImpact, FootLocation, FRotator::ZeroRotator, DinosaurSize);
    
    // Add dust cloud for larger dinosaurs
    if (DinosaurSize > 2.0f)
    {
        FVector DustLocation = FootLocation + FVector(0, 0, 50);
        SpawnEffect(EVFX_EffectType::DustCloud, DustLocation, FRotator::ZeroRotator, DinosaurSize * 0.5f);
    }
}

void UVFX_ParticleEffectManager::SpawnBreathVaporEffect(FVector MouthLocation, FRotator BreathDirection)
{
    SpawnEffect(EVFX_EffectType::BreathVapor, MouthLocation, BreathDirection);
}

void UVFX_ParticleEffectManager::SpawnBloodEffect(FVector ImpactLocation, FVector ImpactNormal)
{
    FRotator BloodRotation = ImpactNormal.Rotation();
    SpawnEffect(EVFX_EffectType::BloodSplatter, ImpactLocation, BloodRotation);
}

void UVFX_ParticleEffectManager::SpawnCampfireEffect(FVector FireLocation)
{
    SpawnEffect(EVFX_EffectType::CampfireFlames, FireLocation);
}

void UVFX_ParticleEffectManager::SpawnDustCloudEffect(FVector Location, float Intensity)
{
    SpawnEffect(EVFX_EffectType::DustCloud, Location, FRotator::ZeroRotator, Intensity);
}

void UVFX_ParticleEffectManager::SpawnWaterSplashEffect(FVector WaterLocation, float SplashSize)
{
    SpawnEffect(EVFX_EffectType::WaterSplash, WaterLocation, FRotator::ZeroRotator, SplashSize);
}

void UVFX_ParticleEffectManager::SpawnWeaponImpactEffect(FVector ImpactLocation, FVector ImpactNormal, EVFX_EffectType WeaponType)
{
    FRotator ImpactRotation = ImpactNormal.Rotation();
    SpawnEffect(EVFX_EffectType::WeaponImpact, ImpactLocation, ImpactRotation);
    
    // Add debris effect for heavy impacts
    SpawnEffect(EVFX_EffectType::RockDebris, ImpactLocation, ImpactRotation, 0.7f);
}

void UVFX_ParticleEffectManager::SpawnRockDebrisEffect(FVector Location, FVector Direction)
{
    FRotator DebrisRotation = Direction.Rotation();
    SpawnEffect(EVFX_EffectType::RockDebris, Location, DebrisRotation);
}

void UVFX_ParticleEffectManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    
    for (UAudioComponent* AudioEffect : ActiveAudioEffects)
    {
        if (IsValid(AudioEffect))
        {
            AudioEffect->Stop();
            AudioEffect->DestroyComponent();
        }
    }
    ActiveAudioEffects.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleEffectManager: All effects stopped"));
}

void UVFX_ParticleEffectManager::CleanupFinishedEffects()
{
    // Clean up finished Niagara effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            CleanupEffect(Effect);
            ActiveEffects.RemoveAt(i);
        }
    }
    
    // Clean up finished audio effects
    for (int32 i = ActiveAudioEffects.Num() - 1; i >= 0; i--)
    {
        UAudioComponent* AudioEffect = ActiveAudioEffects[i];
        if (!IsValid(AudioEffect) || !AudioEffect->IsPlaying())
        {
            CleanupAudioEffect(AudioEffect);
            ActiveAudioEffects.RemoveAt(i);
        }
    }
}

int32 UVFX_ParticleEffectManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num() + ActiveAudioEffects.Num();
}

void UVFX_ParticleEffectManager::SetEffectData(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData)
{
    EffectDatabase.Add(EffectType, EffectData);
}

FVFX_EffectData UVFX_ParticleEffectManager::GetEffectData(EVFX_EffectType EffectType) const
{
    const FVFX_EffectData* FoundData = EffectDatabase.Find(EffectType);
    return FoundData ? *FoundData : FVFX_EffectData();
}

void UVFX_ParticleEffectManager::PlayEffectAudio(const FVFX_EffectData& EffectData, FVector Location)
{
    if (EffectData.SoundEffect.IsValid())
    {
        USoundBase* Sound = EffectData.SoundEffect.LoadSynchronous();
        if (Sound)
        {
            UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
                GetWorld(),
                Sound,
                Location,
                FRotator::ZeroRotator,
                1.0f,
                1.0f,
                0.0f
            );
            
            if (AudioComp)
            {
                ActiveAudioEffects.Add(AudioComp);
            }
        }
    }
}

void UVFX_ParticleEffectManager::CleanupEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        Effect->DestroyComponent();
    }
}

void UVFX_ParticleEffectManager::CleanupAudioEffect(UAudioComponent* AudioEffect)
{
    if (IsValid(AudioEffect))
    {
        AudioEffect->DestroyComponent();
    }
}