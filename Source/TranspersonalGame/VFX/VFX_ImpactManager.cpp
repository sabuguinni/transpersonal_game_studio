#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 0.1 segundos

    ParticleLifetime = 3.0f;
    MaxParticleDistance = 5000.0f;
    MaxActiveParticles = 50;
    CurrentActiveParticles = 0;
    LastCleanupTime = 0.0f;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Sistema de VFX iniciado"));
    
    // Reservar espaço para array de partículas
    ActiveParticleActors.Reserve(MaxActiveParticles);
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Limpeza periódica de partículas antigas (a cada 2 segundos)
    LastCleanupTime += DeltaTime;
    if (LastCleanupTime >= 2.0f)
    {
        CleanupOldParticles();
        LastCleanupTime = 0.0f;
    }
}

void UVFX_ImpactManager::TriggerImpact(const FVFX_ImpactData& ImpactData)
{
    if (!IsLocationValid(ImpactData.ImpactLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Localização inválida para VFX"));
        return;
    }

    if (CurrentActiveParticles >= MaxActiveParticles)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Máximo de partículas atingido"));
        CleanupOldParticles();
        return;
    }

    SpawnParticleSystem(ImpactData.ImpactLocation, ImpactData.ImpactType);
}

void UVFX_ImpactManager::CreateFootstepVFX(FVector Location, float DinosaurSize)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::DinosaurFootstep;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactForce = DinosaurSize * 2.0f; // Força baseada no tamanho
    ImpactData.ImpactNormal = FVector::UpVector;

    TriggerImpact(ImpactData);
}

void UVFX_ImpactManager::CreateBloodVFX(FVector Location, FVector Direction)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::BloodSplatter;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactForce = 1.5f;
    ImpactData.ImpactNormal = Direction.GetSafeNormal();

    TriggerImpact(ImpactData);
}

void UVFX_ImpactManager::CreateDustCloudVFX(FVector Location, float Intensity)
{
    FVFX_ImpactData ImpactData;
    ImpactData.ImpactType = EVFX_ImpactType::DustCloud;
    ImpactData.ImpactLocation = Location;
    ImpactData.ImpactForce = Intensity;
    ImpactData.ImpactNormal = FVector::UpVector;

    TriggerImpact(ImpactData);
}

void UVFX_ImpactManager::SpawnParticleSystem(FVector Location, EVFX_ImpactType Type)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Por agora, usar StaticMeshActor como placeholder visual
    // Em produção, isto seria substituído por NiagaraActor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* ParticleActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    
    if (ParticleActor)
    {
        // Configurar o placeholder baseado no tipo
        FString ActorName;
        switch (Type)
        {
            case EVFX_ImpactType::DinosaurFootstep:
                ActorName = TEXT("VFX_Footstep_Particle");
                break;
            case EVFX_ImpactType::BloodSplatter:
                ActorName = TEXT("VFX_Blood_Particle");
                break;
            case EVFX_ImpactType::DustCloud:
                ActorName = TEXT("VFX_Dust_Particle");
                break;
            default:
                ActorName = TEXT("VFX_Generic_Particle");
                break;
        }

        ParticleActor->SetActorLabel(ActorName);
        ParticleActor->Tags.Add(TEXT("VFXParticle"));
        
        // Adicionar ao array de partículas activas
        ActiveParticleActors.Add(ParticleActor);
        CurrentActiveParticles++;

        // Programar destruição automática
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this, ParticleActor]()
        {
            if (IsValid(ParticleActor))
            {
                ActiveParticleActors.Remove(ParticleActor);
                CurrentActiveParticles--;
                ParticleActor->Destroy();
            }
        }, ParticleLifetime, false);

        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Criada partícula %s em %s"), *ActorName, *Location.ToString());
    }
}

void UVFX_ImpactManager::CleanupOldParticles()
{
    // Remover partículas inválidas do array
    ActiveParticleActors.RemoveAll([](AActor* Actor)
    {
        return !IsValid(Actor);
    });

    // Actualizar contador
    CurrentActiveParticles = ActiveParticleActors.Num();

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Limpeza concluída. Partículas activas: %d"), CurrentActiveParticles);
}

bool UVFX_ImpactManager::IsLocationValid(FVector Location) const
{
    // Verificar se a localização está dentro dos limites do mapa
    // Mapa: 157.000 x 153.000 UU, centrado em (0,0,0)
    const float MapHalfX = 78500.0f;  // 157.000 / 2
    const float MapHalfY = 76500.0f;  // 153.000 / 2

    if (FMath::Abs(Location.X) > MapHalfX || FMath::Abs(Location.Y) > MapHalfY)
    {
        return false;
    }

    // Verificar se não está muito alto ou muito baixo
    if (Location.Z < -1000.0f || Location.Z > 5000.0f)
    {
        return false;
    }

    return true;
}