#include "VFX_CretaceousParticleController.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

UVFX_CretaceousParticleController::UVFX_CretaceousParticleController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize component pointers
    DustComponent = nullptr;
    AshComponent = nullptr;
    PollenComponent = nullptr;
    InsectComponent = nullptr;

    // Set default environmental parameters for Cretaceous period
    WindStrength = 0.3f;
    Humidity = 0.7f;
    Temperature = 28.0f;
    VolcanicActivity = 0.2f;
}

void UVFX_CretaceousParticleController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeParticleSystems();
    
    // Start default environmental effects
    StartAtmosphericDust();
    StartPollenDrift();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Initialized Cretaceous period environmental VFX"));
}

void UVFX_CretaceousParticleController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateParticleParameters();
}

void UVFX_CretaceousParticleController::InitializeParticleSystems()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_CretaceousParticleController: No owner actor found"));
        return;
    }

    // Create Niagara components for each particle system
    if (AtmosphericDustSystem)
    {
        CreateNiagaraComponent(AtmosphericDustSystem, DustComponent, TEXT("DustComponent"));
    }

    if (VolcanicAshSystem)
    {
        CreateNiagaraComponent(VolcanicAshSystem, AshComponent, TEXT("AshComponent"));
    }

    if (PollenDriftSystem)
    {
        CreateNiagaraComponent(PollenDriftSystem, PollenComponent, TEXT("PollenComponent"));
    }

    if (InsectSwarmSystem)
    {
        CreateNiagaraComponent(InsectSwarmSystem, InsectComponent, TEXT("InsectComponent"));
    }
}

void UVFX_CretaceousParticleController::CreateNiagaraComponent(UNiagaraSystem* System, UNiagaraComponent*& Component, const FString& ComponentName)
{
    if (!System || !GetOwner())
    {
        return;
    }

    Component = NewObject<UNiagaraComponent>(GetOwner(), *ComponentName);
    if (Component)
    {
        Component->SetAsset(System);
        Component->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        Component->RegisterComponent();
        Component->SetAutoDestroy(false);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Created %s"), *ComponentName);
    }
}

void UVFX_CretaceousParticleController::UpdateParticleParameters()
{
    // Update dust particles based on wind strength
    if (DustComponent && DustComponent->IsActive())
    {
        DustComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength);
        DustComponent->SetFloatParameter(TEXT("Humidity"), Humidity);
    }

    // Update volcanic ash based on activity level
    if (AshComponent && AshComponent->IsActive())
    {
        DustComponent->SetFloatParameter(TEXT("VolcanicActivity"), VolcanicActivity);
        DustComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength * 0.5f);
    }

    // Update pollen drift based on temperature and humidity
    if (PollenComponent && PollenComponent->IsActive())
    {
        PollenComponent->SetFloatParameter(TEXT("Temperature"), Temperature);
        PollenComponent->SetFloatParameter(TEXT("Humidity"), Humidity);
        PollenComponent->SetFloatParameter(TEXT("WindStrength"), WindStrength * 0.3f);
    }

    // Update insect swarms based on temperature
    if (InsectComponent && InsectComponent->IsActive())
    {
        InsectComponent->SetFloatParameter(TEXT("Temperature"), Temperature);
        InsectComponent->SetFloatParameter(TEXT("Humidity"), Humidity);
    }
}

void UVFX_CretaceousParticleController::StartAtmosphericDust()
{
    if (DustComponent)
    {
        DustComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Started atmospheric dust"));
    }
}

void UVFX_CretaceousParticleController::StopAtmosphericDust()
{
    if (DustComponent)
    {
        DustComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Stopped atmospheric dust"));
    }
}

void UVFX_CretaceousParticleController::StartVolcanicAsh()
{
    if (AshComponent)
    {
        AshComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Started volcanic ash"));
    }
}

void UVFX_CretaceousParticleController::StopVolcanicAsh()
{
    if (AshComponent)
    {
        AshComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Stopped volcanic ash"));
    }
}

void UVFX_CretaceousParticleController::StartPollenDrift()
{
    if (PollenComponent)
    {
        PollenComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Started pollen drift"));
    }
}

void UVFX_CretaceousParticleController::StopPollenDrift()
{
    if (PollenComponent)
    {
        PollenComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Stopped pollen drift"));
    }
}

void UVFX_CretaceousParticleController::StartInsectSwarm()
{
    if (InsectComponent)
    {
        InsectComponent->Activate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Started insect swarm"));
    }
}

void UVFX_CretaceousParticleController::StopInsectSwarm()
{
    if (InsectComponent)
    {
        InsectComponent->Deactivate();
        UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Stopped insect swarm"));
    }
}

void UVFX_CretaceousParticleController::UpdateEnvironmentalParameters(float NewWindStrength, float NewHumidity, float NewTemperature)
{
    WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 1.0f);
    Humidity = FMath::Clamp(NewHumidity, 0.0f, 1.0f);
    Temperature = FMath::Clamp(NewTemperature, 15.0f, 45.0f);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Updated environmental parameters - Wind: %f, Humidity: %f, Temperature: %f"), 
           WindStrength, Humidity, Temperature);
}

void UVFX_CretaceousParticleController::SetVolcanicActivity(float ActivityLevel)
{
    VolcanicActivity = FMath::Clamp(ActivityLevel, 0.0f, 1.0f);
    
    // Start or stop volcanic ash based on activity level
    if (VolcanicActivity > 0.1f)
    {
        StartVolcanicAsh();
    }
    else
    {
        StopVolcanicAsh();
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_CretaceousParticleController: Set volcanic activity to %f"), VolcanicActivity);
}