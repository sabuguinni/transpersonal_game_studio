#include "VFX_NiagaraLibrary.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"

TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> UVFX_NiagaraLibrary::EffectRegistry;

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAtLocation(
    UObject* WorldContext,
    EVFX_EffectType EffectType,
    FVector Location,
    FRotator Rotation,
    FVector Scale)
{
    if (!WorldContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: WorldContext is null"));
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Could not get world from context"));
        return nullptr;
    }

    InitializeEffectRegistry();

    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEffect(EffectType);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system found for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Location,
        Rotation,
        Scale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (VFXComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Successfully spawned VFX effect at location %s"), *Location.ToString());
    }

    return VFXComponent;
}

UNiagaraComponent* UVFX_NiagaraLibrary::AttachVFXToActor(
    AActor* TargetActor,
    EVFX_EffectType EffectType,
    FName SocketName,
    FVector RelativeLocation,
    FRotator RelativeRotation,
    FVector Scale)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: TargetActor is null"));
        return nullptr;
    }

    InitializeEffectRegistry();

    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEffect(EffectType);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system found for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        TargetActor->GetRootComponent(),
        SocketName,
        RelativeLocation,
        RelativeRotation,
        Scale,
        EAttachLocation::KeepRelativeOffset,
        true,
        ENCPoolMethod::None,
        true
    );

    if (VFXComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Successfully attached VFX effect to actor %s"), *TargetActor->GetName());
    }

    return VFXComponent;
}

void UVFX_NiagaraLibrary::StopVFXEffect(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: VFXComponent is null"));
        return;
    }

    VFXComponent->Deactivate();
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: VFX effect stopped"));
}

TArray<FVFX_EffectData> UVFX_NiagaraLibrary::GetAvailableEffects()
{
    TArray<FVFX_EffectData> AvailableEffects;
    
    InitializeEffectRegistry();

    for (const auto& EffectPair : EffectRegistry)
    {
        FVFX_EffectData EffectData;
        EffectData.EffectType = EffectPair.Key;
        EffectData.NiagaraSystem = EffectPair.Value;
        
        switch (EffectPair.Key)
        {
            case EVFX_EffectType::Fire_Campfire:
                EffectData.Duration = 0.0f; // Looping
                EffectData.Scale = FVector(1.0f);
                break;
            case EVFX_EffectType::Dust_Impact:
                EffectData.Duration = 2.0f;
                EffectData.Scale = FVector(1.5f);
                break;
            case EVFX_EffectType::Water_Splash:
                EffectData.Duration = 1.5f;
                EffectData.Scale = FVector(1.0f);
                break;
            case EVFX_EffectType::Blood_Hit:
                EffectData.Duration = 3.0f;
                EffectData.Scale = FVector(0.8f);
                break;
            case EVFX_EffectType::Steam_Breath:
                EffectData.Duration = 4.0f;
                EffectData.Scale = FVector(1.2f);
                break;
            case EVFX_EffectType::Volcanic_Smoke:
                EffectData.Duration = 0.0f; // Looping
                EffectData.Scale = FVector(2.0f);
                break;
        }
        
        AvailableEffects.Add(EffectData);
    }

    return AvailableEffects;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetNiagaraSystemForEffect(EVFX_EffectType EffectType)
{
    InitializeEffectRegistry();

    if (EffectRegistry.Contains(EffectType))
    {
        TSoftObjectPtr<UNiagaraSystem> SystemPtr = EffectRegistry[EffectType];
        if (SystemPtr.IsValid())
        {
            return SystemPtr.Get();
        }
        else if (!SystemPtr.IsNull())
        {
            // Try to load the asset
            return SystemPtr.LoadSynchronous();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No Niagara system registered for effect type %d"), (int32)EffectType);
    return nullptr;
}

void UVFX_NiagaraLibrary::InitializeEffectRegistry()
{
    if (EffectRegistry.Num() > 0)
    {
        return; // Already initialized
    }

    // Initialize with placeholder paths - these will be replaced with actual Niagara systems
    EffectRegistry.Add(EVFX_EffectType::Fire_Campfire, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Smoke"))));
    
    EffectRegistry.Add(EVFX_EffectType::Dust_Impact, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Dust"))));
    
    EffectRegistry.Add(EVFX_EffectType::Water_Splash, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Water"))));
    
    EffectRegistry.Add(EVFX_EffectType::Blood_Hit, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Blood"))));
    
    EffectRegistry.Add(EVFX_EffectType::Steam_Breath, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Steam"))));
    
    EffectRegistry.Add(EVFX_EffectType::Volcanic_Smoke, 
        TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/DefaultAssets/FX_Volcano"))));

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Effect registry initialized with %d effects"), EffectRegistry.Num());
}