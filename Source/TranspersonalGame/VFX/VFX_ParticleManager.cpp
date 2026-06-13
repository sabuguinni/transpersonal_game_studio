#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bAutoStartParticles = true;
    GlobalScale = 1.0f;
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoStartParticles)
    {
        InitializeDefaultParticles();
    }
}

void AVFX_ParticleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up finished particle components
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveParticleComponents[i]) || !ActiveParticleComponents[i]->IsActive())
        {
            ActiveParticleComponents.RemoveAt(i);
        }
    }
}

void AVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, float Scale)
{
    UNiagaraSystem* System = GetNiagaraSystemForType(ParticleType);
    if (System)
    {
        UNiagaraComponent* Component = CreateNiagaraComponent(System, Location, Scale * GlobalScale);
        if (Component)
        {
            ActiveParticleComponents.Add(Component);
            Component->Activate();
        }
    }
}

void AVFX_ParticleManager::StopAllParticles()
{
    for (UNiagaraComponent* Component : ActiveParticleComponents)
    {
        if (IsValid(Component))
        {
            Component->Deactivate();
        }
    }
}

void AVFX_ParticleManager::StartAllParticles()
{
    for (UNiagaraComponent* Component : ActiveParticleComponents)
    {
        if (IsValid(Component))
        {
            Component->Activate();
        }
    }
}

UNiagaraComponent* AVFX_ParticleManager::CreateNiagaraComponent(UNiagaraSystem* System, FVector Location, float Scale)
{
    if (!System)
    {
        return nullptr;
    }

    UNiagaraComponent* Component = NewObject<UNiagaraComponent>(this);
    if (Component)
    {
        Component->SetAsset(System);
        Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        Component->SetWorldLocation(Location);
        Component->SetWorldScale3D(FVector(Scale));
        Component->RegisterComponent();
    }

    return Component;
}

void AVFX_ParticleManager::InitializeDefaultParticles()
{
    // Spawn default particles based on configurations
    for (const FVFX_ParticleConfig& Config : ParticleConfigs)
    {
        if (Config.NiagaraSystem.IsValid())
        {
            SpawnParticleEffect(Config.ParticleType, Config.SpawnLocation, Config.Scale);
        }
    }
}

UNiagaraSystem* AVFX_ParticleManager::GetNiagaraSystemForType(EVFX_ParticleType ParticleType)
{
    // Find matching configuration
    for (const FVFX_ParticleConfig& Config : ParticleConfigs)
    {
        if (Config.ParticleType == ParticleType && Config.NiagaraSystem.IsValid())
        {
            return Config.NiagaraSystem.LoadSynchronous();
        }
    }

    // Return null if no matching system found
    return nullptr;
}