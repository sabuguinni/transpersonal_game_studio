#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

UVFX_Manager::UVFX_Manager()
{
    RainComponent = nullptr;
    bIsRaining = false;
}

void UVFX_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager initialized"));
    
    InitializeVFXDatabase();
}

void UVFX_Manager::Deinitialize()
{
    // Clean up all active VFX
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->DestroyComponent();
        }
    }
    
    ActiveVFXComponents.Empty();
    ActiveVFXByType.Empty();
    
    if (IsValid(RainComponent))
    {
        RainComponent->DestroyComponent();
        RainComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UVFX_Manager::InitializeVFXDatabase()
{
    // Initialize VFX database with realistic prehistoric effects
    FVFX_EffectData CampfireData;
    CampfireData.Duration = -1.0f; // Infinite duration
    CampfireData.VolumeMultiplier = 0.8f;
    VFXDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);
    
    FVFX_EffectData FootstepData;
    FootstepData.Duration = 2.0f;
    FootstepData.VolumeMultiplier = 0.5f;
    VFXDatabase.Add(EVFX_EffectType::Dust_Footstep, FootstepData);
    
    FVFX_EffectData RainData;
    RainData.Duration = -1.0f; // Infinite duration
    RainData.VolumeMultiplier = 0.7f;
    VFXDatabase.Add(EVFX_EffectType::Water_Rain, RainData);
    
    FVFX_EffectData BloodData;
    BloodData.Duration = 5.0f;
    BloodData.VolumeMultiplier = 0.3f;
    VFXDatabase.Add(EVFX_EffectType::Blood_Splatter, BloodData);
    
    FVFX_EffectData SmokeData;
    SmokeData.Duration = 8.0f;
    SmokeData.VolumeMultiplier = 0.4f;
    VFXDatabase.Add(EVFX_EffectType::Smoke_Fire, SmokeData);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Database initialized with %d effects"), VFXDatabase.Num());
}

