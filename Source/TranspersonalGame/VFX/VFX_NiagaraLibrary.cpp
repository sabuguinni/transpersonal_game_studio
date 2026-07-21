#include "VFX_NiagaraLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

// Initialize static member
TMap<EVFX_EffectType, FString> UVFX_NiagaraLibrary::EffectSystemPaths;

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnNiagaraEffectAtLocation(
    const UObject* WorldContext,
    EVFX_EffectType EffectType,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    bool bAutoDestroy)
{
    if (!WorldContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Invalid WorldContext"));
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEffect(EffectType);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system found for effect type"));
        return nullptr;
    }

    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        WorldContext,
        NiagaraSystem,
        Location,
        Rotation,
        Scale,
        bAutoDestroy
    );

    if (NiagaraComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned Niagara effect at location %s"), *Location.ToString());
    }

    return NiagaraComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnNiagaraEffectAttached(
    EVFX_EffectType EffectType,
    USceneComponent* AttachToComponent,
    FName AttachPointName,
    FVector Location,
    FRotator Rotation,
    FVector Scale,
    bool bAutoDestroy)
{
    if (!AttachToComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Invalid AttachToComponent"));
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEffect(EffectType);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system found for effect type"));
        return nullptr;
    }

    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        AttachToComponent,
        AttachPointName,
        Location,
        Rotation,
        Scale,
        EAttachLocation::KeepWorldPosition,
        bAutoDestroy
    );

    if (NiagaraComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned Niagara effect attached to component"));
    }

    return NiagaraComponent;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetNiagaraSystemForEffect(EVFX_EffectType EffectType)
{
    InitializeEffectPaths();

    const FString* SystemPath = EffectSystemPaths.Find(EffectType);
    if (!SystemPath)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No system path found for effect type"));
        return nullptr;
    }

    return LoadNiagaraSystemFromPath(*SystemPath);
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateCampfireEffect(
    const UObject* WorldContext,
    FVector Location,
    float FireIntensity,
    float SmokeAmount)
{
    UNiagaraComponent* CampfireComponent = SpawnNiagaraEffectAtLocation(
        WorldContext,
        EVFX_EffectType::Fire_Campfire,
        Location,
        FRotator::ZeroRotator,
        FVector(1.0f, 1.0f, 1.0f),
        false // Don't auto-destroy campfire
    );

    if (CampfireComponent)
    {
        // Set custom parameters for fire intensity and smoke
        CampfireComponent->SetFloatParameter(FName("FireIntensity"), FireIntensity);
        CampfireComponent->SetFloatParameter(FName("SmokeAmount"), SmokeAmount);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created campfire effect with intensity %f"), FireIntensity);
    }

    return CampfireComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateFootstepDustEffect(
    const UObject* WorldContext,
    FVector Location,
    float DinosaurSize,
    FVector GroundNormal)
{
    UNiagaraComponent* DustComponent = SpawnNiagaraEffectAtLocation(
        WorldContext,
        EVFX_EffectType::Dino_Footstep,
        Location,
        GroundNormal.Rotation(),
        FVector(DinosaurSize, DinosaurSize, DinosaurSize),
        true // Auto-destroy after duration
    );

    if (DustComponent)
    {
        // Set custom parameters for dinosaur size and ground type
        DustComponent->SetFloatParameter(FName("DinosaurSize"), DinosaurSize);
        DustComponent->SetVectorParameter(FName("GroundNormal"), GroundNormal);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created footstep dust effect for size %f"), DinosaurSize);
    }

    return DustComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateRainEffect(
    const UObject* WorldContext,
    FVector Location,
    float RainIntensity,
    FVector RainDirection)
{
    UNiagaraComponent* RainComponent = SpawnNiagaraEffectAtLocation(
        WorldContext,
        EVFX_EffectType::Weather_Rain,
        Location,
        RainDirection.Rotation(),
        FVector(1.0f, 1.0f, 1.0f),
        false // Don't auto-destroy weather effects
    );

    if (RainComponent)
    {
        // Set custom parameters for rain intensity and direction
        RainComponent->SetFloatParameter(FName("RainIntensity"), RainIntensity);
        RainComponent->SetVectorParameter(FName("RainDirection"), RainDirection);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created rain effect with intensity %f"), RainIntensity);
    }

    return RainComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateBloodImpactEffect(
    const UObject* WorldContext,
    FVector Location,
    FVector ImpactNormal,
    float BloodAmount)
{
    UNiagaraComponent* BloodComponent = SpawnNiagaraEffectAtLocation(
        WorldContext,
        EVFX_EffectType::Impact_Blood,
        Location,
        ImpactNormal.Rotation(),
        FVector(BloodAmount, BloodAmount, BloodAmount),
        true // Auto-destroy after impact
    );

    if (BloodComponent)
    {
        // Set custom parameters for blood amount and impact direction
        BloodComponent->SetFloatParameter(FName("BloodAmount"), BloodAmount);
        BloodComponent->SetVectorParameter(FName("ImpactNormal"), ImpactNormal);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Created blood impact effect with amount %f"), BloodAmount);
    }

    return BloodComponent;
}

bool UVFX_NiagaraLibrary::InitializeVFXLibrary()
{
    InitializeEffectPaths();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized with %d effect types"), EffectSystemPaths.Num());
    
    // Verify that at least some core effects are available
    int32 ValidEffects = 0;
    for (const auto& EffectPair : EffectSystemPaths)
    {
        UNiagaraSystem* System = LoadNiagaraSystemFromPath(EffectPair.Value);
        if (System)
        {
            ValidEffects++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: %d valid Niagara systems found"), ValidEffects);
    return ValidEffects > 0;
}

void UVFX_NiagaraLibrary::InitializeEffectPaths()
{
    if (EffectSystemPaths.Num() > 0)
    {
        return; // Already initialized
    }

    // Initialize paths to Niagara systems (these will be created by content creators)
    EffectSystemPaths.Add(EVFX_EffectType::Fire_Campfire, TEXT("/Game/VFX/Niagara/NS_Fire_Campfire"));
    EffectSystemPaths.Add(EVFX_EffectType::Fire_Torch, TEXT("/Game/VFX/Niagara/NS_Fire_Torch"));
    EffectSystemPaths.Add(EVFX_EffectType::Impact_Dust, TEXT("/Game/VFX/Niagara/NS_Impact_Dust"));
    EffectSystemPaths.Add(EVFX_EffectType::Impact_Blood, TEXT("/Game/VFX/Niagara/NS_Impact_Blood"));
    EffectSystemPaths.Add(EVFX_EffectType::Impact_Rock, TEXT("/Game/VFX/Niagara/NS_Impact_Rock"));
    EffectSystemPaths.Add(EVFX_EffectType::Weather_Rain, TEXT("/Game/VFX/Niagara/NS_Weather_Rain"));
    EffectSystemPaths.Add(EVFX_EffectType::Weather_Snow, TEXT("/Game/VFX/Niagara/NS_Weather_Snow"));
    EffectSystemPaths.Add(EVFX_EffectType::Weather_Fog, TEXT("/Game/VFX/Niagara/NS_Weather_Fog"));
    EffectSystemPaths.Add(EVFX_EffectType::Dino_Footstep, TEXT("/Game/VFX/Niagara/NS_Dino_Footstep"));
    EffectSystemPaths.Add(EVFX_EffectType::Dino_Breath, TEXT("/Game/VFX/Niagara/NS_Dino_Breath"));
    EffectSystemPaths.Add(EVFX_EffectType::Water_Splash, TEXT("/Game/VFX/Niagara/NS_Water_Splash"));
    EffectSystemPaths.Add(EVFX_EffectType::Water_Ripple, TEXT("/Game/VFX/Niagara/NS_Water_Ripple"));

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized %d effect system paths"), EffectSystemPaths.Num());
}

UNiagaraSystem* UVFX_NiagaraLibrary::LoadNiagaraSystemFromPath(const FString& SystemPath)
{
    if (SystemPath.IsEmpty())
    {
        return nullptr;
    }

    // Try to load the Niagara system from the specified path
    UNiagaraSystem* NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
    
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Failed to load Niagara system from path: %s"), *SystemPath);
    }

    return NiagaraSystem;
}