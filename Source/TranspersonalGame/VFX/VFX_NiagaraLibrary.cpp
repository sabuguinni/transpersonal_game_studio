#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentWeatherVFX = nullptr;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectLibrary();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished VFX every frame
    CleanupFinishedVFX();
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    UNiagaraSystem* EffectSystem = GetEffectSystem(EffectType);
    if (!EffectSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No effect system found for type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectSystem,
        Location,
        Rotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Apply effect-specific settings
        if (EffectLibrary.Contains(EffectType))
        {
            const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
            VFXComponent->SetWorldScale3D(EffectData.Scale);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned VFX type %d at location %s"), (int32)EffectType, *Location.ToString());
    }

    return VFXComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName)
{
    if (!AttachToComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Invalid attach component"));
        return nullptr;
    }

    UNiagaraSystem* EffectSystem = GetEffectSystem(EffectType);
    if (!EffectSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No effect system found for type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        EffectSystem,
        AttachToComponent,
        AttachPointName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        
        // Apply effect-specific settings
        if (EffectLibrary.Contains(EffectType))
        {
            const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
            VFXComponent->SetWorldScale3D(EffectData.Scale);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned attached VFX type %d"), (int32)EffectType);
    }

    return VFXComponent;
}

void UVFX_NiagaraLibrary::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && IsValid(VFXComponent))
    {
        VFXComponent->Deactivate();
        ActiveVFXComponents.Remove(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped VFX component"));
    }
}

void UVFX_NiagaraLibrary::StopAllVFX()
{
    for (UNiagaraComponent* VFXComponent : ActiveVFXComponents)
    {
        if (VFXComponent && IsValid(VFXComponent))
        {
            VFXComponent->Deactivate();
        }
    }
    ActiveVFXComponents.Empty();
    
    if (CurrentWeatherVFX && IsValid(CurrentWeatherVFX))
    {
        CurrentWeatherVFX->Deactivate();
        CurrentWeatherVFX = nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped all VFX"));
}

void UVFX_NiagaraLibrary::PlayDinosaurFootstepVFX(FVector ImpactLocation, float DinosaurSize)
{
    UNiagaraComponent* FootstepVFX = SpawnVFXAtLocation(EVFX_EffectType::Dust_Footstep, ImpactLocation);
    if (FootstepVFX)
    {
        // Scale effect based on dinosaur size
        FVector ScaledSize = FVector(DinosaurSize, DinosaurSize, DinosaurSize);
        FootstepVFX->SetWorldScale3D(ScaledSize);
        
        // Set dust intensity parameter
        FootstepVFX->SetFloatParameter(FName("Intensity"), DinosaurSize);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Played dinosaur footstep VFX with size %f"), DinosaurSize);
    }
}

void UVFX_NiagaraLibrary::PlayCampfireVFX(FVector FireLocation)
{
    UNiagaraComponent* CampfireVFX = SpawnVFXAtLocation(EVFX_EffectType::Fire_Campfire, FireLocation);
    if (CampfireVFX)
    {
        // Campfire effects should loop indefinitely
        CampfireVFX->SetAutoDestroy(false);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Started campfire VFX at %s"), *FireLocation.ToString());
    }
}

void UVFX_NiagaraLibrary::StartRainVFX(float Intensity)
{
    // Stop existing weather VFX
    if (CurrentWeatherVFX && IsValid(CurrentWeatherVFX))
    {
        CurrentWeatherVFX->Deactivate();
    }

    // Spawn rain VFX high above the player
    FVector RainLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 1000);
    CurrentWeatherVFX = SpawnVFXAtLocation(EVFX_EffectType::Weather_Rain, RainLocation);
    
    if (CurrentWeatherVFX)
    {
        CurrentWeatherVFX->SetAutoDestroy(false);
        CurrentWeatherVFX->SetFloatParameter(FName("Intensity"), Intensity);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Started rain VFX with intensity %f"), Intensity);
    }
}

void UVFX_NiagaraLibrary::StopRainVFX()
{
    if (CurrentWeatherVFX && IsValid(CurrentWeatherVFX))
    {
        CurrentWeatherVFX->Deactivate();
        CurrentWeatherVFX = nullptr;
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped rain VFX"));
    }
}

void UVFX_NiagaraLibrary::PlayImpactVFX(FVector ImpactLocation, EVFX_EffectType ImpactType)
{
    UNiagaraComponent* ImpactVFX = SpawnVFXAtLocation(ImpactType, ImpactLocation);
    if (ImpactVFX)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Played impact VFX type %d at %s"), (int32)ImpactType, *ImpactLocation.ToString());
    }
}

void UVFX_NiagaraLibrary::InitializeEffectLibrary()
{
    // Initialize default VFX effect data
    // Note: In a real project, these would reference actual Niagara system assets
    
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Scale = FVector(1.0f);
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Scale = FVector(1.0f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Dust_Footstep, FootstepData);

    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Scale = FVector(10.0f, 10.0f, 1.0f);
    RainData.Duration = -1.0f; // Infinite
    RainData.bAutoDestroy = false;
    EffectLibrary.Add(EVFX_EffectType::Weather_Rain, RainData);

    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Impact_Blood;
    BloodData.Scale = FVector(1.0f);
    BloodData.Duration = 3.0f;
    BloodData.bAutoDestroy = true;
    EffectLibrary.Add(EVFX_EffectType::Impact_Blood, BloodData);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized effect library with %d effects"), EffectLibrary.Num());
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetEffectSystem(EVFX_EffectType EffectType)
{
    if (EffectLibrary.Contains(EffectType))
    {
        const FVFX_EffectData& EffectData = EffectLibrary[EffectType];
        if (EffectData.NiagaraSystem.IsValid())
        {
            return EffectData.NiagaraSystem.Get();
        }
    }
    
    // Return nullptr if no system is found - in a real project, this would load default systems
    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system configured for effect type %d"), (int32)EffectType);
    return nullptr;
}

void UVFX_NiagaraLibrary::CleanupFinishedVFX()
{
    // Remove invalid or finished VFX components
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* VFXComponent = ActiveVFXComponents[i];
        if (!VFXComponent || !IsValid(VFXComponent) || !VFXComponent->IsActive())
        {
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}