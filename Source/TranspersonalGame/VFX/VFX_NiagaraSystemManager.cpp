#include "VFX_NiagaraSystemManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AVFX_NiagaraSystemManager::AVFX_NiagaraSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default VFX systems paths
    VFXSystems.Add(EVFX_EffectType::Fire_Campfire, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_Fire"))));
    VFXSystems.Add(EVFX_EffectType::Dust_Footstep, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_Dust"))));
    VFXSystems.Add(EVFX_EffectType::Weather_Rain, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_Rain"))));
    VFXSystems.Add(EVFX_EffectType::Combat_Blood, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_Blood"))));
    VFXSystems.Add(EVFX_EffectType::Water_Splash, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_Splash"))));
    VFXSystems.Add(EVFX_EffectType::Environment_Dust, TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_EnvDust"))));
    
    // Set default settings
    DefaultSettings.Duration = 5.0f;
    DefaultSettings.Intensity = 1.0f;
    DefaultSettings.Scale = FVector(1.0f);
    DefaultSettings.bAutoDestroy = true;
}

void AVFX_NiagaraSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize cleanup timer
    CleanupTimer = 0.0f;
}

void AVFX_NiagaraSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= CleanupInterval)
    {
        CleanupFinishedVFX();
        CleanupTimer = 0.0f;
    }
}

UNiagaraComponent* AVFX_NiagaraSystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, const FVFX_EffectSettings& Settings)
{
    if (!VFXSystems.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not found: %d"), (int32)EffectType);
        return nullptr;
    }
    
    TSoftObjectPtr<UNiagaraSystem> SystemPtr = VFXSystems[EffectType];
    UNiagaraSystem* NiagaraSystem = SystemPtr.LoadSynchronous();
    
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara system for effect type: %d"), (int32)EffectType);
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = NewObject<UNiagaraComponent>(this);
    if (!VFXComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Niagara component"));
        return nullptr;
    }
    
    VFXComponent->SetAsset(NiagaraSystem);
    VFXComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    VFXComponent->SetWorldLocation(Location);
    VFXComponent->SetWorldRotation(Rotation);
    VFXComponent->SetWorldScale3D(Settings.Scale);
    
    // Set intensity parameter if available
    VFXComponent->SetFloatParameter(TEXT("Intensity"), Settings.Intensity);
    
    // Auto-destroy setup
    if (Settings.bAutoDestroy && Settings.Duration > 0.0f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, VFXComponent]()
        {
            if (IsValid(VFXComponent))
            {
                StopVFXEffect(VFXComponent);
            }
        }, Settings.Duration, false);
    }
    
    // Register component and activate
    VFXComponent->RegisterComponent();
    VFXComponent->Activate();
    ActiveVFXComponents.Add(VFXComponent);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned VFX effect type %d at location %s"), (int32)EffectType, *Location.ToString());
    
    return VFXComponent;
}

void AVFX_NiagaraSystemManager::StopVFXEffect(UNiagaraComponent* VFXComponent)
{
    if (!IsValid(VFXComponent))
    {
        return;
    }
    
    VFXComponent->Deactivate();
    ActiveVFXComponents.Remove(VFXComponent);
    
    // Destroy component after a short delay to allow particles to fade
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [VFXComponent]()
    {
        if (IsValid(VFXComponent))
        {
            VFXComponent->DestroyComponent();
        }
    }, 1.0f, false);
}

void AVFX_NiagaraSystemManager::StopAllVFXEffects()
{
    TArray<UNiagaraComponent*> ComponentsCopy = ActiveVFXComponents;
    for (UNiagaraComponent* Component : ComponentsCopy)
    {
        StopVFXEffect(Component);
    }
    ActiveVFXComponents.Empty();
}

void AVFX_NiagaraSystemManager::SetVFXIntensity(UNiagaraComponent* VFXComponent, float NewIntensity)
{
    if (IsValid(VFXComponent))
    {
        VFXComponent->SetFloatParameter(TEXT("Intensity"), NewIntensity);
    }
}

void AVFX_NiagaraSystemManager::OnVFXFinished(UNiagaraComponent* FinishedComponent)
{
    if (IsValid(FinishedComponent))
    {
        ActiveVFXComponents.Remove(FinishedComponent);
        FinishedComponent->DestroyComponent();
    }
}

void AVFX_NiagaraSystemManager::CleanupFinishedVFX()
{
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });
}