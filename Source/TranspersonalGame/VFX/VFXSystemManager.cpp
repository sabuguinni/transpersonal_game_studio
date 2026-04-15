#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

const float UVFX_SystemManager::CleanupInterval = 2.0f;

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Performance defaults
    MaxActiveEffects = 50;
    CullingDistance = 5000.0f;
    bEnableLODSystem = true;
    LastCleanupTime = 0.0f;
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d effect types"), EffectDatabase.Num());
}

void UVFX_SystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllVFXEffects();
    Super::EndPlay(EndPlayReason);
}

void UVFX_SystemManager::InitializeEffectDatabase()
{
    EffectDatabase.Empty();
    
    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.DefaultScale = FVector(1.0f, 1.0f, 1.2f);
    CampfireData.DefaultLifetime = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    CampfireData.DefaultColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
    EffectDatabase.Add(CampfireData);
    
    // Initialize footstep effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Impact_Footstep;
    FootstepData.DefaultScale = FVector(0.5f, 0.5f, 0.3f);
    FootstepData.DefaultLifetime = 2.0f;
    FootstepData.bAutoDestroy = true;
    FootstepData.DefaultColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    EffectDatabase.Add(FootstepData);
    
    // Initialize dinosaur step effect
    FVFX_EffectData DinoStepData;
    DinoStepData.EffectType = EVFX_EffectType::Impact_DinoStep;
    DinoStepData.DefaultScale = FVector(2.0f, 2.0f, 1.5f);
    DinoStepData.DefaultLifetime = 3.0f;
    DinoStepData.bAutoDestroy = true;
    DinoStepData.DefaultColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    EffectDatabase.Add(DinoStepData);
    
    // Initialize rain effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.DefaultScale = FVector(10.0f, 10.0f, 5.0f);
    RainData.DefaultLifetime = -1.0f; // Infinite
    RainData.bAutoDestroy = false;
    RainData.DefaultColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.7f);
    EffectDatabase.Add(RainData);
    
    // Initialize blood splatter effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood_Splatter;
    BloodData.DefaultScale = FVector(1.0f, 1.0f, 0.8f);
    BloodData.DefaultLifetime = 1.5f;
    BloodData.bAutoDestroy = true;
    BloodData.DefaultColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
    EffectDatabase.Add(BloodData);
    
    // Initialize dust cloud effect
    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::Dust_Cloud;
    DustData.DefaultScale = FVector(1.5f, 1.5f, 1.0f);
    DustData.DefaultLifetime = 4.0f;
    DustData.bAutoDestroy = true;
    DustData.DefaultColor = FLinearColor(0.7f, 0.6f, 0.5f, 0.6f);
    EffectDatabase.Add(DustData);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Effect database initialized with %d effects"), EffectDatabase.Num());
}

UNiagaraComponent* UVFX_SystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    // Check if we've hit the maximum active effects limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX System: Maximum active effects reached (%d)"), MaxActiveEffects);
            return nullptr;
        }
    }
    
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System: Effect type not found in database"));
        return nullptr;
    }
    
    // For now, create a basic Niagara component without a specific system
    // In a full implementation, you would load the actual Niagara system asset
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Would be EffectData->NiagaraSystem.LoadSynchronous() in full implementation
        Location,
        Rotation,
        Scale * EffectData->DefaultScale,
        true,
        EffectData->bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        RegisterActiveEffect(NiagaraComp);
        
        // Set effect properties
        if (EffectData->DefaultLifetime > 0.0f)
        {
            // Schedule auto-destruction
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                StopVFXEffect(NiagaraComp);
            }, EffectData->DefaultLifetime, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX System: Spawned effect type %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }
    
    return NiagaraComp;
}

UNiagaraComponent* UVFX_SystemManager::SpawnVFXEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName, FVector RelativeLocation)
{
    if (!AttachTo)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System: Cannot attach effect - AttachTo component is null"));
        return nullptr;
    }
    
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        return nullptr;
    }
    
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
        nullptr, // Would be EffectData->NiagaraSystem.LoadSynchronous()
        AttachTo,
        SocketName,
        RelativeLocation,
        FRotator::ZeroRotator,
        EffectData->DefaultScale,
        EAttachLocation::KeepRelativeOffset,
        EffectData->bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        RegisterActiveEffect(NiagaraComp);
        UE_LOG(LogTemp, Log, TEXT("VFX System: Attached effect type %d to component"), (int32)EffectType);
    }
    
    return NiagaraComp;
}

void UVFX_SystemManager::StopVFXEffect(UNiagaraComponent* EffectComponent)
{
    if (!EffectComponent)
    {
        return;
    }
    
    EffectComponent->Deactivate();
    UnregisterActiveEffect(EffectComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFX System: Stopped effect component"));
}

void UVFX_SystemManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX System: Stopped all active effects"));
}

