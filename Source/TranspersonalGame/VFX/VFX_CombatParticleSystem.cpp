#include "VFX_CombatParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVFX_CombatParticleSystem::UVFX_CombatParticleSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    PoolIndex = 0;
    
    // Initialize effect pool
    EffectPool.Reserve(MAX_POOL_SIZE);
}

void UVFX_CombatParticleSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Load default Niagara systems
    BloodImpactEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/DefaultSystems/NS_BloodImpact"));
    DustCloudEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/DefaultSystems/NS_DustCloud"));
    SparkEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/DefaultSystems/NS_Sparks"));
    WaterSplashEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/DefaultSystems/NS_WaterSplash"));
    
    // Pre-populate effect pool
    for (int32 i = 0; i < MAX_POOL_SIZE; ++i)
    {
        UNiagaraComponent* PooledEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), 
            BloodImpactEffect, 
            FVector::ZeroVector, 
            FRotator::ZeroRotator, 
            FVector::ZeroVector, 
            true, 
            true, 
            ENCPoolMethod::AutoRelease
        );
        
        if (PooledEffect)
        {
            PooledEffect->SetAutoDestroy(false);
            PooledEffect->Deactivate();
            EffectPool.Add(PooledEffect);
        }
    }
}

void UVFX_CombatParticleSystem::TriggerBloodImpact(FVector Location, FVector Normal, float Intensity)
{
    if (!BloodImpactEffect) return;
    
    FRotator Rotation = Normal.Rotation();
    FVector Scale = FVector(Intensity, Intensity, Intensity);
    
    SpawnNiagaraEffect(BloodImpactEffect, Location, Rotation, Scale);
    
    // Play blood impact sound
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Location, 0.8f * Intensity);
}

void UVFX_CombatParticleSystem::TriggerDustCloud(FVector Location, float Scale)
{
    if (!DustCloudEffect) return;
    
    FVector ScaleVector = FVector(Scale, Scale, Scale * 0.5f);
    SpawnNiagaraEffect(DustCloudEffect, Location, FRotator::ZeroRotator, ScaleVector);
}

void UVFX_CombatParticleSystem::TriggerSparks(FVector Location, FVector Direction, int32 SparkCount)
{
    if (!SparkEffect) return;
    
    FRotator Rotation = Direction.Rotation();
    
    UNiagaraComponent* SparkComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        SparkEffect, 
        Location, 
        Rotation, 
        FVector::OneVector, 
        true, 
        true, 
        ENCPoolMethod::AutoRelease
    );
    
    if (SparkComponent)
    {
        SparkComponent->SetIntParameter(TEXT("SparkCount"), SparkCount);
        SparkComponent->SetVectorParameter(TEXT("Direction"), Direction);
    }
}

void UVFX_CombatParticleSystem::TriggerWaterSplash(FVector Location, float Velocity)
{
    if (!WaterSplashEffect) return;
    
    UNiagaraComponent* SplashComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        WaterSplashEffect, 
        Location, 
        FRotator::ZeroRotator, 
        FVector::OneVector, 
        true, 
        true, 
        ENCPoolMethod::AutoRelease
    );
    
    if (SplashComponent)
    {
        SplashComponent->SetFloatParameter(TEXT("Velocity"), Velocity);
    }
}

void UVFX_CombatParticleSystem::TriggerDinosaurFootstep(FVector Location, float DinosaurSize)
{
    // Create dust cloud scaled to dinosaur size
    TriggerDustCloud(Location, DinosaurSize);
    
    // Add ground shake effect for large dinosaurs
    if (DinosaurSize > 2.0f)
    {
        // Trigger screen shake or ground particles
        FVector ShakeLocation = Location + FVector(0, 0, -50);
        TriggerDustCloud(ShakeLocation, DinosaurSize * 0.5f);
    }
}

void UVFX_CombatParticleSystem::TriggerDinosaurBreath(FVector Location, FVector Direction, bool bColdWeather)
{
    if (bColdWeather)
    {
        // Create visible breath vapor in cold weather
        UNiagaraComponent* BreathComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), 
            DustCloudEffect, // Reuse dust system for breath vapor
            Location, 
            Direction.Rotation(), 
            FVector(0.3f, 0.3f, 0.3f), 
            true, 
            true, 
            ENCPoolMethod::AutoRelease
        );
        
        if (BreathComponent)
        {
            BreathComponent->SetColorParameter(TEXT("Color"), FLinearColor::White);
        }
    }
}

void UVFX_CombatParticleSystem::TriggerDinosaurRoar(FVector Location, float IntensityRadius)
{
    // Create air distortion effect for powerful roars
    TriggerDustCloud(Location, IntensityRadius / 1000.0f);
    
    // Add particle burst effect
    for (int32 i = 0; i < 8; ++i)
    {
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(0.0f, 1.0f)
        ).GetSafeNormal();
        
        FVector SpawnLocation = Location + RandomDirection * 100.0f;
        TriggerDustCloud(SpawnLocation, 0.5f);
    }
}

void UVFX_CombatParticleSystem::SpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!System || !GetWorld()) return;
    
    // Use pooled effect if available
    if (EffectPool.Num() > 0)
    {
        UNiagaraComponent* PooledEffect = EffectPool[PoolIndex % EffectPool.Num()];
        PoolIndex = (PoolIndex + 1) % EffectPool.Num();
        
        if (PooledEffect && IsValid(PooledEffect))
        {
            PooledEffect->SetWorldLocationAndRotation(Location, Rotation);
            PooledEffect->SetWorldScale3D(Scale);
            PooledEffect->SetAsset(System);
            PooledEffect->Activate();
            return;
        }
    }
    
    // Fallback to direct spawn
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        System, 
        Location, 
        Rotation, 
        Scale, 
        true, 
        true, 
        ENCPoolMethod::AutoRelease
    );
}