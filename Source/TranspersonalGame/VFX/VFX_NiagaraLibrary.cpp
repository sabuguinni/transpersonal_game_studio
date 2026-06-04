#include "VFX_NiagaraLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"

DEFINE_LOG_CATEGORY(LogVFXNiagara);

// Static asset references
TSoftObjectPtr<UNiagaraSystem> UVFX_NiagaraLibrary::CampfireVFX = nullptr;
TSoftObjectPtr<UNiagaraSystem> UVFX_NiagaraLibrary::RainVFX = nullptr;
TSoftObjectPtr<UNiagaraSystem> UVFX_NiagaraLibrary::FootstepDustVFX = nullptr;
TSoftObjectPtr<UNiagaraSystem> UVFX_NiagaraLibrary::BloodImpactVFX = nullptr;
TSoftObjectPtr<UNiagaraSystem> UVFX_NiagaraLibrary::SparksVFX = nullptr;

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    // Initialize asset references
    CampfireVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Environment/NS_Fire_Campfire")));
    RainVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Weather/NS_Weather_Rain")));
    FootstepDustVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaur/NS_Dino_FootstepDust")));
    BloodImpactVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Combat/NS_Combat_BloodImpact")));
    SparksVFX = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Crafting/NS_Craft_Sparks")));
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetCampfireVFX()
{
    if (CampfireVFX.IsValid())
    {
        return CampfireVFX.LoadSynchronous();
    }
    
    UE_LOG(LogVFXNiagara, Warning, TEXT("Campfire VFX asset not found, using fallback"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetRainVFX()
{
    if (RainVFX.IsValid())
    {
        return RainVFX.LoadSynchronous();
    }
    
    UE_LOG(LogVFXNiagara, Warning, TEXT("Rain VFX asset not found"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetFogVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Weather/NS_Weather_Fog
    UE_LOG(LogVFXNiagara, Log, TEXT("Fog VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetWindVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Weather/NS_Weather_Wind
    UE_LOG(LogVFXNiagara, Log, TEXT("Wind VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetFootstepDustVFX()
{
    if (FootstepDustVFX.IsValid())
    {
        return FootstepDustVFX.LoadSynchronous();
    }
    
    UE_LOG(LogVFXNiagara, Warning, TEXT("Footstep dust VFX asset not found"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetBreathVaporVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Dinosaur/NS_Dino_BreathVapor
    UE_LOG(LogVFXNiagara, Log, TEXT("Breath vapor VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetBloodImpactVFX()
{
    if (BloodImpactVFX.IsValid())
    {
        return BloodImpactVFX.LoadSynchronous();
    }
    
    UE_LOG(LogVFXNiagara, Warning, TEXT("Blood impact VFX asset not found"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetRoarDistortionVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Dinosaur/NS_Dino_RoarDistortion
    UE_LOG(LogVFXNiagara, Log, TEXT("Roar distortion VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetSpearImpactVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Combat/NS_Combat_SpearImpact
    UE_LOG(LogVFXNiagara, Log, TEXT("Spear impact VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetStoneImpactVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Combat/NS_Combat_StoneImpact
    UE_LOG(LogVFXNiagara, Log, TEXT("Stone impact VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetWoundBleedingVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Combat/NS_Combat_WoundBleeding
    UE_LOG(LogVFXNiagara, Log, TEXT("Wound bleeding VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetSparksVFX()
{
    if (SparksVFX.IsValid())
    {
        return SparksVFX.LoadSynchronous();
    }
    
    UE_LOG(LogVFXNiagara, Warning, TEXT("Sparks VFX asset not found"));
    return nullptr;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetCookingSmokeVFX()
{
    // Placeholder implementation - would load from /Game/VFX/Crafting/NS_Craft_CookingSmoke
    UE_LOG(LogVFXNiagara, Log, TEXT("Cooking smoke VFX requested - placeholder implementation"));
    return nullptr;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAtLocation(
    UWorld* World,
    UNiagaraSystem* VFXSystem,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale)
{
    if (!World || !VFXSystem)
    {
        UE_LOG(LogVFXNiagara, Error, TEXT("SpawnVFXAtLocation: Invalid World or VFXSystem"));
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        VFXSystem,
        Location,
        Rotation,
        Scale,
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None
    );

    if (VFXComponent)
    {
        UE_LOG(LogVFXNiagara, Log, TEXT("Spawned VFX %s at location %s"), 
               *VFXSystem->GetName(), *Location.ToString());
    }
    else
    {
        UE_LOG(LogVFXNiagara, Error, TEXT("Failed to spawn VFX %s"), *VFXSystem->GetName());
    }

    return VFXComponent;
}

void UVFX_NiagaraLibrary::AttachVFXToActor(
    AActor* TargetActor,
    UNiagaraSystem* VFXSystem,
    const FName& SocketName,
    const FVector& LocationOffset)
{
    if (!TargetActor || !VFXSystem)
    {
        UE_LOG(LogVFXNiagara, Error, TEXT("AttachVFXToActor: Invalid TargetActor or VFXSystem"));
        return;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFXSystem,
        TargetActor->GetRootComponent(),
        SocketName,
        LocationOffset,
        FRotator::ZeroRotator,
        FVector::OneVector,
        EAttachLocation::KeepRelativeOffset,
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None
    );

    if (VFXComponent)
    {
        UE_LOG(LogVFXNiagara, Log, TEXT("Attached VFX %s to actor %s"), 
               *VFXSystem->GetName(), *TargetActor->GetName());
    }
    else
    {
        UE_LOG(LogVFXNiagara, Error, TEXT("Failed to attach VFX %s to actor %s"), 
               *VFXSystem->GetName(), *TargetActor->GetName());
    }
}