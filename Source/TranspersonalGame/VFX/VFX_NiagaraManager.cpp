#include "VFX_NiagaraManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

UVFX_NiagaraManager::UVFX_NiagaraManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    MaxActiveEffects = 50;
    CurrentLODLevel = 0;
    EffectCleanupInterval = 2.0f;
    LastCleanupTime = 0.0f;
    CurrentWeatherEffect = nullptr;
}

void UVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXRegistry();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Initialized with %d effect types"), VFXRegistry.Num());
}

void UVFX_NiagaraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastCleanupTime += DeltaTime;
    
    // Periodic cleanup of finished effects
    if (LastCleanupTime >= EffectCleanupInterval)
    {
        CleanupFinishedEffects();
        LastCleanupTime = 0.0f;
    }
}

void UVFX_NiagaraManager::InitializeVFXRegistry()
{
    // Register prehistoric VFX effects with placeholder paths
    // In production, these would point to actual Niagara systems
    RegisterVFXEffect(EVFX_EffectType::Fire_Campfire, TEXT("/Game/VFX/NS_Fire_Campfire"), 0.0f, 1.0f); // Continuous
    RegisterVFXEffect(EVFX_EffectType::Dust_Footstep, TEXT("/Game/VFX/NS_Dust_Footstep"), 2.0f, 1.0f);
    RegisterVFXEffect(EVFX_EffectType::Blood_Splatter, TEXT("/Game/VFX/NS_Blood_Splatter"), 3.0f, 1.0f);
    RegisterVFXEffect(EVFX_EffectType::Weather_Rain, TEXT("/Game/VFX/NS_Weather_Rain"), 0.0f, 1.0f); // Continuous
    RegisterVFXEffect(EVFX_EffectType::Weather_Snow, TEXT("/Game/VFX/NS_Weather_Snow"), 0.0f, 1.0f); // Continuous
    RegisterVFXEffect(EVFX_EffectType::Volcanic_Ash, TEXT("/Game/VFX/NS_Volcanic_Ash"), 0.0f, 1.0f); // Continuous
    RegisterVFXEffect(EVFX_EffectType::Water_Splash, TEXT("/Game/VFX/NS_Water_Splash"), 1.5f, 1.0f);
    RegisterVFXEffect(EVFX_EffectType::Combat_Impact, TEXT("/Game/VFX/NS_Combat_Impact"), 1.0f, 1.0f);
    RegisterVFXEffect(EVFX_EffectType::Breath_Steam, TEXT("/Game/VFX/NS_Breath_Steam"), 2.0f, 1.0f);
    RegisterVFXEffect(EVFX_EffectType::Crafting_Sparks, TEXT("/Game/VFX/NS_Crafting_Sparks"), 3.0f, 1.0f);
}

void UVFX_NiagaraManager::RegisterVFXEffect(EVFX_EffectType EffectType, const FString& SystemPath, float Duration, float Scale)
{
    FVFX_EffectData EffectData;
    EffectData.EffectType = EffectType;
    EffectData.Duration = Duration;
    EffectData.Scale = Scale;
    EffectData.bAutoDestroy = (Duration > 0.0f); // Continuous effects don't auto-destroy
    EffectData.LODLevel = 0;
    
    // In production, load the actual Niagara system
    // EffectData.NiagaraSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(SystemPath));
    
    VFXRegistry.Add(EffectType, EffectData);
}

UNiagaraComponent* UVFX_NiagaraManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!VFXRegistry.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Effect type %d not found in registry"), (int32)EffectType);
        return nullptr;
    }
    
    // Check active effect limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Max active effects reached (%d), skipping spawn"), MaxActiveEffects);
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = VFXRegistry[EffectType];
    UNiagaraComponent* NewEffect = CreateNiagaraComponent(EffectData, Location, Rotation);
    
    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Spawned effect %d at location %s"), (int32)EffectType, *Location.ToString());
    }
    
    return NewEffect;
}

UNiagaraComponent* UVFX_NiagaraManager::SpawnVFXEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName)
{
    if (!AttachToComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Invalid attach component"));
        return nullptr;
    }
    
    UNiagaraComponent* NewEffect = SpawnVFXEffect(EffectType, AttachToComponent->GetComponentLocation(), AttachToComponent->GetComponentRotation());
    
    if (NewEffect)
    {
        NewEffect->AttachToComponent(AttachToComponent, FAttachmentTransformRules::KeepWorldTransform, AttachPointName);
    }
    
    return NewEffect;
}

UNiagaraComponent* UVFX_NiagaraManager::CreateNiagaraComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        nullptr, // Would be EffectData.NiagaraSystem.LoadSynchronous() in production
        Location,
        Rotation,
        FVector(EffectData.Scale),
        EffectData.bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        ApplyLODSettings(NiagaraComp, CurrentLODLevel);
        
        // Set duration for timed effects
        if (EffectData.Duration > 0.0f)
        {
            // Schedule destruction
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                StopVFXEffect(NiagaraComp);
            }, EffectData.Duration, false);
        }
    }
    
    return NiagaraComp;
}

