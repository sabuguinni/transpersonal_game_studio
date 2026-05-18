#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Tick every 0.5 seconds for cleanup
    MaxActiveEffects = 50;
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultVFX();
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupExpiredEffects();
}

void UVFX_SystemManager::InitializeDefaultVFX()
{
    // Initialize default VFX data for prehistoric effects
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Blood;
    BloodData.Scale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 3.0f;
    BloodData.bAutoDestroy = true;
    VFXDatabase.Add(EVFX_EffectType::Blood, BloodData);

    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::Dust;
    DustData.Scale = FVector(1.5f, 1.5f, 1.0f);
    DustData.Duration = 2.0f;
    DustData.bAutoDestroy = true;
    VFXDatabase.Add(EVFX_EffectType::Dust, DustData);

    FVFX_EffectData FireData;
    FireData.EffectType = EVFX_EffectType::Fire;
    FireData.Scale = FVector(1.0f, 1.0f, 1.2f);
    FireData.Duration = -1.0f; // Persistent fire
    FireData.bAutoDestroy = false;
    VFXDatabase.Add(EVFX_EffectType::Fire, FireData);

    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Footstep;
    FootstepData.Scale = FVector(2.0f, 2.0f, 1.0f);
    FootstepData.Duration = 1.5f;
    FootstepData.bAutoDestroy = true;
    VFXDatabase.Add(EVFX_EffectType::Footstep, FootstepData);

    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized with %d effect types"), VFXDatabase.Num());
}

void UVFX_SystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation)
{
    if (!VFXDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Effect type not found in database: %d"), (int32)EffectType);
        return;
    }

    const FVFX_EffectData& EffectData = VFXDatabase[EffectType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No Niagara system assigned for effect type: %d"), (int32)EffectType);
        return;
    }

    UNiagaraSystem* System = EffectData.NiagaraSystem.LoadSynchronous();
    if (System)
    {
        UNiagaraComponent* Effect = CreateNiagaraEffect(System, Location, Rotation);
        if (Effect)
        {
            Effect->SetWorldScale3D(EffectData.Scale);
            
            if (EffectData.bAutoDestroy && EffectData.Duration > 0.0f)
            {
                // Auto-destroy after duration
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Effect]()
                {
                    if (IsValid(Effect))
                    {
                        Effect->DestroyComponent();
                    }
                }, EffectData.Duration, false);
            }
        }
    }
}

void UVFX_SystemManager::SpawnBloodSplatter(const FVector& Location, const FVector& Direction)
{
    // Calculate rotation from direction
    FRotator Rotation = Direction.Rotation();
    
    // Add some randomness to blood splatter
    Rotation.Yaw += FMath::RandRange(-30.0f, 30.0f);
    Rotation.Pitch += FMath::RandRange(-15.0f, 15.0f);
    
    SpawnVFXEffect(EVFX_EffectType::Blood, Location, Rotation);
    
    UE_LOG(LogTemp, Log, TEXT("Blood splatter spawned at location: %s"), *Location.ToString());
}

void UVFX_SystemManager::SpawnDustCloud(const FVector& Location, float Intensity)
{
    FVector ScaledLocation = Location;
    ScaledLocation.Z += 10.0f; // Slightly above ground
    
    SpawnVFXEffect(EVFX_EffectType::Dust, ScaledLocation);
    
    // If high intensity, spawn additional dust particles
    if (Intensity > 1.5f)
    {
        for (int32 i = 0; i < 3; i++)
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                0.0f
            );
            SpawnVFXEffect(EVFX_EffectType::Dust, ScaledLocation + RandomOffset);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dust cloud spawned with intensity: %f"), Intensity);
}

void UVFX_SystemManager::SpawnFootstepDust(const FVector& Location, float DinosaurSize)
{
    FVector GroundLocation = Location;
    GroundLocation.Z -= 50.0f; // At ground level
    
    // Scale effect based on dinosaur size
    if (FVFX_EffectData* FootstepData = VFXDatabase.Find(EVFX_EffectType::Footstep))
    {
        FVector OriginalScale = FootstepData->Scale;
        FootstepData->Scale = OriginalScale * DinosaurSize;
        
        SpawnVFXEffect(EVFX_EffectType::Footstep, GroundLocation);
        
        // Restore original scale
        FootstepData->Scale = OriginalScale;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Footstep dust spawned for dinosaur size: %f"), DinosaurSize);
}

void UVFX_SystemManager::SpawnCampfire(const FVector& Location)
{
    FVector FireLocation = Location;
    FireLocation.Z += 20.0f; // Slightly above ground for fire
    
    SpawnVFXEffect(EVFX_EffectType::Fire, FireLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Campfire VFX spawned at: %s"), *FireLocation.ToString());
}

void UVFX_SystemManager::RegisterVFXEffect(EVFX_EffectType EffectType, UNiagaraSystem* NiagaraSystem)
{
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null Niagara system"));
        return;
    }
    
    if (FVFX_EffectData* ExistingData = VFXDatabase.Find(EffectType))
    {
        ExistingData->NiagaraSystem = NiagaraSystem;
        UE_LOG(LogTemp, Log, TEXT("Updated VFX effect registration for type: %d"), (int32)EffectType);
    }
    else
    {
        FVFX_EffectData NewData;
        NewData.EffectType = EffectType;
        NewData.NiagaraSystem = NiagaraSystem;
        VFXDatabase.Add(EffectType, NewData);
        UE_LOG(LogTemp, Log, TEXT("Registered new VFX effect for type: %d"), (int32)EffectType);
    }
}

UNiagaraComponent* UVFX_SystemManager::CreateNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }
    
    // Limit active effects to prevent performance issues
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max active VFX effects reached (%d), skipping new effect"), MaxActiveEffects);
        return nullptr;
    }
    
    UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(1.0f),
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );
    
    if (Effect)
    {
        ActiveEffects.Add(Effect);
        UE_LOG(LogTemp, Log, TEXT("Created Niagara effect at location: %s"), *Location.ToString());
    }
    
    return Effect;
}

void UVFX_SystemManager::CleanupExpiredEffects()
{
    // Remove null or destroyed components from active effects list
    ActiveEffects.RemoveAll([](const UNiagaraComponent* Effect)
    {
        return !IsValid(Effect) || Effect->IsBeingDestroyed();
    });
    
    // Log cleanup if we removed any effects
    static int32 LastCount = 0;
    if (ActiveEffects.Num() != LastCount)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Active VFX effects: %d"), ActiveEffects.Num());
        LastCount = ActiveEffects.Num();
    }
}