UNiagaraComponent* UVFX_Manager::SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!VFXDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX type not found in database"));
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = VFXDatabase[EffectType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Niagara system not valid for effect type"));
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        EffectData.NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Scale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        if (!ActiveVFXByType.Contains(EffectType))
        {
            ActiveVFXByType.Add(EffectType, TArray<UNiagaraComponent*>());
        }
        ActiveVFXByType[EffectType].Add(VFXComponent);
        
        // Play associated sound
        if (EffectData.Sound.IsValid())
        {
            PlayVFXSound(EffectData, Location);
        }
        
        // Set auto-destroy timer if duration is specified
        if (EffectData.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, VFXComponent]()
            {
                StopVFX(VFXComponent);
            }, EffectData.Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX at location: %s"), *Location.ToString());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFX_Manager::SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!IsValid(AttachToComponent))
    {
        return nullptr;
    }
    
    if (!VFXDatabase.Contains(EffectType))
    {
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = VFXDatabase[EffectType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData.NiagaraSystem.LoadSynchronous(),
        AttachToComponent,
        AttachPointName,
        Location,
        Rotation,
        Scale,
        EAttachLocation::KeepRelativeOffset,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        if (!ActiveVFXByType.Contains(EffectType))
        {
            ActiveVFXByType.Add(EffectType, TArray<UNiagaraComponent*>());
        }
        ActiveVFXByType[EffectType].Add(VFXComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX attached to component"));
    }
    
    return VFXComponent;
}

void UVFX_Manager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (!IsValid(VFXComponent))
    {
        return;
    }
    
    VFXComponent->Deactivate();
    ActiveVFXComponents.Remove(VFXComponent);
    
    // Remove from type tracking
    for (auto& Pair : ActiveVFXByType)
    {
        Pair.Value.Remove(VFXComponent);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Stopped VFX component"));
}

void UVFX_Manager::StopAllVFXOfType(EVFX_EffectType EffectType)
{
    if (!ActiveVFXByType.Contains(EffectType))
    {
        return;
    }
    
    TArray<UNiagaraComponent*>& TypeComponents = ActiveVFXByType[EffectType];
    
    for (UNiagaraComponent* VFXComp : TypeComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->Deactivate();
            ActiveVFXComponents.Remove(VFXComp);
        }
    }
    
    TypeComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Stopped all VFX of specified type"));
}

void UVFX_Manager::StartRainEffect(float Intensity)
{
    if (bIsRaining)
    {
        StopRainEffect();
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn rain VFX above the player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        FVector RainLocation = PlayerPawn->GetActorLocation() + FVector(0, 0, 2000);
        RainComponent = SpawnVFXAtLocation(EVFX_EffectType::Water_Rain, RainLocation);
        
        if (RainComponent)
        {
            // Set rain intensity parameter
            RainComponent->SetFloatParameter(FName("Intensity"), Intensity);
            bIsRaining = true;
            
            UE_LOG(LogTemp, Log, TEXT("Started rain effect with intensity: %f"), Intensity);
        }
    }
}

void UVFX_Manager::StopRainEffect()
{
    if (IsValid(RainComponent))
    {
        StopVFX(RainComponent);
        RainComponent = nullptr;
    }
    
    bIsRaining = false;
    UE_LOG(LogTemp, Log, TEXT("Stopped rain effect"));
}

void UVFX_Manager::CreateFootstepDust(const FVector& Location, float Intensity)
{
    UNiagaraComponent* DustVFX = SpawnVFXAtLocation(EVFX_EffectType::Dust_Footstep, Location);
    
    if (DustVFX)
    {
        DustVFX->SetFloatParameter(FName("Intensity"), Intensity);
        UE_LOG(LogTemp, Log, TEXT("Created footstep dust at: %s"), *Location.ToString());
    }
}

void UVFX_Manager::CreateBloodSplatter(const FVector& Location, const FVector& Direction, float Amount)
{
    UNiagaraComponent* BloodVFX = SpawnVFXAtLocation(EVFX_EffectType::Blood_Splatter, Location);
    
    if (BloodVFX)
    {
        BloodVFX->SetVectorParameter(FName("Direction"), Direction);
        BloodVFX->SetFloatParameter(FName("Amount"), Amount);
        UE_LOG(LogTemp, Log, TEXT("Created blood splatter at: %s"), *Location.ToString());
    }
}

UNiagaraComponent* UVFX_Manager::CreateCampfire(const FVector& Location)
{
    UNiagaraComponent* FireVFX = SpawnVFXAtLocation(EVFX_EffectType::Fire_Campfire, Location);
    
    if (FireVFX)
    {
        UE_LOG(LogTemp, Log, TEXT("Created campfire at: %s"), *Location.ToString());
    }
    
    return FireVFX;
}

void UVFX_Manager::ExtinguishFire(UNiagaraComponent* FireComponent)
{
    if (IsValid(FireComponent))
    {
        // Gradually reduce fire intensity before stopping
        FireComponent->SetFloatParameter(FName("Intensity"), 0.0f);
        
        // Stop after a brief delay to show extinguishing effect
        UWorld* World = GetWorld();
        if (World)
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, FireComponent]()
            {
                StopVFX(FireComponent);
            }, 2.0f, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Extinguishing fire"));
    }
}

void UVFX_Manager::CleanupInactiveVFX()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* VFXComp)
    {
        return !IsValid(VFXComp) || !VFXComp->IsActive();
    });
    
    for (auto& Pair : ActiveVFXByType)
    {
        Pair.Value.RemoveAll([](UNiagaraComponent* VFXComp)
        {
            return !IsValid(VFXComp) || !VFXComp->IsActive();
        });
    }
}

UAudioComponent* UVFX_Manager::PlayVFXSound(const FVFX_EffectData& EffectData, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World || !EffectData.Sound.IsValid())
    {
        return nullptr;
    }
    
    UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
        World,
        EffectData.Sound.LoadSynchronous(),
        Location,
        FRotator::ZeroRotator,
        EffectData.VolumeMultiplier
    );
    
    return AudioComp;
}