UNiagaraComponent* UVFX_SystemManager::CreateCampfire(FVector Location, float Intensity)
{
    UNiagaraComponent* CampfireEffect = SpawnVFXEffect(EVFX_EffectType::Fire_Campfire, Location);
    
    if (CampfireEffect)
    {
        // Set campfire-specific parameters
        FVector ScaleMultiplier = FVector(Intensity, Intensity, Intensity);
        CampfireEffect->SetWorldScale3D(CampfireEffect->GetComponentScale() * ScaleMultiplier);
        
        UE_LOG(LogTemp, Log, TEXT("VFX System: Created campfire at %s with intensity %f"), 
               *Location.ToString(), Intensity);
    }
    
    return CampfireEffect;
}

void UVFX_SystemManager::CreateFootstepEffect(FVector Location, float DinosaurSize)
{
    EVFX_EffectType EffectType = (DinosaurSize > 2.0f) ? EVFX_EffectType::Impact_DinoStep : EVFX_EffectType::Impact_Footstep;
    
    FVector Scale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    SpawnVFXEffect(EffectType, Location, FRotator::ZeroRotator, Scale);
    
    UE_LOG(LogTemp, Log, TEXT("VFX System: Created footstep effect for size %f"), DinosaurSize);
}

void UVFX_SystemManager::StartRainEffect(float Intensity)
{
    // Stop any existing rain
    StopRainEffect();
    
    // Get player location for rain positioning
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector RainLocation = PlayerPawn->GetActorLocation() + FVector(0, 0, 1000);
        FVector Scale = FVector(Intensity, Intensity, 1.0f);
        
        SpawnVFXEffect(EVFX_EffectType::Weather_Rain, RainLocation, FRotator::ZeroRotator, Scale);
        UE_LOG(LogTemp, Log, TEXT("VFX System: Started rain effect with intensity %f"), Intensity);
    }
}

void UVFX_SystemManager::StopRainEffect()
{
    // Find and stop rain effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (Effect && IsValid(Effect))
        {
            // In a full implementation, you would check the effect type
            // For now, we'll assume any large-scale effect is rain
            FVector Scale = Effect->GetComponentScale();
            if (Scale.X > 5.0f) // Rain effects are typically large scale
            {
                StopVFXEffect(Effect);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX System: Stopped rain effects"));
}

void UVFX_SystemManager::CreateBloodSplatter(FVector Location, FVector Direction, float Amount)
{
    FRotator SplatterRotation = Direction.Rotation();
    FVector Scale = FVector(Amount, Amount, 1.0f);
    
    SpawnVFXEffect(EVFX_EffectType::Blood_Splatter, Location, SplatterRotation, Scale);
    
    UE_LOG(LogTemp, Log, TEXT("VFX System: Created blood splatter at %s"), *Location.ToString());
}

void UVFX_SystemManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Only cleanup if enough time has passed
    if (CurrentTime - LastCleanupTime < CleanupInterval)
    {
        return;
    }
    
    LastCleanupTime = CurrentTime;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
        else if (bEnableLODSystem && ShouldCullEffect(Effect))
        {
            StopVFXEffect(Effect);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("VFX System: Cleanup complete, %d active effects remaining"), ActiveEffects.Num());
}

int32 UVFX_SystemManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_SystemManager::SetVFXQualityLevel(int32 QualityLevel)
{
    // Adjust performance settings based on quality level
    switch (QualityLevel)
    {
    case 0: // Low
        MaxActiveEffects = 20;
        CullingDistance = 2000.0f;
        bEnableLODSystem = true;
        break;
    case 1: // Medium
        MaxActiveEffects = 35;
        CullingDistance = 3500.0f;
        bEnableLODSystem = true;
        break;
    case 2: // High
        MaxActiveEffects = 50;
        CullingDistance = 5000.0f;
        bEnableLODSystem = false;
        break;
    case 3: // Ultra
        MaxActiveEffects = 75;
        CullingDistance = 7500.0f;
        bEnableLODSystem = false;
        break;
    default:
        break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX System: Quality level set to %d (Max Effects: %d, Culling: %f)"), 
           QualityLevel, MaxActiveEffects, CullingDistance);
}

FVFX_EffectData* UVFX_SystemManager::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& Data : EffectDatabase)
    {
        if (Data.EffectType == EffectType)
        {
            return &Data;
        }
    }
    return nullptr;
}

void UVFX_SystemManager::RegisterActiveEffect(UNiagaraComponent* Effect)
{
    if (Effect)
    {
        ActiveEffects.AddUnique(Effect);
    }
}

void UVFX_SystemManager::UnregisterActiveEffect(UNiagaraComponent* Effect)
{
    ActiveEffects.Remove(Effect);
}

bool UVFX_SystemManager::ShouldCullEffect(UNiagaraComponent* Effect) const
{
    if (!Effect)
    {
        return true;
    }
    
    // Get player location for distance culling
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Effect->GetComponentLocation());
        return Distance > CullingDistance;
    }
    
    return false;
}