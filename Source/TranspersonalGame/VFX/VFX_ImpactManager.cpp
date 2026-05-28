#include "VFX_ImpactManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize VFX settings
    bEnableVFXLOD = true;
    VFXCullingDistance = 5000.0f;
    MaxActiveEffects = 50;

    // Initialize impact settings
    ImpactSettings.EffectScale = 1.0f;
    ImpactSettings.EffectDuration = 2.0f;
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXAssets();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager initialized with LOD: %s, Max Effects: %d"), 
           bEnableVFXLOD ? TEXT("Enabled") : TEXT("Disabled"), MaxActiveEffects);
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Cleanup expired effects every few seconds
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 3.0f)
    {
        CleanupExpiredEffects();
        CleanupTimer = 0.0f;
    }
}

void AVFX_ImpactManager::InitializeVFXAssets()
{
    // Try to load default Niagara systems from Engine content
    // These paths should exist in most UE5 installations
    if (!ImpactSettings.DustEffect.IsValid())
    {
        UNiagaraSystem* DustSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDust"));
        if (DustSystem)
        {
            ImpactSettings.DustEffect = DustSystem;
            UE_LOG(LogTemp, Warning, TEXT("Loaded default dust effect"));
        }
    }

    if (!ImpactSettings.BloodEffect.IsValid())
    {
        UNiagaraSystem* BloodSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultBlood"));
        if (BloodSystem)
        {
            ImpactSettings.BloodEffect = BloodSystem;
            UE_LOG(LogTemp, Warning, TEXT("Loaded default blood effect"));
        }
    }

    if (!EnvironmentSettings.CampfireEffect.IsValid())
    {
        UNiagaraSystem* FireSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultFire"));
        if (FireSystem)
        {
            EnvironmentSettings.CampfireEffect = FireSystem;
            UE_LOG(LogTemp, Warning, TEXT("Loaded default fire effect"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX asset initialization complete"));
}

void AVFX_ImpactManager::SpawnDustImpact(const FVector& Location, float Scale)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }

    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            return; // Still at limit, skip this effect
        }
    }

    UNiagaraSystem* DustSystem = ImpactSettings.DustEffect.LoadSynchronous();
    if (DustSystem)
    {
        UNiagaraComponent* DustEffect = CreateNiagaraEffect(DustSystem, Location);
        if (DustEffect)
        {
            DustEffect->SetFloatParameter(TEXT("Scale"), Scale * ImpactSettings.EffectScale);
            DustEffect->SetFloatParameter(TEXT("Lifetime"), ImpactSettings.EffectDuration);
            
            // Apply LOD based on distance
            float Distance = FVector::Dist(Location, GetActorLocation());
            ApplyLODSettings(DustEffect, Distance);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned dust impact at %s with scale %f"), *Location.ToString(), Scale);
        }
    }
}

void AVFX_ImpactManager::SpawnBloodImpact(const FVector& Location, const FVector& Direction, float Scale)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }

    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            return;
        }
    }

    UNiagaraSystem* BloodSystem = ImpactSettings.BloodEffect.LoadSynchronous();
    if (BloodSystem)
    {
        FRotator BloodRotation = Direction.Rotation();
        UNiagaraComponent* BloodEffect = CreateNiagaraEffect(BloodSystem, Location, BloodRotation);
        if (BloodEffect)
        {
            BloodEffect->SetFloatParameter(TEXT("Scale"), Scale * ImpactSettings.EffectScale);
            BloodEffect->SetVectorParameter(TEXT("Direction"), Direction);
            
            float Distance = FVector::Dist(Location, GetActorLocation());
            ApplyLODSettings(BloodEffect, Distance);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned blood impact at %s"), *Location.ToString());
        }
    }
}

void AVFX_ImpactManager::SpawnWaterSplash(const FVector& Location, float Scale)
{
    if (!IsWithinCullingDistance(Location))
    {
        return;
    }

    UNiagaraSystem* WaterSystem = ImpactSettings.WaterSplashEffect.LoadSynchronous();
    if (WaterSystem)
    {
        UNiagaraComponent* WaterEffect = CreateNiagaraEffect(WaterSystem, Location);
        if (WaterEffect)
        {
            WaterEffect->SetFloatParameter(TEXT("Scale"), Scale * ImpactSettings.EffectScale);
            WaterEffect->SetFloatParameter(TEXT("SplashIntensity"), Scale);
            
            UE_LOG(LogTemp, Log, TEXT("Spawned water splash at %s"), *Location.ToString());
        }
    }
}

