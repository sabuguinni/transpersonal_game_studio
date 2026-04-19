#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

void UVFX_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager Initialized"));
    
    // Initialize default settings
    MaxActiveEffects = 50;
    LODDistance_High = 1000.0f;
    LODDistance_Medium = 2000.0f;
    LODDistance_Low = 4000.0f;
    
    // Initialize effect templates
    InitializeEffectTemplates();
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_SystemManager::CleanupExpiredEffects,
            1.0f,
            true
        );
    }
}

void UVFX_SystemManager::Deinitialize()
{
    StopAllEffects();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

void UVFX_SystemManager::InitializeEffectTemplates()
{
    // Initialize footstep impact effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Impact;
    FootstepData.Duration = 2.0f;
    FootstepData.Scale = 1.0f;
    FootstepData.bAutoDestroy = true;
    EffectTemplates.Add(EVFX_EffectType::Impact, FootstepData);
    
    // Initialize fire effect
    FVFX_EffectData FireData;
    FireData.EffectType = EVFX_EffectType::Fire;
    FireData.Duration = 0.0f; // Continuous
    FireData.Scale = 1.0f;
    FireData.bAutoDestroy = false;
    EffectTemplates.Add(EVFX_EffectType::Fire, FireData);
    
    // Initialize dust effect
    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::Dust;
    DustData.Duration = 3.0f;
    DustData.Scale = 1.0f;
    DustData.bAutoDestroy = true;
    EffectTemplates.Add(EVFX_EffectType::Dust, DustData);
    
    // Initialize blood effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood;
    BloodData.Duration = 5.0f;
    BloodData.Scale = 1.0f;
    BloodData.bAutoDestroy = true;
    EffectTemplates.Add(EVFX_EffectType::Blood, BloodData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Effect Templates Initialized: %d templates"), EffectTemplates.Num());
}

void UVFX_SystemManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, EVFX_IntensityLevel Intensity)
{
    if (!EffectTemplates.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Effect type not found in templates"));
        return;
    }
    
    // Check max effects limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Max active effects reached (%d)"), MaxActiveEffects);
        return;
    }
    
    const FVFX_EffectData& EffectData = EffectTemplates[EffectType];
    
    // Calculate scale based on intensity
    float IntensityScale = 1.0f;
    switch (Intensity)
    {
        case EVFX_IntensityLevel::Low:
            IntensityScale = 0.5f;
            break;
        case EVFX_IntensityLevel::Medium:
            IntensityScale = 1.0f;
            break;
        case EVFX_IntensityLevel::High:
            IntensityScale = 1.5f;
            break;
        case EVFX_IntensityLevel::Extreme:
            IntensityScale = 2.0f;
            break;
    }
    
    float FinalScale = EffectData.Scale * IntensityScale;
    
    // For now, create a simple placeholder effect using basic Niagara
    // In a real implementation, this would load specific Niagara systems
    UE_LOG(LogTemp, Log, TEXT("VFX: Spawning %s effect at location %s with scale %f"), 
        *UEnum::GetValueAsString(EffectType), 
        *Location.ToString(), 
        FinalScale);
    
    // Try to spawn a basic Niagara effect
    if (UWorld* World = GetWorld())
    {
        // Create a simple particle effect using UNiagaraFunctionLibrary
        UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            nullptr, // We'll need to load actual Niagara systems later
            Location,
            Rotation,
            FVector(FinalScale),
            EffectData.bAutoDestroy
        );
        
        if (NewEffect)
        {
            ActiveEffects.Add(NewEffect);
            UE_LOG(LogTemp, Log, TEXT("VFX: Successfully spawned effect"));
        }
    }
}

void UVFX_SystemManager::SpawnDinosaurFootstep(FVector Location, float DinosaurSize)
{
    EVFX_IntensityLevel Intensity = EVFX_IntensityLevel::Medium;
    
    // Scale intensity based on dinosaur size
    if (DinosaurSize > 2.0f)
    {
        Intensity = EVFX_IntensityLevel::Extreme; // Large dinosaurs like T-Rex
    }
    else if (DinosaurSize > 1.0f)
    {
        Intensity = EVFX_IntensityLevel::High; // Medium dinosaurs
    }
    else
    {
        Intensity = EVFX_IntensityLevel::Low; // Small dinosaurs like Raptors
    }
    
    // Spawn dust impact effect
    SpawnEffect(EVFX_EffectType::Dust, Location, FRotator::ZeroRotator, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Dinosaur footstep at %s, size %f, intensity %s"), 
        *Location.ToString(), 
        DinosaurSize,
        *UEnum::GetValueAsString(Intensity));
}

