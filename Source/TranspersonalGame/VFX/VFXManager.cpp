#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update LOD 10 times per second

    // Set default VFX library entries
    FVFX_EffectData FootstepData;
    FootstepData.Duration = 2.0f;
    FootstepData.Scale = FVector(1.0f, 1.0f, 0.5f);
    VFXLibrary.Add(EDir_VFXType::Footstep, FootstepData);

    FVFX_EffectData CampfireData;
    CampfireData.Duration = -1.0f; // Infinite duration
    CampfireData.bAutoDestroy = false;
    CampfireData.Scale = FVector(1.5f, 1.5f, 2.0f);
    VFXLibrary.Add(EDir_VFXType::Fire, CampfireData);

    FVFX_EffectData BloodData;
    BloodData.Duration = 3.0f;
    BloodData.Scale = FVector(0.8f, 0.8f, 0.8f);
    VFXLibrary.Add(EDir_VFXType::Blood, BloodData);

    FVFX_EffectData DustData;
    DustData.Duration = 1.5f;
    DustData.Scale = FVector(2.0f, 2.0f, 1.0f);
    VFXLibrary.Add(EDir_VFXType::Dust, DustData);
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager initialized with %d effect types"), VFXLibrary.Num());
}

void AVFX_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clean up finished effects
    CleanupFinishedEffects();

    // Update LOD based on player location
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            UpdateLOD(PlayerPawn->GetActorLocation());
        }
    }
}

UNiagaraComponent* AVFX_Manager::SpawnVFXAtLocation(EDir_VFXType VFXType, FVector Location, FRotator Rotation)
{
    if (!VFXLibrary.Contains(VFXType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX type not found in library: %d"), (int32)VFXType);
        return nullptr;
    }

    // Check if we've reached the maximum number of active effects
    if (ActiveEffects.Num() >= LODSettings.MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active VFX effects reached (%d)"), LODSettings.MaxActiveEffects);
        return nullptr;
    }

    const FVFX_EffectData& EffectData = VFXLibrary[VFXType];
    
    if (!EffectData.NiagaraSystem.IsValid())
    {
        // Try to load the system if it's not loaded
        UNiagaraSystem* LoadedSystem = EffectData.NiagaraSystem.LoadSynchronous();
        if (!LoadedSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara system for VFX type: %d"), (int32)VFXType);
            return nullptr;
        }
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem.Get(),
        Location,
        Rotation,
        EffectData.Scale,
        EffectData.bAutoDestroy
    );

    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        
        // Set duration if specified
        if (EffectData.Duration > 0.0f)
        {
            VFXComponent->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
        }

        UE_LOG(LogTemp, Log, TEXT("Spawned VFX at location: %s"), *Location.ToString());
    }

    return VFXComponent;
}

UNiagaraComponent* AVFX_Manager::SpawnVFXAttached(EDir_VFXType VFXType, USceneComponent* AttachComponent, FName SocketName)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn attached VFX: AttachComponent is null"));
        return nullptr;
    }

    if (!VFXLibrary.Contains(VFXType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX type not found in library: %d"), (int32)VFXType);
        return nullptr;
    }

    // Check active effects limit
    if (ActiveEffects.Num() >= LODSettings.MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active VFX effects reached (%d)"), LODSettings.MaxActiveEffects);
        return nullptr;
    }

    const FVFX_EffectData& EffectData = VFXLibrary[VFXType];
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData.NiagaraSystem.Get(),
        AttachComponent,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EffectData.Scale,
        EAttachLocation::KeepRelativeOffset,
        EffectData.bAutoDestroy
    );

    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        
        if (EffectData.Duration > 0.0f)
        {
            VFXComponent->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
        }

        UE_LOG(LogTemp, Log, TEXT("Spawned attached VFX on component: %s"), *AttachComponent->GetName());
    }

    return VFXComponent;
}

void AVFX_Manager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent)
    {
        return;
    }

    VFXComponent->Deactivate();
    ActiveEffects.Remove(VFXComponent);
    
    UE_LOG(LogTemp, Log, TEXT("Stopped VFX component"));
}

void AVFX_Manager::StopAllVFX()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("Stopped all VFX effects"));
}

int32 AVFX_Manager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_Manager::UpdateLOD(FVector ViewerLocation)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }

        float Distance = CalculateDistanceToViewer(Effect->GetComponentLocation());
        ApplyLODToEffect(Effect, Distance);
    }
}

void AVFX_Manager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

float AVFX_Manager::CalculateDistanceToViewer(FVector EffectLocation) const
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            return FVector::Dist(PlayerPawn->GetActorLocation(), EffectLocation);
        }
    }
    
    return 0.0f;
}

void AVFX_Manager::ApplyLODToEffect(UNiagaraComponent* Effect, float Distance)
{
    if (!Effect)
    {
        return;
    }

    // Apply LOD based on distance
    if (Distance <= LODSettings.HighQualityDistance)
    {
        // High quality - full particles
        Effect->SetFloatParameter(TEXT("ParticleCount"), 1.0f);
        Effect->SetFloatParameter(TEXT("QualityLevel"), 3.0f);
    }
    else if (Distance <= LODSettings.MediumQualityDistance)
    {
        // Medium quality - reduced particles
        Effect->SetFloatParameter(TEXT("ParticleCount"), 0.6f);
        Effect->SetFloatParameter(TEXT("QualityLevel"), 2.0f);
    }
    else if (Distance <= LODSettings.LowQualityDistance)
    {
        // Low quality - minimal particles
        Effect->SetFloatParameter(TEXT("ParticleCount"), 0.3f);
        Effect->SetFloatParameter(TEXT("QualityLevel"), 1.0f);
    }
    else
    {
        // Too far - deactivate
        Effect->Deactivate();
    }
}