void AVFX_ImpactManager::SpawnCampfire(const FVector& Location)
{
    UNiagaraSystem* FireSystem = EnvironmentSettings.CampfireEffect.LoadSynchronous();
    if (FireSystem)
    {
        UNiagaraComponent* FireEffect = CreateNiagaraEffect(FireSystem, Location);
        if (FireEffect)
        {
            FireEffect->SetFloatParameter(TEXT("FireIntensity"), 1.0f);
            FireEffect->SetFloatParameter(TEXT("SmokeAmount"), 0.8f);
            
            // Campfires are persistent, don't add to cleanup list
            UE_LOG(LogTemp, Warning, TEXT("Spawned campfire at %s"), *Location.ToString());
        }
    }
}

void AVFX_ImpactManager::SpawnRainEffect(const FVector& Location, float Intensity)
{
    UNiagaraSystem* RainSystem = EnvironmentSettings.RainEffect.LoadSynchronous();
    if (RainSystem)
    {
        UNiagaraComponent* RainEffect = CreateNiagaraEffect(RainSystem, Location);
        if (RainEffect)
        {
            RainEffect->SetFloatParameter(TEXT("RainIntensity"), Intensity);
            RainEffect->SetFloatParameter(TEXT("DropletSize"), FMath::Lerp(0.5f, 2.0f, Intensity));
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned rain effect at %s with intensity %f"), *Location.ToString(), Intensity);
        }
    }
}

void AVFX_ImpactManager::SpawnVolcanicAsh(const FVector& Location, float Scale)
{
    UNiagaraSystem* AshSystem = EnvironmentSettings.VolcanicAshEffect.LoadSynchronous();
    if (AshSystem)
    {
        UNiagaraComponent* AshEffect = CreateNiagaraEffect(AshSystem, Location);
        if (AshEffect)
        {
            AshEffect->SetFloatParameter(TEXT("AshDensity"), Scale);
            AshEffect->SetFloatParameter(TEXT("WindStrength"), 0.7f);
            
            UE_LOG(LogTemp, Warning, TEXT("Spawned volcanic ash at %s"), *Location.ToString());
        }
    }
}

void AVFX_ImpactManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !IsValid(Effect) || !Effect->IsActive())
        {
            if (Effect && IsValid(Effect))
            {
                Effect->DestroyComponent();
            }
            ActiveEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleaned up expired VFX. Active effects: %d"), ActiveEffects.Num());
}

void AVFX_ImpactManager::SetVFXQuality(int32 QualityLevel)
{
    // Quality levels: 0=Low, 1=Medium, 2=High, 3=Epic
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 20;
            VFXCullingDistance = 2000.0f;
            break;
        case 1: // Medium
            MaxActiveEffects = 35;
            VFXCullingDistance = 3500.0f;
            break;
        case 2: // High
            MaxActiveEffects = 50;
            VFXCullingDistance = 5000.0f;
            break;
        case 3: // Epic
            MaxActiveEffects = 100;
            VFXCullingDistance = 8000.0f;
            break;
        default:
            QualityLevel = 2; // Default to High
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Quality set to %d - Max Effects: %d, Culling Distance: %f"), 
           QualityLevel, MaxActiveEffects, VFXCullingDistance);
}

UNiagaraComponent* AVFX_ImpactManager::CreateNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation)
{
    if (!System)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(), 
        System, 
        Location, 
        Rotation, 
        FVector(1.0f), 
        true, 
        EPSCPoolMethod::None,
        true
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        
        // Set auto-destroy timer
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
        {
            if (NiagaraComp && IsValid(NiagaraComp))
            {
                ActiveEffects.Remove(NiagaraComp);
                NiagaraComp->DestroyComponent();
            }
        }, ImpactSettings.EffectDuration, false);
    }

    return NiagaraComp;
}

bool AVFX_ImpactManager::IsWithinCullingDistance(const FVector& Location) const
{
    if (!bEnableVFXLOD)
    {
        return true;
    }

    // Get player camera location for distance calculation
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
        float Distance = FVector::Dist(CameraLocation, Location);
        return Distance <= VFXCullingDistance;
    }

    return true;
}

void AVFX_ImpactManager::ApplyLODSettings(UNiagaraComponent* Effect, float Distance) const
{
    if (!Effect || !bEnableVFXLOD)
    {
        return;
    }

    float DistanceRatio = Distance / VFXCullingDistance;
    
    // Reduce particle count and quality based on distance
    if (DistanceRatio > 0.7f)
    {
        // Far distance - low quality
        Effect->SetFloatParameter(TEXT("ParticleScale"), 0.5f);
        Effect->SetIntParameter(TEXT("ParticleCount"), 50);
    }
    else if (DistanceRatio > 0.4f)
    {
        // Medium distance - medium quality
        Effect->SetFloatParameter(TEXT("ParticleScale"), 0.75f);
        Effect->SetIntParameter(TEXT("ParticleCount"), 100);
    }
    // Close distance - full quality (no changes needed)
}