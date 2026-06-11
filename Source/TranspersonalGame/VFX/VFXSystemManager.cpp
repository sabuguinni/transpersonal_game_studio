#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UVFX_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeEffectLibrary();
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager Initialized"));
}

void UVFX_SystemManager::Deinitialize()
{
    EffectLibrary.Empty();
    Super::Deinitialize();
}

void UVFX_SystemManager::InitializeEffectLibrary()
{
    // Initialize fire effect
    FVFX_EffectData FireData;
    FireData.Duration = 5.0f;
    FireData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Fire, FireData);

    // Initialize dust effect
    FVFX_EffectData DustData;
    DustData.Duration = 2.0f;
    DustData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Dust, DustData);

    // Initialize blood effect
    FVFX_EffectData BloodData;
    BloodData.Duration = 3.0f;
    BloodData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Blood, BloodData);

    // Initialize water effect
    FVFX_EffectData WaterData;
    WaterData.Duration = 4.0f;
    WaterData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Water, WaterData);

    // Initialize impact effect
    FVFX_EffectData ImpactData;
    ImpactData.Duration = 1.5f;
    ImpactData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Impact, ImpactData);

    // Initialize footstep effect
    FVFX_EffectData FootstepData;
    FootstepData.Duration = 1.0f;
    FootstepData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Footstep, FootstepData);

    // Initialize breath effect
    FVFX_EffectData BreathData;
    BreathData.Duration = 2.0f;
    BreathData.Scale = 1.0f;
    EffectLibrary.Add(EVFX_EffectType::Breath, BreathData);
}

void UVFX_SystemManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float Scale)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not found in library"));
        return;
    }

    const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
    
    if (EffectData.ParticleSystem)
    {
        SpawnNiagaraEffect(EffectData.ParticleSystem, Location, Rotation, Scale);
    }

    if (EffectData.Sound)
    {
        PlayEffectSound(EffectData.Sound, Location);
    }
}

void UVFX_SystemManager::SpawnFootstepEffect(FVector Location, bool bIsLarge)
{
    float Scale = bIsLarge ? 2.0f : 1.0f;
    SpawnEffect(EVFX_EffectType::Footstep, Location, FRotator::ZeroRotator, Scale);
    
    // Add dust for large footsteps
    if (bIsLarge)
    {
        FVector DustLocation = Location + FVector(0, 0, 10);
        SpawnEffect(EVFX_EffectType::Dust, DustLocation, FRotator::ZeroRotator, Scale * 1.5f);
    }
}

void UVFX_SystemManager::SpawnImpactEffect(FVector Location, FVector Normal)
{
    FRotator ImpactRotation = Normal.Rotation();
    SpawnEffect(EVFX_EffectType::Impact, Location, ImpactRotation, 1.0f);
}

void UVFX_SystemManager::SpawnFireEffect(FVector Location, float Intensity)
{
    float Scale = FMath::Clamp(Intensity, 0.5f, 3.0f);
    SpawnEffect(EVFX_EffectType::Fire, Location, FRotator::ZeroRotator, Scale);
}

void UVFX_SystemManager::SpawnBloodEffect(FVector Location, FVector Direction)
{
    FRotator BloodRotation = Direction.Rotation();
    SpawnEffect(EVFX_EffectType::Blood, Location, BloodRotation, 1.0f);
}

void UVFX_SystemManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius)
{
    if (WeatherType != EVFX_EffectType::Weather)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid weather effect type"));
        return;
    }

    float Scale = Radius / 1000.0f;
    SpawnEffect(WeatherType, Location, FRotator::ZeroRotator, Scale);
}

UNiagaraComponent* UVFX_SystemManager::SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale)
{
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("Niagara System is null"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in VFX System Manager"));
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, System, Location, Rotation, FVector(Scale), true, true
    );

    if (NiagaraComp)
    {
        UE_LOG(LogTemp, Log, TEXT("Spawned Niagara effect at location: %s"), *Location.ToString());
    }

    return NiagaraComp;
}

UAudioComponent* UVFX_SystemManager::PlayEffectSound(USoundCue* Sound, FVector Location)
{
    if (!Sound)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World, Sound, Location, FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f
    );

    return AudioComp;
}