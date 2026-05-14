#include "VFX_ParticleSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ParticleSystemManager::UVFX_ParticleSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for cleanup
}

void UVFX_ParticleSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeParticleSystemMap();
}

void UVFX_ParticleSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupFinishedEffects();
}

void UVFX_ParticleSystemManager::InitializeParticleSystemMap()
{
    // Initialize particle system paths for realistic prehistoric effects
    ParticleSystemMap.Empty();
    
    // Note: These would be actual Niagara system paths in a real project
    // For now, we set up the structure for future asset integration
    ParticleSystemMap.Add(EVFX_ParticleType::Fire_Campfire, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Fire_Campfire"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Dust_Footstep, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Dust_DinosaurFootstep"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Blood_Impact, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Blood_Impact"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Water_Splash, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Water_Splash"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Rain_Weather, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Weather_Rain"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Smoke_Fire, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Smoke_Fire"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Sparks_Crafting, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Sparks_Crafting"))));
    
    ParticleSystemMap.Add(EVFX_ParticleType::Breath_Cold, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_Breath_Cold"))));

    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: Initialized %d particle system mappings"), ParticleSystemMap.Num());
}

UNiagaraComponent* UVFX_ParticleSystemManager::SpawnParticleEffect(const FVFX_ParticleConfig& Config)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleSystemManager: No valid world for particle spawn"));
        return nullptr;
    }

    UNiagaraSystem* ParticleSystem = GetParticleSystemForType(Config.ParticleType);
    if (!ParticleSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: No particle system found for type %d"), (int32)Config.ParticleType);
        return nullptr;
    }

    // Spawn Niagara component at world location
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ParticleSystem,
        Config.SpawnLocation,
        Config.SpawnRotation,
        FVector(Config.Scale),
        Config.bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveParticleComponents.Add(NiagaraComp);
        
        // Set lifetime if not auto-destroying
        if (!Config.bAutoDestroy && Config.LifeTime > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                    ActiveParticleComponents.Remove(NiagaraComp);
                }
            }, Config.LifeTime, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned particle effect type %d at location %s"), 
            (int32)Config.ParticleType, *Config.SpawnLocation.ToString());
    }

    return NiagaraComp;
}

UNiagaraComponent* UVFX_ParticleSystemManager::SpawnParticleEffectAttached(const FVFX_ParticleConfig& Config, AActor* AttachActor, FName SocketName)
{
    if (!AttachActor || !GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleSystemManager: Invalid attach actor or world"));
        return nullptr;
    }

    UNiagaraSystem* ParticleSystem = GetParticleSystemForType(Config.ParticleType);
    if (!ParticleSystem)
    {
        return nullptr;
    }

    // Spawn attached to actor
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
        ParticleSystem,
        AttachActor->GetRootComponent(),
        SocketName,
        Config.SpawnLocation,
        Config.SpawnRotation,
        FVector(Config.Scale),
        EAttachLocation::KeepRelativeOffset,
        Config.bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveParticleComponents.Add(NiagaraComp);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned attached particle effect type %d"), (int32)Config.ParticleType);
    }

    return NiagaraComp;
}

void UVFX_ParticleSystemManager::StopAllParticleEffects()
{
    for (UNiagaraComponent* NiagaraComp : ActiveParticleComponents)
    {
        if (IsValid(NiagaraComp))
        {
            NiagaraComp->DestroyComponent();
        }
    }
    ActiveParticleComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Stopped all active particle effects"));
}

UNiagaraSystem* UVFX_ParticleSystemManager::GetParticleSystemForType(EVFX_ParticleType ParticleType)
{
    if (TSoftObjectPtr<UNiagaraSystem>* SystemPtr = ParticleSystemMap.Find(ParticleType))
    {
        // Try to load the asset if it's not already loaded
        if (!SystemPtr->IsValid())
        {
            UNiagaraSystem* LoadedSystem = SystemPtr->LoadSynchronous();
            if (LoadedSystem)
            {
                return LoadedSystem;
            }
        }
        else
        {
            return SystemPtr->Get();
        }
    }

    // Return nullptr if no system found - in real project, we'd have default fallback systems
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: No particle system asset found for type %d"), (int32)ParticleType);
    return nullptr;
}

void UVFX_ParticleSystemManager::SpawnDinosaurFootstepEffect(FVector FootLocation, float DinosaurSize)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::Dust_Footstep;
    Config.SpawnLocation = FootLocation;
    Config.SpawnRotation = FRotator::ZeroRotator;
    Config.Scale = FMath::Clamp(DinosaurSize, 0.5f, 5.0f); // Scale based on dinosaur size
    Config.LifeTime = 3.0f;
    Config.bAutoDestroy = true;

    SpawnParticleEffect(Config);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned dinosaur footstep effect at %s, scale %f"), 
        *FootLocation.ToString(), Config.Scale);
}

UNiagaraComponent* UVFX_ParticleSystemManager::SpawnCampfireEffect(FVector FireLocation)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::Fire_Campfire;
    Config.SpawnLocation = FireLocation;
    Config.SpawnRotation = FRotator::ZeroRotator;
    Config.Scale = 1.0f;
    Config.LifeTime = 0.0f; // Infinite - campfire burns until manually stopped
    Config.bAutoDestroy = false;

    UNiagaraComponent* FireEffect = SpawnParticleEffect(Config);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned campfire effect at %s"), *FireLocation.ToString());
    return FireEffect;
}

void UVFX_ParticleSystemManager::SpawnBloodImpactEffect(FVector ImpactLocation, FVector ImpactNormal)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::Blood_Impact;
    Config.SpawnLocation = ImpactLocation;
    Config.SpawnRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    Config.Scale = 1.0f;
    Config.LifeTime = 2.0f;
    Config.bAutoDestroy = true;

    SpawnParticleEffect(Config);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned blood impact effect at %s"), *ImpactLocation.ToString());
}

void UVFX_ParticleSystemManager::CleanupFinishedEffects()
{
    // Remove invalid or destroyed components from active list
    ActiveParticleComponents.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });
}