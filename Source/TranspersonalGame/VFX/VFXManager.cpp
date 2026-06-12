#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    LastCleanupTime = 0.0f;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXLibrary();
    
    UE_LOG(LogTemp, Warning, TEXT("VFXManager initialized with %d effects in library"), VFXLibrary.Num());
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic cleanup of finished VFX
    LastCleanupTime += DeltaTime;
    if (LastCleanupTime >= CleanupInterval)
    {
        CleanupFinishedVFX();
        LastCleanupTime = 0.0f;
    }
}

void AVFXManager::InitializeVFXLibrary()
{
    // Initialize common VFX effects
    FVFX_EffectData CampfireData;
    CampfireData.DefaultLifetime = -1.0f; // Persistent effect
    CampfireData.bAutoDestroy = false;
    CampfireData.DefaultScale = FVector(1.0f, 1.0f, 1.5f);
    VFXLibrary.Add(TEXT("Campfire"), CampfireData);
    
    FVFX_EffectData FootstepData;
    FootstepData.DefaultLifetime = 2.0f;
    FootstepData.bAutoDestroy = true;
    FootstepData.DefaultScale = FVector(1.0f);
    VFXLibrary.Add(TEXT("FootstepDust"), FootstepData);
    
    FVFX_EffectData BloodData;
    BloodData.DefaultLifetime = 3.0f;
    BloodData.bAutoDestroy = true;
    BloodData.DefaultScale = FVector(0.8f);
    VFXLibrary.Add(TEXT("BloodSplatter"), BloodData);
    
    FVFX_EffectData RainData;
    RainData.DefaultLifetime = -1.0f; // Weather effect
    RainData.bAutoDestroy = false;
    RainData.DefaultScale = FVector(10.0f, 10.0f, 5.0f);
    VFXLibrary.Add(TEXT("Rain"), RainData);
}

UNiagaraComponent* AVFXManager::SpawnVFXAtLocation(FName EffectName, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!VFXLibrary.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX effect '%s' not found in library"), *EffectName.ToString());
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = VFXLibrary[EffectName];
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Niagara system for effect '%s' is not valid"), *EffectName.ToString());
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem.Get(),
        Location,
        Rotation,
        Scale * EffectData.DefaultScale,
        EffectData.bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Set lifetime if specified
        if (EffectData.DefaultLifetime > 0.0f)
        {
            VFXComponent->SetFloatParameter(TEXT("Lifetime"), EffectData.DefaultLifetime);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX '%s' at location %s"), *EffectName.ToString(), *Location.ToString());
    }
    
    return VFXComponent;
}

UNiagaraComponent* AVFXManager::SpawnVFXAttached(FName EffectName, USceneComponent* AttachComponent, FName AttachPointName, FVector RelativeLocation, FRotator RelativeRotation, FVector Scale)
{
    if (!VFXLibrary.Contains(EffectName) || !AttachComponent)
    {
        return nullptr;
    }
    
    const FVFX_EffectData& EffectData = VFXLibrary[EffectName];
    if (!EffectData.NiagaraSystem.IsValid())
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectData.NiagaraSystem.Get(),
        AttachComponent,
        AttachPointName,
        RelativeLocation,
        RelativeRotation,
        Scale * EffectData.DefaultScale,
        EAttachLocation::KeepRelativeOffset,
        EffectData.bAutoDestroy
    );
    
    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
    }
    
    return VFXComponent;
}

void AVFXManager::SpawnCampfireEffect(FVector Location)
{
    SpawnVFXAtLocation(TEXT("Campfire"), Location);
}

void AVFXManager::SpawnFootstepDust(FVector Location, float IntensityScale)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(TEXT("FootstepDust"), Location, FRotator::ZeroRotator, FVector(IntensityScale));
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("Intensity"), IntensityScale);
    }
}

void AVFXManager::SpawnBloodSplatter(FVector Location, FVector Direction)
{
    FRotator SplatterRotation = Direction.Rotation();
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(TEXT("BloodSplatter"), Location, SplatterRotation);
    if (VFXComp)
    {
        VFXComp->SetVectorParameter(TEXT("Direction"), Direction);
    }
}

void AVFXManager::SpawnRainEffect(FVector Location, float Intensity)
{
    UNiagaraComponent* VFXComp = SpawnVFXAtLocation(TEXT("Rain"), Location, FRotator::ZeroRotator, FVector(Intensity));
    if (VFXComp)
    {
        VFXComp->SetFloatParameter(TEXT("RainIntensity"), Intensity);
    }
}

void AVFXManager::StopVFXComponent(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && IsValid(VFXComponent))
    {
        VFXComponent->DeactivateImmediate();
        ActiveVFXComponents.Remove(VFXComponent);
    }
}

void AVFXManager::StopAllVFX()
{
    for (UNiagaraComponent* VFXComp : ActiveVFXComponents)
    {
        if (IsValid(VFXComp))
        {
            VFXComp->DeactivateImmediate();
        }
    }
    ActiveVFXComponents.Empty();
}

void AVFXManager::RegisterVFXEffect(FName EffectName, UNiagaraSystem* NiagaraSystem, float Lifetime, bool bAutoDestroy)
{
    if (!NiagaraSystem)
    {
        return;
    }
    
    FVFX_EffectData NewEffectData;
    NewEffectData.NiagaraSystem = NiagaraSystem;
    NewEffectData.DefaultLifetime = Lifetime;
    NewEffectData.bAutoDestroy = bAutoDestroy;
    
    VFXLibrary.Add(EffectName, NewEffectData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered VFX effect '%s'"), *EffectName.ToString());
}

void AVFXManager::CleanupFinishedVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp) || !VFXComp->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

void AVFXManager::CleanupVFXComponent(UNiagaraComponent* Component)
{
    if (Component && IsValid(Component))
    {
        Component->DeactivateImmediate();
        ActiveVFXComponents.Remove(Component);
    }
}