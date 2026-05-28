#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance
    
    MaxActiveEffects = 50;
    CurrentQualityLevel = 2;
    EffectCullDistance = 5000.0f;
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultConfigs();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Initialized with %d effect configs"), EffectConfigs.Num());
}

void UVFX_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CleanupFinishedEffects();
}

UNiagaraComponent* UVFX_ParticleManager::SpawnVFXAtLocation(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Max active effects reached (%d)"), MaxActiveEffects);
        return nullptr;
    }

    const FVFX_EffectConfig* Config = EffectConfigs.Find(EffectType);
    if (!Config || !Config->NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No valid config for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = CreateNiagaraComponent(EffectType, Location, Rotation);
    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned effect at location %s"), *Location.ToString());
    }

    return EffectComponent;
}

UNiagaraComponent* UVFX_ParticleManager::SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachComponent, FName SocketName)
{
    if (!AttachComponent || ActiveEffects.Num() >= MaxActiveEffects)
    {
        return nullptr;
    }

    const FVFX_EffectConfig* Config = EffectConfigs.Find(EffectType);
    if (!Config || !Config->NiagaraSystem.IsValid())
    {
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        Config->NiagaraSystem.LoadSynchronous(),
        AttachComponent,
        SocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        Config->Scale,
        EAttachLocation::KeepWorldPosition,
        Config->bAutoDestroy
    );

    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned attached effect"));
    }

    return EffectComponent;
}

void UVFX_ParticleManager::StopVFXEffect(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->DestroyComponent();
        ActiveEffects.Remove(EffectComponent);
    }
}

void UVFX_ParticleManager::StopAllVFXEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Stopped all active effects"));
}

void UVFX_ParticleManager::CreateCampfire(const FVector& Location)
{
    UNiagaraComponent* FireEffect = SpawnVFXAtLocation(EVFX_EffectType::Fire_Campfire, Location);
    if (FireEffect)
    {
        // Create additional smoke effect slightly above fire
        SpawnVFXAtLocation(EVFX_EffectType::Smoke_Ambient, Location + FVector(0, 0, 50));
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Created campfire at %s"), *Location.ToString());
    }
}

void UVFX_ParticleManager::CreateBloodSplatter(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
    // Calculate rotation from impact normal
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    
    UNiagaraComponent* BloodEffect = SpawnVFXAtLocation(EVFX_EffectType::Blood_Impact, ImpactLocation, ImpactRotation);
    if (BloodEffect)
    {
        // Set blood-specific parameters
        BloodEffect->SetFloatParameter(TEXT("ImpactForce"), 1.0f);
        BloodEffect->SetVectorParameter(TEXT("ImpactDirection"), -ImpactNormal);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Created blood splatter"));
    }
}

void UVFX_ParticleManager::CreateFootstepDust(const FVector& FootLocation, float DinosaurSize)
{
    UNiagaraComponent* DustEffect = SpawnVFXAtLocation(EVFX_EffectType::Dust_Footstep, FootLocation);
    if (DustEffect)
    {
        // Scale dust based on dinosaur size
        float DustScale = FMath::Clamp(DinosaurSize, 0.5f, 5.0f);
        DustEffect->SetFloatParameter(TEXT("SizeMultiplier"), DustScale);
        DustEffect->SetFloatParameter(TEXT("ParticleCount"), DustScale * 50.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Created footstep dust (size: %.2f)"), DustScale);
    }
}

void UVFX_ParticleManager::CreateCraftingSparks(const FVector& CraftLocation)
{
    UNiagaraComponent* SparksEffect = SpawnVFXAtLocation(EVFX_EffectType::Sparks_Crafting, CraftLocation);
    if (SparksEffect)
    {
        // Set crafting-specific parameters
        SparksEffect->SetFloatParameter(TEXT("SparkIntensity"), 2.0f);
        SparksEffect->SetFloatParameter(TEXT("SparkLifetime"), 1.5f);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Created crafting sparks"));
    }
}

void UVFX_ParticleManager::SetEffectConfig(EVFX_EffectType EffectType, const FVFX_EffectConfig& Config)
{
    EffectConfigs.Add(EffectType, Config);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Updated config for effect type %d"), (int32)EffectType);
}

FVFX_EffectConfig UVFX_ParticleManager::GetEffectConfig(EVFX_EffectType EffectType) const
{
    const FVFX_EffectConfig* Config = EffectConfigs.Find(EffectType);
    return Config ? *Config : FVFX_EffectConfig();
}

void UVFX_ParticleManager::SetVFXQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 2);
    
    // Adjust max effects based on quality
    switch (CurrentQualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            EffectCullDistance = 2000.0f;
            break;
        case 1: // Medium
            MaxActiveEffects = 35;
            EffectCullDistance = 3500.0f;
            break;
        case 2: // High
            MaxActiveEffects = 50;
            EffectCullDistance = 5000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Quality set to %d (MaxEffects: %d)"), CurrentQualityLevel, MaxActiveEffects);
}