void UVFX_SystemManager::SpawnCampfire(FVector Location, bool bWithSound)
{
    // Spawn fire effect
    SpawnEffect(EVFX_EffectType::Fire, Location, FRotator::ZeroRotator, EVFX_IntensityLevel::Medium);
    
    // Add sound if requested
    if (bWithSound && GetWorld())
    {
        // Create audio component for campfire crackling
        UAudioComponent* FireSound = CreateSoundEffect(nullptr, Location, 0.8f);
        if (FireSound)
        {
            UE_LOG(LogTemp, Log, TEXT("VFX: Campfire with sound created at %s"), *Location.ToString());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Campfire spawned at %s"), *Location.ToString());
}

void UVFX_SystemManager::SpawnBloodImpact(FVector Location, FVector ImpactDirection)
{
    // Normalize impact direction
    FVector Direction = ImpactDirection.GetSafeNormal();
    
    // Calculate rotation from impact direction
    FRotator ImpactRotation = Direction.Rotation();
    
    // Spawn blood effect
    SpawnEffect(EVFX_EffectType::Blood, Location, ImpactRotation, EVFX_IntensityLevel::High);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Blood impact at %s, direction %s"), 
        *Location.ToString(), 
        *Direction.ToString());
}

void UVFX_SystemManager::SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius)
{
    if (WeatherType != EVFX_EffectType::Weather)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX: Invalid weather effect type"));
        return;
    }
    
    // Spawn weather effect with large radius
    SpawnEffect(WeatherType, Location, FRotator::ZeroRotator, EVFX_IntensityLevel::Medium);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Weather effect spawned at %s with radius %f"), 
        *Location.ToString(), 
        Radius);
}

void UVFX_SystemManager::StopAllEffects()
{
    // Stop all active Niagara effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    
    // Stop all active sounds
    for (UAudioComponent* Sound : ActiveSounds)
    {
        if (IsValid(Sound))
        {
            Sound->Stop();
            Sound->DestroyComponent();
        }
    }
    ActiveSounds.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX: All effects stopped"));
}

void UVFX_SystemManager::SetGlobalVFXQuality(int32 QualityLevel)
{
    // Adjust max effects based on quality level
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            break;
        case 1: // Medium
            MaxActiveEffects = 50;
            break;
        case 2: // High
            MaxActiveEffects = 100;
            break;
        case 3: // Ultra
            MaxActiveEffects = 200;
            break;
        default:
            MaxActiveEffects = 50;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Quality level set to %d, max effects: %d"), 
        QualityLevel, 
        MaxActiveEffects);
}

void UVFX_SystemManager::UpdateLODDistances()
{
    if (!GetWorld())
        return;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for all active effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Effect->GetComponentLocation());
        
        // Adjust effect quality based on distance
        if (Distance > LODDistance_Low)
        {
            Effect->SetVisibility(false);
        }
        else if (Distance > LODDistance_Medium)
        {
            Effect->SetVisibility(true);
            // Set low quality parameters
        }
        else if (Distance > LODDistance_High)
        {
            Effect->SetVisibility(true);
            // Set medium quality parameters
        }
        else
        {
            Effect->SetVisibility(true);
            // Set high quality parameters
        }
    }
}

int32 UVFX_SystemManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_SystemManager::CleanupExpiredEffects()
{
    // Remove invalid or destroyed effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ActiveEffects[i]))
        {
            ActiveEffects.RemoveAt(i);
        }
    }
    
    // Remove invalid or stopped sounds
    for (int32 i = ActiveSounds.Num() - 1; i >= 0; --i)
    {
        UAudioComponent* Sound = ActiveSounds[i];
        if (!IsValid(Sound) || !Sound->IsPlaying())
        {
            if (IsValid(Sound))
            {
                Sound->DestroyComponent();
            }
            ActiveSounds.RemoveAt(i);
        }
    }
}

UNiagaraComponent* UVFX_SystemManager::CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, float Scale)
{
    if (!System || !GetWorld())
        return nullptr;
    
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(Scale),
        true
    );
    
    if (NewEffect)
    {
        ActiveEffects.Add(NewEffect);
    }
    
    return NewEffect;
}

UAudioComponent* UVFX_SystemManager::CreateSoundEffect(USoundCue* Sound, FVector Location, float Volume)
{
    if (!Sound || !GetWorld())
        return nullptr;
    
    UAudioComponent* NewSound = UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FRotator::ZeroRotator,
        Volume
    );
    
    if (NewSound)
    {
        ActiveSounds.Add(NewSound);
    }
    
    return NewSound;
}