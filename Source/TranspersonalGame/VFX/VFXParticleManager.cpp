#include "VFXParticleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check for cleanup every second
    
    RainEffect = nullptr;
    
    InitializeDefaultEffects();
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Particle Manager initialized"));
}

void UVFX_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup expired effects periodically
    CleanupExpiredEffects();
}

void UVFX_ParticleManager::SpawnFootstepEffect(const FVector& Location, float ImpactForce, EBiomeType BiomeType)
{
    if (!FootstepEffects.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("No footstep effect configured for biome type"));
        return;
    }

    const FVFX_FootstepEffectData& FootstepData = FootstepEffects[BiomeType];
    
    if (ImpactForce < FootstepData.MinImpactForce)
    {
        return; // Impact too weak for VFX
    }

    // Calculate effect scale based on impact force
    FVector EffectScale = CalculateEffectScale(ImpactForce, FootstepData.MinImpactForce, FootstepData.MinImpactForce * 5.0f);

    // Spawn dust cloud
    if (FootstepData.DustCloud.ParticleSystem)
    {
        FVFX_ParticleEffectData DustData = FootstepData.DustCloud;
        DustData.Scale = EffectScale;
        UParticleSystemComponent* DustEffect = SpawnParticleEffect(DustData, Location);
        if (DustEffect)
        {
            ActiveEffects.Add(DustEffect);
        }
    }

    // Spawn ground cracks
    if (FootstepData.GroundCracks.ParticleSystem && ImpactForce > FootstepData.MinImpactForce * 2.0f)
    {
        FVFX_ParticleEffectData CrackData = FootstepData.GroundCracks;
        CrackData.Scale = EffectScale * 0.8f;
        UParticleSystemComponent* CrackEffect = SpawnParticleEffect(CrackData, Location);
        if (CrackEffect)
        {
            ActiveEffects.Add(CrackEffect);
        }
    }

    // Spawn debris scatter
    if (FootstepData.DebrisScatter.ParticleSystem)
    {
        FVFX_ParticleEffectData DebrisData = FootstepData.DebrisScatter;
        DebrisData.Scale = EffectScale * 1.2f;
        UParticleSystemComponent* DebrisEffect = SpawnParticleEffect(DebrisData, Location + FVector(0, 0, 10));
        if (DebrisEffect)
        {
            ActiveEffects.Add(DebrisEffect);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned footstep VFX at %s with impact force %f"), *Location.ToString(), ImpactForce);
}

void UVFX_ParticleManager::SpawnBloodEffect(const FVector& Location, const FVector& ImpactDirection, float DamageAmount)
{
    if (!BloodSplatter.ParticleSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Blood splatter particle system not configured"));
        return;
    }

    // Calculate blood effect intensity based on damage
    float BloodIntensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);
    
    // Spawn blood splatter
    FVFX_ParticleEffectData SplatterData = BloodSplatter;
    SplatterData.Scale = FVector(BloodIntensity, BloodIntensity, BloodIntensity);
    
    FRotator BloodRotation = ImpactDirection.Rotation();
    UParticleSystemComponent* SplatterEffect = SpawnParticleEffect(SplatterData, Location, BloodRotation);
    if (SplatterEffect)
    {
        ActiveEffects.Add(SplatterEffect);
    }

    // Spawn blood drops for severe damage
    if (DamageAmount > 50.0f && BloodDrops.ParticleSystem)
    {
        FVFX_ParticleEffectData DropsData = BloodDrops;
        DropsData.Scale = FVector(BloodIntensity * 0.7f, BloodIntensity * 0.7f, BloodIntensity * 0.7f);
        UParticleSystemComponent* DropsEffect = SpawnParticleEffect(DropsData, Location + FVector(0, 0, 20));
        if (DropsEffect)
        {
            ActiveEffects.Add(DropsEffect);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned blood VFX at %s with damage %f"), *Location.ToString(), DamageAmount);
}

void UVFX_ParticleManager::SpawnCampfireEffect(const FVector& Location)
{
    // Spawn flames
    if (CampfireFlames.ParticleSystem)
    {
        UParticleSystemComponent* FlameEffect = SpawnParticleEffect(CampfireFlames, Location);
        if (FlameEffect)
        {
            FlameEffect->SetAutoDestroy(false); // Campfire effects persist
            CampfireEffects.Add(FlameEffect);
        }
    }

    // Spawn smoke
    if (CampfireSmoke.ParticleSystem)
    {
        UParticleSystemComponent* SmokeEffect = SpawnParticleEffect(CampfireSmoke, Location + FVector(0, 0, 30));
        if (SmokeEffect)
        {
            SmokeEffect->SetAutoDestroy(false);
            CampfireEffects.Add(SmokeEffect);
        }
    }

    // Spawn embers
    if (CampfireEmbers.ParticleSystem)
    {
        UParticleSystemComponent* EmberEffect = SpawnParticleEffect(CampfireEmbers, Location + FVector(0, 0, 15));
        if (EmberEffect)
        {
            EmberEffect->SetAutoDestroy(false);
            CampfireEffects.Add(EmberEffect);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned campfire VFX at %s"), *Location.ToString());
}

void UVFX_ParticleManager::StopCampfireEffect(const FVector& Location)
{
    // Find and stop campfire effects near the location
    for (int32 i = CampfireEffects.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* Effect = CampfireEffects[i];
        if (Effect && IsValid(Effect))
        {
            FVector EffectLocation = Effect->GetComponentLocation();
            float Distance = FVector::Dist(EffectLocation, Location);
            if (Distance < 100.0f) // Within 1 meter
            {
                Effect->Deactivate();
                Effect->DestroyComponent();
                CampfireEffects.RemoveAt(i);
            }
        }
        else
        {
            CampfireEffects.RemoveAt(i);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Stopped campfire VFX near %s"), *Location.ToString());
}

void UVFX_ParticleManager::StartRainEffect(float Intensity)
{
    if (RainEffect && IsValid(RainEffect))
    {
        StopRainEffect();
    }

    if (!RainDrops.ParticleSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rain particle system not configured"));
        return;
    }

    // Spawn rain above the player
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager has no owner for rain effect"));
        return;
    }

    FVector RainLocation = Owner->GetActorLocation() + FVector(0, 0, 1000); // 10 meters above
    
    FVFX_ParticleEffectData RainData = RainDrops;
    RainData.Scale = FVector(Intensity, Intensity, Intensity);
    RainData.bAutoDestroy = false;
    
    RainEffect = SpawnParticleEffect(RainData, RainLocation);
    if (RainEffect)
    {
        RainEffect->SetAutoDestroy(false);
    }

    UE_LOG(LogTemp, Log, TEXT("Started rain VFX with intensity %f"), Intensity);
}

void UVFX_ParticleManager::StopRainEffect()
{
    if (RainEffect && IsValid(RainEffect))
    {
        RainEffect->Deactivate();
        RainEffect->DestroyComponent();
        RainEffect = nullptr;
        UE_LOG(LogTemp, Log, TEXT("Stopped rain VFX"));
    }
}

void UVFX_ParticleManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UParticleSystemComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
}

int32 UVFX_ParticleManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num() + CampfireEffects.Num() + (RainEffect ? 1 : 0);
}

UParticleSystemComponent* UVFX_ParticleManager::SpawnParticleEffect(const FVFX_ParticleEffectData& EffectData, const FVector& Location, const FRotator& Rotation)
{
    if (!EffectData.ParticleSystem)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
        World,
        EffectData.ParticleSystem,
        Location,
        Rotation,
        EffectData.Scale,
        EffectData.bAutoDestroy
    );

    return ParticleComponent;
}

void UVFX_ParticleManager::InitializeDefaultEffects()
{
    // Initialize default footstep effects for each biome
    FVFX_FootstepEffectData SavanaFootstep;
    SavanaFootstep.MinImpactForce = 100.0f;
    SavanaFootstep.MaxEffectRadius = 300.0f;
    FootstepEffects.Add(EBiomeType::Savana, SavanaFootstep);

    FVFX_FootstepEffectData ForestFootstep;
    ForestFootstep.MinImpactForce = 80.0f;
    ForestFootstep.MaxEffectRadius = 250.0f;
    FootstepEffects.Add(EBiomeType::Forest, ForestFootstep);

    FVFX_FootstepEffectData DesertFootstep;
    DesertFootstep.MinImpactForce = 120.0f;
    DesertFootstep.MaxEffectRadius = 400.0f;
    FootstepEffects.Add(EBiomeType::Desert, DesertFootstep);

    FVFX_FootstepEffectData SwampFootstep;
    SwampFootstep.MinImpactForce = 90.0f;
    SwampFootstep.MaxEffectRadius = 200.0f;
    FootstepEffects.Add(EBiomeType::Swamp, SwampFootstep);

    FVFX_FootstepEffectData MountainFootstep;
    MountainFootstep.MinImpactForce = 150.0f;
    MountainFootstep.MaxEffectRadius = 350.0f;
    FootstepEffects.Add(EBiomeType::Mountain, MountainFootstep);

    // Initialize default blood effects
    BloodSplatter.Duration = 5.0f;
    BloodSplatter.Scale = FVector(1.0f, 1.0f, 1.0f);
    BloodSplatter.bAutoDestroy = true;

    BloodDrops.Duration = 3.0f;
    BloodDrops.Scale = FVector(0.8f, 0.8f, 0.8f);
    BloodDrops.bAutoDestroy = true;

    // Initialize default fire effects
    CampfireFlames.Duration = 0.0f; // Infinite
    CampfireFlames.Scale = FVector(1.0f, 1.0f, 1.0f);
    CampfireFlames.bAutoDestroy = false;

    CampfireSmoke.Duration = 0.0f;
    CampfireSmoke.Scale = FVector(1.2f, 1.2f, 1.2f);
    CampfireSmoke.bAutoDestroy = false;

    CampfireEmbers.Duration = 0.0f;
    CampfireEmbers.Scale = FVector(0.8f, 0.8f, 0.8f);
    CampfireEmbers.bAutoDestroy = false;

    // Initialize default weather effects
    RainDrops.Duration = 0.0f;
    RainDrops.Scale = FVector(2.0f, 2.0f, 1.0f);
    RainDrops.bAutoDestroy = false;

    RainSplash.Duration = 2.0f;
    RainSplash.Scale = FVector(1.0f, 1.0f, 1.0f);
    RainSplash.bAutoDestroy = true;
}

FVector UVFX_ParticleManager::CalculateEffectScale(float ImpactForce, float MinForce, float MaxForce) const
{
    float ScaleFactor = FMath::Clamp((ImpactForce - MinForce) / (MaxForce - MinForce), 0.1f, 2.0f);
    return FVector(ScaleFactor, ScaleFactor, ScaleFactor);
}

void UVFX_ParticleManager::RemoveExpiredEffect(UParticleSystemComponent* Effect)
{
    if (Effect && IsValid(Effect))
    {
        Effect->DestroyComponent();
    }
    ActiveEffects.Remove(Effect);
}