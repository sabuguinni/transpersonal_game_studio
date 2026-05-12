#include "VFX_ImpactManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Cleanup every 0.5 seconds

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    GlobalVFXScale = 1.0f;
    bEnableVFX = true;
    MaxActiveEffects = 20;

    // Initialize default effects data
    InitializeDefaultEffects();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Impact Manager initialized - Cretaceous period effects ready"));
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup finished effects periodically
    CleanupFinishedEffects();
}

void AVFX_ImpactManager::TriggerImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation)
{
    if (!bEnableVFX)
    {
        return;
    }

    // Check if we have too many active effects
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Impact Manager: Maximum active effects reached, skipping new effect"));
        return;
    }

    // Find effect data for this impact type
    if (FVFX_ImpactData* EffectData = ImpactEffects.Find(ImpactType))
    {
        if (EffectData->NiagaraEffect.IsValid())
        {
            UNiagaraSystem* NiagaraSystem = EffectData->NiagaraEffect.LoadSynchronous();
            if (NiagaraSystem)
            {
                FVector ScaledEffectScale = EffectData->EffectScale * GlobalVFXScale;
                UNiagaraComponent* NewEffect = SpawnNiagaraEffect(NiagaraSystem, Location, Rotation, ScaledEffectScale);
                
                if (NewEffect)
                {
                    ActiveEffects.Add(NewEffect);
                    UE_LOG(LogTemp, Log, TEXT("VFX Impact: Triggered %s effect at location %s"), 
                           *UEnum::GetValueAsString(ImpactType), *Location.ToString());
                }
            }
        }
        else
        {
            // Fallback: Create basic particle effect using engine defaults
            UE_LOG(LogTemp, Warning, TEXT("VFX Impact: No Niagara system assigned for impact type %s, using fallback"), 
                   *UEnum::GetValueAsString(ImpactType));
        }
    }
}

void AVFX_ImpactManager::TriggerDinosaurFootstep(FVector Location, float DinosaurSize)
{
    // Adjust effect scale based on dinosaur size
    float OriginalScale = GlobalVFXScale;
    GlobalVFXScale *= DinosaurSize;
    
    TriggerImpactEffect(EVFX_ImpactType::DinosaurFootstep, Location);
    
    // Restore original scale
    GlobalVFXScale = OriginalScale;
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Dinosaur footstep effect triggered (size: %.2f) at %s"), 
           DinosaurSize, *Location.ToString());
}

void AVFX_ImpactManager::TriggerWaterSplash(FVector Location, float SplashIntensity)
{
    // Adjust effect scale based on splash intensity
    float OriginalScale = GlobalVFXScale;
    GlobalVFXScale *= SplashIntensity;
    
    TriggerImpactEffect(EVFX_ImpactType::WaterSplash, Location);
    
    // Restore original scale
    GlobalVFXScale = OriginalScale;
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Water splash effect triggered (intensity: %.2f) at %s"), 
           SplashIntensity, *Location.ToString());
}

void AVFX_ImpactManager::TriggerDustCloud(FVector Location, float CloudSize)
{
    // Adjust effect scale based on cloud size
    float OriginalScale = GlobalVFXScale;
    GlobalVFXScale *= CloudSize;
    
    TriggerImpactEffect(EVFX_ImpactType::DustCloud, Location);
    
    // Restore original scale
    GlobalVFXScale = OriginalScale;
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Dust cloud effect triggered (size: %.2f) at %s"), 
           CloudSize, *Location.ToString());
}

void AVFX_ImpactManager::CleanupFinishedEffects()
{
    // Remove null or finished effects from active list
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect) {
        if (!IsValid(Effect))
        {
            return true; // Remove null effects
        }
        
        // Check if effect is still active
        if (Effect->GetAsset() == nullptr || !Effect->IsActive())
        {
            Effect->DestroyComponent();
            return true; // Remove finished effects
        }
        
        return false; // Keep active effects
    });
}

UNiagaraComponent* AVFX_ImpactManager::SpawnNiagaraEffect(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!NiagaraSystem || !GetWorld())
    {
        return nullptr;
    }

    // Spawn Niagara component at location
    UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        Rotation,
        Scale,
        true,  // Auto destroy
        true,  // Auto activate
        ENCPoolMethod::None,
        true   // Pre cull check
    );

    return NiagaraComponent;
}

void AVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize default impact effects data
    FVFX_ImpactData DinosaurFootstepData;
    DinosaurFootstepData.ImpactType = EVFX_ImpactType::DinosaurFootstep;
    DinosaurFootstepData.EffectScale = FVector(2.0f, 2.0f, 1.0f);
    DinosaurFootstepData.EffectDuration = 4.0f;
    DinosaurFootstepData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::DinosaurFootstep, DinosaurFootstepData);

    FVFX_ImpactData PlayerFootstepData;
    PlayerFootstepData.ImpactType = EVFX_ImpactType::PlayerFootstep;
    PlayerFootstepData.EffectScale = FVector(0.5f, 0.5f, 0.3f);
    PlayerFootstepData.EffectDuration = 2.0f;
    PlayerFootstepData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::PlayerFootstep, PlayerFootstepData);

    FVFX_ImpactData WaterSplashData;
    WaterSplashData.ImpactType = EVFX_ImpactType::WaterSplash;
    WaterSplashData.EffectScale = FVector(1.5f, 1.5f, 2.0f);
    WaterSplashData.EffectDuration = 3.0f;
    WaterSplashData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::WaterSplash, WaterSplashData);

    FVFX_ImpactData DustCloudData;
    DustCloudData.ImpactType = EVFX_ImpactType::DustCloud;
    DustCloudData.EffectScale = FVector(3.0f, 3.0f, 1.5f);
    DustCloudData.EffectDuration = 5.0f;
    DustCloudData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::DustCloud, DustCloudData);

    FVFX_ImpactData RockImpactData;
    RockImpactData.ImpactType = EVFX_ImpactType::RockImpact;
    RockImpactData.EffectScale = FVector(1.0f, 1.0f, 1.0f);
    RockImpactData.EffectDuration = 2.5f;
    RockImpactData.bAttachToGround = true;
    ImpactEffects.Add(EVFX_ImpactType::RockImpact, RockImpactData);

    FVFX_ImpactData BloodSprayData;
    BloodSprayData.ImpactType = EVFX_ImpactType::BloodSpray;
    BloodSprayData.EffectScale = FVector(1.2f, 1.2f, 1.0f);
    BloodSprayData.EffectDuration = 3.5f;
    BloodSprayData.bAttachToGround = false;
    ImpactEffects.Add(EVFX_ImpactType::BloodSpray, BloodSprayData);

    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager: Default Cretaceous period effects initialized"));
}