void UVFX_NiagaraManager::StopVFXEffect(UNiagaraComponent* EffectComponent)
{
    if (!EffectComponent)
    {
        return;
    }
    
    EffectComponent->Deactivate();
    ActiveEffects.Remove(EffectComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Stopped VFX effect"));
}

void UVFX_NiagaraManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            Effect->Deactivate();
        }
    }
    
    ActiveEffects.Empty();
    
    if (CurrentWeatherEffect)
    {
        CurrentWeatherEffect->Deactivate();
        CurrentWeatherEffect = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Stopped all VFX effects"));
}

void UVFX_NiagaraManager::PlayDinosaurFootstepVFX(FVector Location, float DinosaurSize)
{
    UNiagaraComponent* FootstepEffect = SpawnVFXEffect(EVFX_EffectType::Dust_Footstep, Location);
    
    if (FootstepEffect)
    {
        // Scale effect based on dinosaur size
        FootstepEffect->SetWorldScale3D(FVector(DinosaurSize));
        
        // Set parameters for dust intensity based on size
        FootstepEffect->SetFloatParameter(TEXT("DustIntensity"), DinosaurSize);
        FootstepEffect->SetFloatParameter(TEXT("ParticleCount"), DinosaurSize * 100.0f);
    }
}

void UVFX_NiagaraManager::PlayBloodImpactVFX(FVector Location, FVector ImpactDirection)
{
    UNiagaraComponent* BloodEffect = SpawnVFXEffect(EVFX_EffectType::Blood_Splatter, Location);
    
    if (BloodEffect)
    {
        // Set impact direction for blood spray
        FootstepEffect->SetVectorParameter(TEXT("ImpactDirection"), ImpactDirection.GetSafeNormal());
        FootstepEffect->SetFloatParameter(TEXT("ImpactForce"), ImpactDirection.Size());
    }
}

void UVFX_NiagaraManager::PlayCampfireVFX(FVector Location)
{
    // Stop any existing campfire effect first
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (Effect && Effect->GetAsset() && Effect->GetAsset()->GetName().Contains(TEXT("Campfire")))
        {
            StopVFXEffect(Effect);
        }
    }
    
    // Start new campfire effect
    SpawnVFXEffect(EVFX_EffectType::Fire_Campfire, Location);
}

void UVFX_NiagaraManager::StartWeatherVFX(EVFX_EffectType WeatherType, float Intensity)
{
    // Stop current weather effect
    StopWeatherVFX();
    
    // Validate weather type
    if (WeatherType != EVFX_EffectType::Weather_Rain && 
        WeatherType != EVFX_EffectType::Weather_Snow && 
        WeatherType != EVFX_EffectType::Volcanic_Ash)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraManager: Invalid weather type %d"), (int32)WeatherType);
        return;
    }
    
    // Spawn weather effect at player location (would get actual player location in production)
    FVector WeatherLocation = FVector::ZeroVector;
    CurrentWeatherEffect = SpawnVFXEffect(WeatherType, WeatherLocation);
    
    if (CurrentWeatherEffect)
    {
        CurrentWeatherEffect->SetFloatParameter(TEXT("Intensity"), Intensity);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Started weather effect %d with intensity %f"), (int32)WeatherType, Intensity);
    }
}

void UVFX_NiagaraManager::StopWeatherVFX()
{
    if (CurrentWeatherEffect)
    {
        StopVFXEffect(CurrentWeatherEffect);
        CurrentWeatherEffect = nullptr;
    }
}

void UVFX_NiagaraManager::SetVFXLODLevel(int32 NewLODLevel)
{
    CurrentLODLevel = FMath::Clamp(NewLODLevel, 0, 3);
    
    // Apply LOD to all active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            ApplyLODSettings(Effect, CurrentLODLevel);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Set LOD level to %d"), CurrentLODLevel);
}

void UVFX_NiagaraManager::ApplyLODSettings(UNiagaraComponent* Component, int32 LODLevel)
{
    if (!Component)
    {
        return;
    }
    
    // Apply LOD-based quality settings
    switch (LODLevel)
    {
        case 0: // High quality
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 1.0f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 1.0f);
            break;
        case 1: // Medium quality
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 0.75f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 0.7f);
            break;
        case 2: // Low quality
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 0.5f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 0.5f);
            break;
        case 3: // Very low quality
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 0.25f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 0.3f);
            break;
    }
}

int32 UVFX_NiagaraManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_NiagaraManager::CleanupFinishedEffects()
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        
        if (!Effect || !Effect->IsActive() || Effect->IsBeingDestroyed())
        {
            ActiveEffects.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraManager: Cleaned up %d finished effects"), RemovedCount);
    }
}