#include "VFX_NiagaraLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxActiveEffects = 50;
    DefaultEffectDuration = 5.0f;
}

void UVFX_NiagaraLibrary::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectAssets();
    LoadEffectAssets();
}

void UVFX_NiagaraLibrary::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupExpiredEffects();
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EffectType;
    
    return SpawnEffectAdvanced(Config, Location, Rotation);
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnEffectAdvanced(const FVFX_EffectConfig& Config, FVector Location, FRotator Rotation)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Maximum active effects reached (%d)"), MaxActiveEffects);
        return nullptr;
    }

    UNiagaraSystem* EffectAsset = GetEffectAsset(Config.EffectType);
    if (!EffectAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: No asset found for effect type %d"), (int32)Config.EffectType);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = CreateEffectComponent(EffectAsset, Location, Rotation);
    if (!EffectComponent)
    {
        return nullptr;
    }

    // Apply configuration
    EffectComponent->SetWorldScale3D(Config.Scale);
    
    // Set intensity parameter if the effect supports it
    EffectComponent->SetFloatParameter(TEXT("Intensity"), Config.Intensity);
    
    // Auto-destroy timer
    if (Config.bAutoDestroy && Config.Duration > 0.0f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, EffectComponent]()
        {
            StopEffect(EffectComponent);
        }, Config.Duration, false);
    }

    ActiveEffects.Add(EffectComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Spawned effect type %d at location %s"), 
           (int32)Config.EffectType, *Location.ToString());
    
    return EffectComponent;
}

void UVFX_NiagaraLibrary::StopEffect(UNiagaraComponent* EffectComponent)
{
    if (!EffectComponent)
    {
        return;
    }

    ActiveEffects.Remove(EffectComponent);
    
    EffectComponent->Deactivate();
    EffectComponent->DestroyComponent();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped and destroyed effect component"));
}

void UVFX_NiagaraLibrary::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            Effect->Deactivate();
            Effect->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Stopped all active effects"));
}

bool UVFX_NiagaraLibrary::LoadEffectAssets()
{
    int32 LoadedCount = 0;
    
    for (auto& AssetPair : EffectAssets)
    {
        if (AssetPair.Value.IsValid())
        {
            UNiagaraSystem* LoadedAsset = AssetPair.Value.LoadSynchronous();
            if (LoadedAsset)
            {
                LoadedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Loaded %d/%d effect assets"), LoadedCount, EffectAssets.Num());
    return LoadedCount > 0;
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetEffectAsset(EVFX_EffectType EffectType)
{
    if (TSoftObjectPtr<UNiagaraSystem>* AssetPtr = EffectAssets.Find(EffectType))
    {
        return AssetPtr->LoadSynchronous();
    }
    
    return nullptr;
}

void UVFX_NiagaraLibrary::InitializeEffectAssets()
{
    // Initialize with placeholder paths - these would be replaced with actual Niagara assets
    EffectAssets.Add(EVFX_EffectType::Fire_Campfire, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Fire/NS_Fire_Campfire"))));
    
    EffectAssets.Add(EVFX_EffectType::Fire_Torch, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Fire/NS_Fire_Torch"))));
    
    EffectAssets.Add(EVFX_EffectType::Impact_Blood, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Impact/NS_Impact_Blood"))));
    
    EffectAssets.Add(EVFX_EffectType::Impact_Dust, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Impact/NS_Impact_Dust"))));
    
    EffectAssets.Add(EVFX_EffectType::Impact_Rock, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Impact/NS_Impact_Rock"))));
    
    EffectAssets.Add(EVFX_EffectType::Weather_Rain, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Weather/NS_Weather_Rain"))));
    
    EffectAssets.Add(EVFX_EffectType::Weather_Snow, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Weather/NS_Weather_Snow"))));
    
    EffectAssets.Add(EVFX_EffectType::Weather_Fog, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Weather/NS_Weather_Fog"))));
    
    EffectAssets.Add(EVFX_EffectType::Dino_Footstep, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaur/NS_Dino_Footstep"))));
    
    EffectAssets.Add(EVFX_EffectType::Dino_Breath, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaur/NS_Dino_Breath"))));
    
    EffectAssets.Add(EVFX_EffectType::Dino_Roar, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Dinosaur/NS_Dino_Roar"))));
    
    EffectAssets.Add(EVFX_EffectType::Water_Splash, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Water/NS_Water_Splash"))));
    
    EffectAssets.Add(EVFX_EffectType::Water_Ripple, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Water/NS_Water_Ripple"))));
    
    EffectAssets.Add(EVFX_EffectType::Ambient_Insects, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Ambient/NS_Ambient_Insects"))));
    
    EffectAssets.Add(EVFX_EffectType::Ambient_Pollen, 
                     TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/Ambient/NS_Ambient_Pollen"))));
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraLibrary: Initialized %d effect asset references"), EffectAssets.Num());
}

void UVFX_NiagaraLibrary::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive())
        {
            if (Effect)
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
}

UNiagaraComponent* UVFX_NiagaraLibrary::CreateEffectComponent(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation)
{
    if (!NiagaraSystem)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World, NiagaraSystem, Location, Rotation);
    
    if (!EffectComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraLibrary: Failed to spawn Niagara component"));
        return nullptr;
    }

    return EffectComponent;
}