#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second for cleanup
    
    InitializeParticleDatabase();
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize particle database with default values
    InitializeParticleDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Initialized with %d particle types"), ParticleDatabase.Num());
}

void UVFX_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Cleanup expired particles every tick
    RemoveNullParticles();
}

void UVFX_ParticleManager::InitializeParticleDatabase()
{
    // Initialize all particle types with default data
    // In production, these would reference actual Niagara assets
    
    FVFX_ParticleData CampfireData;
    CampfireData.DefaultLifetime = -1.0f; // Persistent
    CampfireData.bAutoDestroy = false;
    CampfireData.DefaultScale = FVector(1.5f, 1.5f, 2.0f);
    ParticleDatabase.Add(EVFX_ParticleType::Campfire, CampfireData);
    
    FVFX_ParticleData DustData;
    DustData.DefaultLifetime = 3.0f;
    DustData.bAutoDestroy = true;
    DustData.DefaultScale = FVector(2.0f, 2.0f, 1.0f);
    ParticleDatabase.Add(EVFX_ParticleType::DinosaurDust, DustData);
    
    FVFX_ParticleData BloodData;
    BloodData.DefaultLifetime = 2.0f;
    BloodData.bAutoDestroy = true;
    BloodData.DefaultScale = FVector(1.0f, 1.0f, 1.0f);
    ParticleDatabase.Add(EVFX_ParticleType::BloodSplatter, BloodData);
    
    FVFX_ParticleData WaterData;
    WaterData.DefaultLifetime = 4.0f;
    WaterData.bAutoDestroy = true;
    WaterData.DefaultScale = FVector(1.5f, 1.5f, 1.5f);
    ParticleDatabase.Add(EVFX_ParticleType::WaterSplash, WaterData);
    
    FVFX_ParticleData VaporData;
    VaporData.DefaultLifetime = 2.5f;
    VaporData.bAutoDestroy = true;
    VaporData.DefaultScale = FVector(0.8f, 0.8f, 1.2f);
    ParticleDatabase.Add(EVFX_ParticleType::BreathVapor, VaporData);
    
    FVFX_ParticleData RainData;
    RainData.DefaultLifetime = -1.0f; // Persistent weather
    RainData.bAutoDestroy = false;
    RainData.DefaultScale = FVector(10.0f, 10.0f, 5.0f);
    ParticleDatabase.Add(EVFX_ParticleType::WeatherRain, RainData);
    
    FVFX_ParticleData AshData;
    AshData.DefaultLifetime = 15.0f;
    AshData.bAutoDestroy = true;
    AshData.DefaultScale = FVector(5.0f, 5.0f, 3.0f);
    ParticleDatabase.Add(EVFX_ParticleType::VolcanicAsh, AshData);
    
    FVFX_ParticleData InsectData;
    InsectData.DefaultLifetime = 8.0f;
    InsectData.bAutoDestroy = true;
    InsectData.DefaultScale = FVector(3.0f, 3.0f, 2.0f);
    ParticleDatabase.Add(EVFX_ParticleType::InsectSwarm, InsectData);
}

UNiagaraComponent* UVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!ParticleDatabase.Contains(ParticleType))
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleManager: Unknown particle type requested"));
        return nullptr;
    }
    
    const FVFX_ParticleData& ParticleData = ParticleDatabase[ParticleType];
    
    // For now, create a basic Niagara component without specific system
    // In production, this would load the actual Niagara asset
    UNiagaraComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        nullptr, // Would be ParticleData.NiagaraSystem in production
        Location,
        Rotation,
        Scale.IsZero() ? ParticleData.DefaultScale : Scale,
        true // Auto destroy
    );
    
    if (ParticleComponent)
    {
        ActiveParticles.Add(ParticleComponent);
        
        // Set auto-destroy timer if specified
        if (ParticleData.bAutoDestroy && ParticleData.DefaultLifetime > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ParticleComponent]()
            {
                if (IsValid(ParticleComponent))
                {
                    ParticleComponent->DestroyComponent();
                }
            }, ParticleData.DefaultLifetime, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect at location %s"), *Location.ToString());
    }
    
    return ParticleComponent;
}

void UVFX_ParticleManager::SpawnCampfireEffect(FVector Location)
{
    SpawnParticleEffect(EVFX_ParticleType::Campfire, Location);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Campfire effect spawned at %s"), *Location.ToString());
}

void UVFX_ParticleManager::SpawnDinosaurDustEffect(FVector Location, FVector Direction)
{
    FRotator DustRotation = Direction.Rotation();
    SpawnParticleEffect(EVFX_ParticleType::DinosaurDust, Location, DustRotation);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Dinosaur dust effect spawned"));
}

void UVFX_ParticleManager::SpawnBloodSplatterEffect(FVector Location, FVector ImpactDirection)
{
    FRotator BloodRotation = ImpactDirection.Rotation();
    SpawnParticleEffect(EVFX_ParticleType::BloodSplatter, Location, BloodRotation);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Blood splatter effect spawned"));
}

void UVFX_ParticleManager::SpawnWaterSplashEffect(FVector Location, float Intensity)
{
    FVector SplashScale = FVector(Intensity, Intensity, Intensity);
    SpawnParticleEffect(EVFX_ParticleType::WaterSplash, Location, FRotator::ZeroRotator, SplashScale);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Water splash effect spawned with intensity %f"), Intensity);
}

void UVFX_ParticleManager::SpawnBreathVaporEffect(FVector Location, FVector Direction)
{
    FRotator VaporRotation = Direction.Rotation();
    SpawnParticleEffect(EVFX_ParticleType::BreathVapor, Location, VaporRotation);
}

void UVFX_ParticleManager::SpawnWeatherRainEffect(FVector Location, float Intensity)
{
    FVector RainScale = FVector(Intensity * 10.0f, Intensity * 10.0f, Intensity * 5.0f);
    SpawnParticleEffect(EVFX_ParticleType::WeatherRain, Location, FRotator::ZeroRotator, RainScale);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Weather rain effect spawned"));
}

void UVFX_ParticleManager::SpawnVolcanicAshEffect(FVector Location, FVector WindDirection)
{
    FRotator AshRotation = WindDirection.Rotation();
    SpawnParticleEffect(EVFX_ParticleType::VolcanicAsh, Location, AshRotation);
}

void UVFX_ParticleManager::SpawnInsectSwarmEffect(FVector Location, float SwarmSize)
{
    FVector SwarmScale = FVector(SwarmSize * 3.0f, SwarmSize * 3.0f, SwarmSize * 2.0f);
    SpawnParticleEffect(EVFX_ParticleType::InsectSwarm, Location, FRotator::ZeroRotator, SwarmScale);
}

void UVFX_ParticleManager::CleanupExpiredParticles()
{
    RemoveNullParticles();
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Cleanup completed, %d active particles remaining"), ActiveParticles.Num());
}

void UVFX_ParticleManager::StopAllParticleEffects()
{
    for (UNiagaraComponent* Particle : ActiveParticles)
    {
        if (IsValid(Particle))
        {
            Particle->DestroyComponent();
        }
    }
    ActiveParticles.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: All particle effects stopped"));
}

int32 UVFX_ParticleManager::GetActiveParticleCount() const
{
    return ActiveParticles.Num();
}

void UVFX_ParticleManager::RemoveNullParticles()
{
    ActiveParticles.RemoveAll([](UNiagaraComponent* Particle)
    {
        return !IsValid(Particle);
    });
}