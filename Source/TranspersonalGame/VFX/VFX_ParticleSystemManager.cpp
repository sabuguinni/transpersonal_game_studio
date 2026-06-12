#include "VFX_ParticleSystemManager.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVFX_ParticleSystemManager::AVFX_ParticleSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root mesh component
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMeshComponent"));
    RootComponent = RootMeshComponent;

    // Initialize default particle system data
    ParticleSystems.SetNum(8);
    
    // Campfire
    ParticleSystems[0].ParticleType = EVFX_ParticleType::Campfire;
    ParticleSystems[0].SpawnLocation = FVector(0, 0, 0);
    ParticleSystems[0].bAutoActivate = true;
    ParticleSystems[0].LifeSpan = 0.0f; // Infinite
    
    // Dust Impact
    ParticleSystems[1].ParticleType = EVFX_ParticleType::DustImpact;
    ParticleSystems[1].SpawnLocation = FVector(100, 0, 0);
    ParticleSystems[1].bAutoActivate = false;
    ParticleSystems[1].LifeSpan = 3.0f;
    
    // Blood Impact
    ParticleSystems[2].ParticleType = EVFX_ParticleType::BloodImpact;
    ParticleSystems[2].SpawnLocation = FVector(200, 0, 0);
    ParticleSystems[2].bAutoActivate = false;
    ParticleSystems[2].LifeSpan = 2.0f;
    
    // Rain
    ParticleSystems[3].ParticleType = EVFX_ParticleType::Rain;
    ParticleSystems[3].SpawnLocation = FVector(300, 0, 500);
    ParticleSystems[3].bAutoActivate = false;
    ParticleSystems[3].LifeSpan = 0.0f; // Infinite
    
    // Dinosaur Breath
    ParticleSystems[4].ParticleType = EVFX_ParticleType::DinosaurBreath;
    ParticleSystems[4].SpawnLocation = FVector(400, 0, 150);
    ParticleSystems[4].bAutoActivate = false;
    ParticleSystems[4].LifeSpan = 1.5f;
    
    // Water Splash
    ParticleSystems[5].ParticleType = EVFX_ParticleType::WaterSplash;
    ParticleSystems[5].SpawnLocation = FVector(500, 0, 0);
    ParticleSystems[5].bAutoActivate = false;
    ParticleSystems[5].LifeSpan = 2.5f;
    
    // Volcanic Ash
    ParticleSystems[6].ParticleType = EVFX_ParticleType::VolcanicAsh;
    ParticleSystems[6].SpawnLocation = FVector(600, 0, 300);
    ParticleSystems[6].bAutoActivate = false;
    ParticleSystems[6].LifeSpan = 0.0f; // Infinite
    
    // Insect Swarm
    ParticleSystems[7].ParticleType = EVFX_ParticleType::InsectSwarm;
    ParticleSystems[7].SpawnLocation = FVector(700, 0, 100);
    ParticleSystems[7].bAutoActivate = false;
    ParticleSystems[7].LifeSpan = 10.0f;
}

void AVFX_ParticleSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeParticleSystems();
    
    // Auto-activate systems that should start immediately
    for (const FVFX_ParticleSystemData& SystemData : ParticleSystems)
    {
        if (SystemData.bAutoActivate)
        {
            SpawnParticleEffect(SystemData.ParticleType, SystemData.SpawnLocation, SystemData.SpawnRotation);
        }
    }
}

void AVFX_ParticleSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up inactive components periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    
    if (CleanupTimer >= 5.0f) // Clean up every 5 seconds
    {
        CleanupInactiveComponents();
        CleanupTimer = 0.0f;
    }
}

void AVFX_ParticleSystemManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation)
{
    // Find the particle system data for this type
    const FVFX_ParticleSystemData* SystemData = nullptr;
    for (const FVFX_ParticleSystemData& Data : ParticleSystems)
    {
        if (Data.ParticleType == ParticleType)
        {
            SystemData = &Data;
            break;
        }
    }
    
    if (!SystemData || !SystemData->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: No valid Niagara system found for particle type"));
        return;
    }
    
    // Load the Niagara system if it's not already loaded
    UNiagaraSystem* LoadedSystem = SystemData->NiagaraSystem.LoadSynchronous();
    if (!LoadedSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleSystemManager: Failed to load Niagara system"));
        return;
    }
    
    // Create and configure the Niagara component
    UNiagaraComponent* NiagaraComp = CreateNiagaraComponent(LoadedSystem);
    if (NiagaraComp)
    {
        NiagaraComp->SetWorldLocationAndRotation(Location, Rotation);
        NiagaraComp->Activate();
        
        ActiveNiagaraComponents.Add(NiagaraComp);
        
        // Set up auto-destroy timer if lifespan is specified
        if (SystemData->LifeSpan > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                }
            }, SystemData->LifeSpan, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Spawned particle effect at location %s"), *Location.ToString());
    }
}

void AVFX_ParticleSystemManager::StopParticleEffect(EVFX_ParticleType ParticleType)
{
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Component = ActiveNiagaraComponents[i];
        if (IsValid(Component))
        {
            // Note: In a full implementation, we'd need to track which component corresponds to which particle type
            // For now, we'll deactivate all components of this type
            Component->Deactivate();
            Component->DestroyComponent();
            ActiveNiagaraComponents.RemoveAt(i);
        }
    }
}

void AVFX_ParticleSystemManager::StopAllParticleEffects()
{
    for (UNiagaraComponent* Component : ActiveNiagaraComponents)
    {
        if (IsValid(Component))
        {
            Component->Deactivate();
            Component->DestroyComponent();
        }
    }
    
    ActiveNiagaraComponents.Empty();
}

bool AVFX_ParticleSystemManager::IsParticleEffectActive(EVFX_ParticleType ParticleType) const
{
    // In a full implementation, we'd track which components correspond to which particle types
    return ActiveNiagaraComponents.Num() > 0;
}

void AVFX_ParticleSystemManager::SetParticleSystemData(const TArray<FVFX_ParticleSystemData>& NewParticleSystemData)
{
    ParticleSystems = NewParticleSystemData;
}

TArray<FVFX_ParticleSystemData> AVFX_ParticleSystemManager::GetParticleSystemData() const
{
    return ParticleSystems;
}

void AVFX_ParticleSystemManager::InitializeParticleSystems()
{
    // In a full implementation, this would load all the Niagara systems
    // For now, we'll just log that initialization is complete
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleSystemManager: Initialized %d particle systems"), ParticleSystems.Num());
}

UNiagaraComponent* AVFX_ParticleSystemManager::CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem)
{
    if (!NiagaraSystem)
    {
        return nullptr;
    }
    
    UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(this);
    NiagaraComp->SetAsset(NiagaraSystem);
    NiagaraComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    
    return NiagaraComp;
}

void AVFX_ParticleSystemManager::CleanupInactiveComponents()
{
    for (int32 i = ActiveNiagaraComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Component = ActiveNiagaraComponents[i];
        if (!IsValid(Component) || !Component->IsActive())
        {
            if (IsValid(Component))
            {
                Component->DestroyComponent();
            }
            ActiveNiagaraComponents.RemoveAt(i);
        }
    }
}