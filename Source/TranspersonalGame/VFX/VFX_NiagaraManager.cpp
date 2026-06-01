#include "VFX_NiagaraManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default values
    CleanupTimer = 0.0f;
    CleanupInterval = 5.0f;
}

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the effect registry with default values
    InitializeEffectRegistry();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: BeginPlay - System initialized"));
}

void AVFX_NiagaraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update active effects
    UpdateActiveEffects(DeltaTime);
    
    // Periodic cleanup
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= CleanupInterval)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

void AVFX_NiagaraManager::InitializeEffectRegistry()
{
    // Initialize default effect data
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::FootstepDust;
    FootstepData.Duration = 3.0f;
    FootstepData.Scale = 1.0f;
    FootstepData.bAutoDestroy = true;
    EffectRegistry.Add(EVFX_EffectType::FootstepDust, FootstepData);

    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::CampfireFire;
    CampfireData.Duration = -1.0f; // Continuous
    CampfireData.Scale = 1.0f;
    CampfireData.bAutoDestroy = false;
    EffectRegistry.Add(EVFX_EffectType::CampfireFire, CampfireData);

    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodImpact;
    BloodData.Duration = 2.0f;
    BloodData.Scale = 1.0f;
    BloodData.bAutoDestroy = true;
    EffectRegistry.Add(EVFX_EffectType::BloodImpact, BloodData);

    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::WeatherRain;
    RainData.Duration = -1.0f; // Continuous
    RainData.Scale = 1.0f;
    RainData.bAutoDestroy = false;
    EffectRegistry.Add(EVFX_EffectType::WeatherRain, RainData);

    FVFX_EffectData VaporData;
    VaporData.EffectType = EVFX_EffectType::BreathVapor;
    VaporData.Duration = 1.5f;
    VaporData.Scale = 1.0f;
    VaporData.bAutoDestroy = true;
    EffectRegistry.Add(EVFX_EffectType::BreathVapor, VaporData);

    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Effect registry initialized with %d effects"), EffectRegistry.Num());
}

UNiagaraComponent* AVFX_NiagaraManager::SpawnVFXAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectRegistry.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Effect type not found in registry"));
        return nullptr;
    }

    FVFX_EffectData* EffectData = EffectRegistry.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Invalid Niagara system for effect type"));
        return nullptr;
    }

    // Spawn Niagara effect at location
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData->NiagaraSystem.Get(),
        Location,
        Rotation,
        FVector(EffectData->Scale),
        EffectData->bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Spawned effect at location %s"), *Location.ToString());
    }

    return NiagaraComp;
}

UNiagaraComponent* AVFX_NiagaraManager::SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName)
{
    if (!AttachTo || !EffectRegistry.Contains(EffectType))
    {
        return nullptr;
    }

    FVFX_EffectData* EffectData = EffectRegistry.Find(EffectType);
    if (!EffectData || !EffectData->NiagaraSystem.IsValid())
    {
        return nullptr;
    }

    // Spawn Niagara effect attached to component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData->NiagaraSystem.Get(),
        AttachTo,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector(EffectData->Scale),
        EAttachLocation::KeepWorldPosition,
        EffectData->bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Spawned attached effect"));
    }

    return NiagaraComp;
}

void AVFX_NiagaraManager::StopVFXEffect(UNiagaraComponent* Effect)
{
    if (Effect && IsValid(Effect))
    {
        Effect->DestroyComponent();
        ActiveEffects.Remove(Effect);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Stopped VFX effect"));
    }
}

void AVFX_NiagaraManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Stopped all VFX effects"));
}

void AVFX_NiagaraManager::PlayFootstepDust(FVector Location, float DinosaurSize)
{
    FVFX_EffectData* EffectData = EffectRegistry.Find(EVFX_EffectType::FootstepDust);
    if (EffectData)
    {
        // Scale effect based on dinosaur size
        float OriginalScale = EffectData->Scale;
        EffectData->Scale = DinosaurSize;
        
        SpawnVFXAtLocation(EVFX_EffectType::FootstepDust, Location, FRotator::ZeroRotator);
        
        // Restore original scale
        EffectData->Scale = OriginalScale;
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Played footstep dust at %s with size %f"), *Location.ToString(), DinosaurSize);
    }
}

void AVFX_NiagaraManager::PlayCampfireEffect(FVector Location)
{
    SpawnVFXAtLocation(EVFX_EffectType::CampfireFire, Location, FRotator::ZeroRotator);
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Played campfire effect at %s"), *Location.ToString());
}

void AVFX_NiagaraManager::PlayBloodImpact(FVector Location, FVector ImpactNormal)
{
    FRotator ImpactRotation = ImpactNormal.Rotation();
    SpawnVFXAtLocation(EVFX_EffectType::BloodImpact, Location, ImpactRotation);
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Played blood impact at %s"), *Location.ToString());
}

void AVFX_NiagaraManager::PlayWeatherRain(bool bEnable)
{
    if (bEnable)
    {
        FVector SkyLocation = FVector(0.0f, 0.0f, 2000.0f);
        SpawnVFXAtLocation(EVFX_EffectType::WeatherRain, SkyLocation, FRotator::ZeroRotator);
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Enabled weather rain"));
    }
    else
    {
        // Stop all rain effects
        for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
        {
            UNiagaraComponent* Effect = ActiveEffects[i];
            if (Effect && Effect->GetAsset())
            {
                // Check if this is a rain effect (simplified check)
                StopVFXEffect(Effect);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Disabled weather rain"));
    }
}

void AVFX_NiagaraManager::PlayBreathVapor(FVector Location, float Temperature)
{
    // Only show breath vapor in cold conditions
    if (Temperature < 10.0f)
    {
        SpawnVFXAtLocation(EVFX_EffectType::BreathVapor, Location, FRotator::ZeroRotator);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Played breath vapor at %s (temp: %f)"), *Location.ToString(), Temperature);
    }
}

void AVFX_NiagaraManager::RegisterEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem, float Duration)
{
    if (NiagaraSystem)
    {
        FVFX_EffectData NewEffectData;
        NewEffectData.EffectType = EffectType;
        NewEffectData.NiagaraSystem = NiagaraSystem;
        NewEffectData.Duration = Duration;
        NewEffectData.Scale = 1.0f;
        NewEffectData.bAutoDestroy = (Duration > 0.0f);
        
        EffectRegistry.Add(EffectType, NewEffectData);
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Registered new effect type"));
    }
}

void AVFX_NiagaraManager::CleanupExpiredEffects()
{
    int32 RemovedCount = 0;
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Cleaned up %d expired effects"), RemovedCount);
    }
}

int32 AVFX_NiagaraManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_NiagaraManager::UpdateActiveEffects(float DeltaTime)
{
    // Update logic for active effects if needed
    // This could include fading, scaling, or other time-based modifications
}