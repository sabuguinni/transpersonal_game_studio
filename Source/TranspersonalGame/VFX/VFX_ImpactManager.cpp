#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Camera/PlayerCameraManager.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    MinTimeBetweenEffects = 0.1f;
    bUseGroundMaterialDetection = true;
    EffectCullDistance = 2000.0f;
    LastEffectTime = 0.0f;
    CachedWorld = nullptr;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    CachedWorld = GetWorld();
    SetupDefaultEffects();
}

void UVFX_ImpactManager::SetupDefaultEffects()
{
    // Light Footstep (Small dinosaurs, humans)
    FVFX_ImpactData LightData;
    LightData.ParticleScale = 0.5f;
    LightData.EffectDuration = 1.0f;
    LightData.SpawnRadius = 50.0f;
    ImpactEffects.Add(EVFX_ImpactType::Light_Footstep, LightData);

    // Medium Footstep (Medium dinosaurs)
    FVFX_ImpactData MediumData;
    MediumData.ParticleScale = 1.0f;
    MediumData.EffectDuration = 1.5f;
    MediumData.SpawnRadius = 100.0f;
    ImpactEffects.Add(EVFX_ImpactType::Medium_Footstep, MediumData);

    // Heavy Footstep (Large theropods)
    FVFX_ImpactData HeavyData;
    HeavyData.ParticleScale = 2.0f;
    HeavyData.EffectDuration = 2.0f;
    HeavyData.SpawnRadius = 150.0f;
    ImpactEffects.Add(EVFX_ImpactType::Heavy_Footstep, HeavyData);

    // Massive Footstep (T-Rex, Brachiosaurus)
    FVFX_ImpactData MassiveData;
    MassiveData.ParticleScale = 3.0f;
    MassiveData.EffectDuration = 3.0f;
    MassiveData.SpawnRadius = 250.0f;
    ImpactEffects.Add(EVFX_ImpactType::Massive_Footstep, MassiveData);

    // Weapon Impact
    FVFX_ImpactData WeaponData;
    WeaponData.ParticleScale = 0.8f;
    WeaponData.EffectDuration = 1.2f;
    WeaponData.SpawnRadius = 75.0f;
    ImpactEffects.Add(EVFX_ImpactType::Weapon_Impact, WeaponData);

    // Fall Impact
    FVFX_ImpactData FallData;
    FallData.ParticleScale = 1.5f;
    FallData.EffectDuration = 2.5f;
    FallData.SpawnRadius = 120.0f;
    ImpactEffects.Add(EVFX_ImpactType::Fall_Impact, FallData);

    // Rock Impact
    FVFX_ImpactData RockData;
    RockData.ParticleScale = 0.6f;
    RockData.EffectDuration = 1.0f;
    RockData.SpawnRadius = 60.0f;
    ImpactEffects.Add(EVFX_ImpactType::Rock_Impact, RockData);
}

void UVFX_ImpactManager::TriggerImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal, float IntensityMultiplier)
{
    if (!CachedWorld || !ShouldSpawnEffect(Location))
    {
        return;
    }

    const FVFX_ImpactData* EffectData = ImpactEffects.Find(ImpactType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No effect data found for impact type"));
        return;
    }

    float ScaledIntensity = EffectData->ParticleScale * IntensityMultiplier;

    // Spawn dust effect
    if (EffectData->DustEffect)
    {
        SpawnDustEffect(EffectData->DustEffect, Location, Normal, ScaledIntensity);
    }

    // Spawn debris effect
    if (EffectData->DebrisEffect)
    {
        SpawnDebrisEffect(EffectData->DebrisEffect, Location, Normal, ScaledIntensity);
    }

    // Play impact sound
    if (EffectData->ImpactSound)
    {
        PlayImpactSound(EffectData->ImpactSound, Location, IntensityMultiplier);
    }

    LastEffectTime = CachedWorld->GetTimeSeconds();
}

void UVFX_ImpactManager::TriggerFootstepEffect(float DinosaurMass, FVector Location, FVector Normal)
{
    EVFX_ImpactType ImpactType = GetImpactTypeFromMass(DinosaurMass);
    float IntensityMultiplier = FMath::Clamp(DinosaurMass / 1000.0f, 0.5f, 3.0f); // Scale based on mass
    TriggerImpactEffect(ImpactType, Location, Normal, IntensityMultiplier);
}

EVFX_ImpactType UVFX_ImpactManager::GetImpactTypeFromMass(float Mass)
{
    if (Mass < 100.0f) // Small dinosaurs, humans
    {
        return EVFX_ImpactType::Light_Footstep;
    }
    else if (Mass < 500.0f) // Medium dinosaurs
    {
        return EVFX_ImpactType::Medium_Footstep;
    }
    else if (Mass < 2000.0f) // Large theropods
    {
        return EVFX_ImpactType::Heavy_Footstep;
    }
    else // T-Rex, Sauropods
    {
        return EVFX_ImpactType::Massive_Footstep;
    }
}

bool UVFX_ImpactManager::ShouldSpawnEffect(FVector Location)
{
    if (!CachedWorld)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = CachedWorld->GetTimeSeconds();
    if (CurrentTime - LastEffectTime < MinTimeBetweenEffects)
    {
        return false;
    }

    // Check distance to player camera
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(CachedWorld, 0);
    if (CameraManager)
    {
        float DistanceToCamera = FVector::Dist(Location, CameraManager->GetCameraLocation());
        if (DistanceToCamera > EffectCullDistance)
        {
            return false;
        }
    }

    return true;
}

void UVFX_ImpactManager::SpawnDustEffect(UNiagaraSystem* Effect, FVector Location, FVector Normal, float Scale)
{
    if (!Effect || !CachedWorld)
    {
        return;
    }

    FRotator EffectRotation = Normal.Rotation();
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        CachedWorld, 
        Effect, 
        Location, 
        EffectRotation
    );

    if (NiagaraComp)
    {
        NiagaraComp->SetFloatParameter(FName("Scale"), Scale);
        NiagaraComp->SetVectorParameter(FName("ImpactNormal"), Normal);
        NiagaraComp->SetFloatParameter(FName("Intensity"), Scale);
    }
}

void UVFX_ImpactManager::SpawnDebrisEffect(UNiagaraSystem* Effect, FVector Location, FVector Normal, float Scale)
{
    if (!Effect || !CachedWorld)
    {
        return;
    }

    FRotator EffectRotation = Normal.Rotation();
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        CachedWorld, 
        Effect, 
        Location, 
        EffectRotation
    );

    if (NiagaraComp)
    {
        NiagaraComp->SetFloatParameter(FName("DebrisScale"), Scale);
        NiagaraComp->SetVectorParameter(FName("ImpactDirection"), Normal);
        NiagaraComp->SetFloatParameter(FName("SpreadRadius"), Scale * 50.0f);
    }
}

void UVFX_ImpactManager::PlayImpactSound(USoundCue* Sound, FVector Location, float VolumeMultiplier)
{
    if (!Sound || !CachedWorld)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        CachedWorld,
        Sound,
        Location,
        VolumeMultiplier,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation override
        nullptr, // Concurrency settings
        GetOwner() // Owner for spatialization
    );
}