void UVFX_ParticleManager::InitializeDefaultConfigs()
{
    // Initialize default configurations for each effect type
    FVFX_EffectConfig FireConfig;
    FireConfig.Scale = FVector(1.0f, 1.0f, 1.5f);
    FireConfig.Lifetime = 0.0f; // Infinite for campfires
    FireConfig.bAutoDestroy = false;
    FireConfig.SpawnRate = 200.0f;
    EffectConfigs.Add(EVFX_EffectType::Fire_Campfire, FireConfig);

    FVFX_EffectConfig BloodConfig;
    BloodConfig.Scale = FVector(0.8f);
    BloodConfig.Lifetime = 3.0f;
    BloodConfig.bAutoDestroy = true;
    BloodConfig.SpawnRate = 150.0f;
    EffectConfigs.Add(EVFX_EffectType::Blood_Impact, BloodConfig);

    FVFX_EffectConfig DustConfig;
    DustConfig.Scale = FVector(1.2f);
    DustConfig.Lifetime = 2.0f;
    DustConfig.bAutoDestroy = true;
    DustConfig.SpawnRate = 100.0f;
    EffectConfigs.Add(EVFX_EffectType::Dust_Footstep, DustConfig);

    FVFX_EffectConfig SparksConfig;
    SparksConfig.Scale = FVector(0.6f);
    SparksConfig.Lifetime = 1.5f;
    SparksConfig.bAutoDestroy = true;
    SparksConfig.SpawnRate = 300.0f;
    EffectConfigs.Add(EVFX_EffectType::Sparks_Crafting, SparksConfig);

    FVFX_EffectConfig SmokeConfig;
    SmokeConfig.Scale = FVector(1.5f, 1.5f, 2.0f);
    SmokeConfig.Lifetime = 0.0f; // Infinite for ambient smoke
    SmokeConfig.bAutoDestroy = false;
    SmokeConfig.SpawnRate = 80.0f;
    EffectConfigs.Add(EVFX_EffectType::Smoke_Ambient, SmokeConfig);

    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Initialized %d default effect configs"), EffectConfigs.Num());
}

void UVFX_ParticleManager::CleanupFinishedEffects()
{
    ActiveEffects.RemoveAll([this](UNiagaraComponent* Effect)
    {
        if (!IsValid(Effect) || ShouldCullEffect(Effect))
        {
            if (IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            return true;
        }
        return false;
    });
}

bool UVFX_ParticleManager::ShouldCullEffect(UNiagaraComponent* Effect) const
{
    if (!Effect || !GetOwner())
    {
        return true;
    }

    // Distance culling
    float DistanceToPlayer = FVector::Dist(Effect->GetComponentLocation(), GetOwner()->GetActorLocation());
    return DistanceToPlayer > EffectCullDistance;
}

UNiagaraComponent* UVFX_ParticleManager::CreateNiagaraComponent(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation)
{
    const FVFX_EffectConfig* Config = EffectConfigs.Find(EffectType);
    if (!Config || !Config->NiagaraSystem.IsValid())
    {
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        Config->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Config->Scale,
        Config->bAutoDestroy
    );

    return EffectComponent;
}