// VFXNiagaraController.cpp
// Integration Agent #19 — Cycle 005
// Stub implementation to unblock compilation (QA flag: missing .cpp for VFXNiagaraController.h)

#include "VFXNiagaraController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UVFXNiagaraController::UVFXNiagaraController()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
}

void UVFXNiagaraController::BeginPlay()
{
    Super::BeginPlay();
}

void UVFXNiagaraController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVFXNiagaraController::SpawnVFXAtLocation(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation)
{
    if (!NiagaraSystem)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Location,
        Rotation,
        FVector(1.0f),
        true,   // bAutoDestroy
        true,   // bAutoActivate
        ENCPoolMethod::None
    );
}

void UVFXNiagaraController::SpawnVFXAttached(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent, FName AttachPointName)
{
    if (!NiagaraSystem || !AttachToComponent)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        AttachToComponent,
        AttachPointName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true    // bAutoDestroy
    );
}

void UVFXNiagaraController::StopAllVFX()
{
    // Deactivate all Niagara components on the owning actor
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UNiagaraComponent*> NiagaraComponents;
    Owner->GetComponents<UNiagaraComponent>(NiagaraComponents);
    for (UNiagaraComponent* Comp : NiagaraComponents)
    {
        if (Comp)
        {
            Comp->Deactivate();
        }
    }
}
