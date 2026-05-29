#include "EnvArt_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

DEFINE_LOG_CATEGORY(LogEnvArtParticles);

UEnvArt_ParticleManager::UEnvArt_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick 10 times per second for performance
    
    // Default configuration
    UpdateRadius = 5000.0f;
    MaxActiveParticleSystems = 20;
    bUseNiagaraSystem = true;
    
    // Initialize default wind
    CurrentWindDirection = FVector(1.0f, 0.0f, 0.0f);
    CurrentWindStrength = 1.0f;
}

void UEnvArt_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Initializing particle manager"));
    
    InitializeParticleManager();
}

void UEnvArt_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ParticleUpdateTimer += DeltaTime;
    ParticleSystemsSpawnedThisFrame = 0;
    
    // Update particles periodically for performance
    if (ParticleUpdateTimer >= ParticleUpdateInterval)
    {
        ParticleUpdateTimer = 0.0f;
        
        // Get player location
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            LastPlayerLocation = PlayerLocation;
            
            // Clean up distant particles
            CleanupDistantParticles(PlayerLocation);
            
            // Update particles for current biome
            UpdateParticlesForBiome(CurrentBiome, PlayerLocation);
        }
    }
}

void UEnvArt_ParticleManager::InitializeParticleManager()
{
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Initializing biome particle configurations"));
    
    InitializeBiomeParticleConfigs();
    
    // Clear any existing particle systems
    ActiveParticleSystems.Empty();
    
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Particle manager initialized successfully"));
}

void UEnvArt_ParticleManager::InitializeBiomeParticleConfigs()
{
    // Savanna configuration - dusty, dry atmosphere
    FEnvArt_ParticleConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.bEnableDust = true;
    SavannaConfig.DustSpawnRate = 8.0f;
    SavannaConfig.DustVelocity = FVector(15.0f, 0.0f, 8.0f);
    SavannaConfig.DustColor = FLinearColor(0.9f, 0.8f, 0.6f, 0.4f);
    SavannaConfig.bEnablePollen = false;
    SavannaConfig.bEnableMist = false;
    SavannaConfig.WindSensitivity = 1.5f;
    SavannaConfig.SpawnAreaSize = FVector(3000.0f, 3000.0f, 800.0f);
    BiomeParticleConfigs.Add(EBiomeType::Savanna, SavannaConfig);
    
    // Forest configuration - pollen and mist
    FEnvArt_ParticleConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.bEnableDust = false;
    ForestConfig.bEnablePollen = true;
    ForestConfig.PollenSpawnRate = 5.0f;
    ForestConfig.PollenVelocity = FVector(8.0f, 0.0f, 3.0f);
    ForestConfig.PollenColor = FLinearColor(1.0f, 1.0f, 0.7f, 0.3f);
    ForestConfig.bEnableMist = true;
    ForestConfig.MistSpawnRate = 2.0f;
    ForestConfig.MistColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.2f);
    ForestConfig.WindSensitivity = 0.8f;
    ForestConfig.SpawnAreaSize = FVector(2500.0f, 2500.0f, 1200.0f);
    BiomeParticleConfigs.Add(EBiomeType::Forest, ForestConfig);
    
    // Desert configuration - heavy dust and sand
    FEnvArt_ParticleConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.bEnableDust = true;
    DesertConfig.DustSpawnRate = 12.0f;
    DesertConfig.DustVelocity = FVector(20.0f, 0.0f, 10.0f);
    DesertConfig.DustColor = FLinearColor(1.0f, 0.9f, 0.7f, 0.5f);
    DesertConfig.bEnablePollen = false;
    DesertConfig.bEnableMist = false;
    DesertConfig.WindSensitivity = 2.0f;
    DesertConfig.SpawnAreaSize = FVector(4000.0f, 4000.0f, 600.0f);
    BiomeParticleConfigs.Add(EBiomeType::Desert, DesertConfig);
    
    // Wetland configuration - mist and humidity effects
    FEnvArt_ParticleConfig WetlandConfig;
    WetlandConfig.BiomeType = EBiomeType::Wetland;
    WetlandConfig.bEnableDust = false;
    WetlandConfig.bEnablePollen = true;
    WetlandConfig.PollenSpawnRate = 3.0f;
    WetlandConfig.bEnableMist = true;
    WetlandConfig.MistSpawnRate = 4.0f;
    WetlandConfig.MistColor = FLinearColor(0.9f, 0.95f, 1.0f, 0.3f);
    WetlandConfig.WindSensitivity = 0.5f;
    WetlandConfig.SpawnAreaSize = FVector(2000.0f, 2000.0f, 800.0f);
    BiomeParticleConfigs.Add(EBiomeType::Wetland, WetlandConfig);
    
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Configured %d biome particle settings"), BiomeParticleConfigs.Num());
}

void UEnvArt_ParticleManager::UpdateParticlesForBiome(EBiomeType BiomeType, const FVector& PlayerLocation)
{
    if (ParticleSystemsSpawnedThisFrame >= MaxParticleSystemsPerFrame)
    {
        return; // Performance limit reached
    }
    
    CurrentBiome = BiomeType;
    
    const FEnvArt_ParticleConfig* Config = BiomeParticleConfigs.Find(BiomeType);
    if (!Config)
    {
        UE_LOG(LogEnvArtParticles, Warning, TEXT("EnvArt_ParticleManager: No particle config found for biome type %d"), (int32)BiomeType);
        return;
    }
    
    // Spawn particles around player location
    FVector SpawnLocation = PlayerLocation + FVector(
        FMath::RandRange(-Config->SpawnAreaSize.X * 0.5f, Config->SpawnAreaSize.X * 0.5f),
        FMath::RandRange(-Config->SpawnAreaSize.Y * 0.5f, Config->SpawnAreaSize.Y * 0.5f),
        FMath::RandRange(0.0f, Config->SpawnAreaSize.Z)
    );
    
    // Spawn appropriate particle types based on configuration
    if (Config->bEnableDust && FMath::RandRange(0.0f, 10.0f) < Config->DustSpawnRate)
    {
        SpawnDustParticles(SpawnLocation, *Config);
    }
    
    if (Config->bEnablePollen && FMath::RandRange(0.0f, 10.0f) < Config->PollenSpawnRate)
    {
        SpawnPollenParticles(SpawnLocation, *Config);
    }
    
    if (Config->bEnableMist && FMath::RandRange(0.0f, 10.0f) < Config->MistSpawnRate)
    {
        SpawnMistParticles(SpawnLocation, *Config);
    }
}

void UEnvArt_ParticleManager::SpawnDustParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config)
{
    if (!bUseNiagaraSystem || !DustParticleSystem)
    {
        UE_LOG(LogEnvArtParticles, Warning, TEXT("EnvArt_ParticleManager: Dust particle system not available"));
        return;
    }
    
    if (ActiveParticleSystems.Num() >= MaxActiveParticleSystems)
    {
        RemoveOldestParticleSystem();
    }
    
    // Spawn Niagara dust particles
    UNiagaraComponent* DustComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        DustParticleSystem,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (DustComponent)
    {
        UpdateParticleParameters(DustComponent, Config);
        ActiveParticleSystems.Add(DustComponent);
        ParticleSystemsSpawnedThisFrame++;
        
        UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Spawned dust particles at %s"), *Location.ToString());
    }
}

void UEnvArt_ParticleManager::SpawnPollenParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config)
{
    if (!bUseNiagaraSystem || !PollenParticleSystem)
    {
        UE_LOG(LogEnvArtParticles, Warning, TEXT("EnvArt_ParticleManager: Pollen particle system not available"));
        return;
    }
    
    if (ActiveParticleSystems.Num() >= MaxActiveParticleSystems)
    {
        RemoveOldestParticleSystem();
    }
    
    // Spawn Niagara pollen particles
    UNiagaraComponent* PollenComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        PollenParticleSystem,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (PollenComponent)
    {
        UpdateParticleParameters(PollenComponent, Config);
        ActiveParticleSystems.Add(PollenComponent);
        ParticleSystemsSpawnedThisFrame++;
        
        UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Spawned pollen particles at %s"), *Location.ToString());
    }
}

void UEnvArt_ParticleManager::SpawnMistParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config)
{
    if (!bUseNiagaraSystem || !MistParticleSystem)
    {
        UE_LOG(LogEnvArtParticles, Warning, TEXT("EnvArt_ParticleManager: Mist particle system not available"));
        return;
    }
    
    if (ActiveParticleSystems.Num() >= MaxActiveParticleSystems)
    {
        RemoveOldestParticleSystem();
    }
    
    // Spawn Niagara mist particles
    UNiagaraComponent* MistComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        MistParticleSystem,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::None,
        true
    );
    
    if (MistComponent)
    {
        UpdateParticleParameters(MistComponent, Config);
        ActiveParticleSystems.Add(MistComponent);
        ParticleSystemsSpawnedThisFrame++;
        
        UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Spawned mist particles at %s"), *Location.ToString());
    }
}

void UEnvArt_ParticleManager::UpdateWindEffects(const FVector& WindDirection, float WindStrength)
{
    CurrentWindDirection = WindDirection.GetSafeNormal();
    CurrentWindStrength = FMath::Clamp(WindStrength, 0.0f, 5.0f);
    
    // Update all active particle systems with new wind parameters
    for (UNiagaraComponent* ParticleComponent : ActiveParticleSystems)
    {
        if (IsValid(ParticleComponent))
        {
            ParticleComponent->SetVectorParameter(FName("WindDirection"), CurrentWindDirection);
            ParticleComponent->SetFloatParameter(FName("WindStrength"), CurrentWindStrength);
        }
    }
    
    UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Updated wind effects - Direction: %s, Strength: %f"), 
           *CurrentWindDirection.ToString(), CurrentWindStrength);
}

void UEnvArt_ParticleManager::CreateGoldenHourParticles(const FVector& SunDirection)
{
    // Create enhanced dust particles for golden hour lighting
    FEnvArt_ParticleConfig GoldenHourConfig;
    GoldenHourConfig.bEnableDust = true;
    GoldenHourConfig.DustSpawnRate = 15.0f;
    GoldenHourConfig.DustColor = FLinearColor(1.0f, 0.8f, 0.4f, 0.6f); // Golden tint
    GoldenHourConfig.DustVelocity = FVector(5.0f, 0.0f, 2.0f); // Slower, more visible
    GoldenHourConfig.SpawnAreaSize = FVector(1500.0f, 1500.0f, 400.0f);
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Spawn particles in sun rays
        for (int32 i = 0; i < 5; i++)
        {
            FVector SpawnLocation = PlayerLocation + SunDirection * FMath::RandRange(500.0f, 2000.0f);
            SpawnLocation.Z += FMath::RandRange(100.0f, 500.0f);
            
            SpawnDustParticles(SpawnLocation, GoldenHourConfig);
        }
    }
    
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Created golden hour particle effects"));
}

void UEnvArt_ParticleManager::CreateForestMistEffect(const FVector& ForestCenter, float MistRadius)
{
    FEnvArt_ParticleConfig MistConfig;
    MistConfig.bEnableMist = true;
    MistConfig.MistSpawnRate = 8.0f;
    MistConfig.MistColor = FLinearColor(0.9f, 0.95f, 1.0f, 0.4f);
    MistConfig.MistLifetime = 30.0f;
    MistConfig.SpawnAreaSize = FVector(MistRadius * 2.0f, MistRadius * 2.0f, 200.0f);
    
    // Spawn mist particles in a circle around the forest center
    int32 NumMistPoints = 8;
    for (int32 i = 0; i < NumMistPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumMistPoints;
        FVector MistLocation = ForestCenter + FVector(
            FMath::Cos(Angle) * MistRadius * 0.7f,
            FMath::Sin(Angle) * MistRadius * 0.7f,
            FMath::RandRange(50.0f, 150.0f)
        );
        
        SpawnMistParticles(MistLocation, MistConfig);
    }
    
    UE_LOG(LogEnvArtParticles, Log, TEXT("EnvArt_ParticleManager: Created forest mist effect at %s with radius %f"), 
           *ForestCenter.ToString(), MistRadius);
}

void UEnvArt_ParticleManager::SetParticleIntensity(float Intensity)
{
    float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    // Update all active particle systems
    for (UNiagaraComponent* ParticleComponent : ActiveParticleSystems)
    {
        if (IsValid(ParticleComponent))
        {
            ParticleComponent->SetFloatParameter(FName("Intensity"), ClampedIntensity);
        }
    }
    
    UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Set particle intensity to %f"), ClampedIntensity);
}

void UEnvArt_ParticleManager::CleanupDistantParticles(const FVector& PlayerLocation)
{
    TArray<UNiagaraComponent*> ParticlesToRemove;
    
    for (UNiagaraComponent* ParticleComponent : ActiveParticleSystems)
    {
        if (!IsValid(ParticleComponent))
        {
            ParticlesToRemove.Add(ParticleComponent);
            continue;
        }
        
        FVector ParticleLocation = ParticleComponent->GetComponentLocation();
        float Distance = FVector::Dist(PlayerLocation, ParticleLocation);
        
        if (Distance > UpdateRadius)
        {
            ParticlesToRemove.Add(ParticleComponent);
        }
    }
    
    // Remove distant or invalid particles
    for (UNiagaraComponent* ParticleToRemove : ParticlesToRemove)
    {
        ActiveParticleSystems.Remove(ParticleToRemove);
        if (IsValid(ParticleToRemove))
        {
            ParticleToRemove->DestroyComponent();
        }
    }
    
    if (ParticlesToRemove.Num() > 0)
    {
        UE_LOG(LogEnvArtParticles, VeryVerbose, TEXT("EnvArt_ParticleManager: Cleaned up %d distant particle systems"), ParticlesToRemove.Num());
    }
}

void UEnvArt_ParticleManager::UpdateParticleParameters(UNiagaraComponent* Component, const FEnvArt_ParticleConfig& Config)
{
    if (!IsValid(Component))
    {
        return;
    }
    
    // Set common parameters
    Component->SetVectorParameter(FName("Velocity"), Config.DustVelocity);
    Component->SetVectorParameter(FName("WindDirection"), CurrentWindDirection);
    Component->SetFloatParameter(FName("WindStrength"), CurrentWindStrength * Config.WindSensitivity);
    Component->SetFloatParameter(FName("Lifetime"), Config.ParticleLifetime);
    
    // Set color parameters based on particle type
    if (Config.bEnableDust)
    {
        Component->SetVectorParameter(FName("ParticleColor"), FVector(Config.DustColor.R, Config.DustColor.G, Config.DustColor.B));
        Component->SetFloatParameter(FName("ParticleAlpha"), Config.DustColor.A);
    }
    else if (Config.bEnablePollen)
    {
        Component->SetVectorParameter(FName("ParticleColor"), FVector(Config.PollenColor.R, Config.PollenColor.G, Config.PollenColor.B));
        Component->SetFloatParameter(FName("ParticleAlpha"), Config.PollenColor.A);
    }
    else if (Config.bEnableMist)
    {
        Component->SetVectorParameter(FName("ParticleColor"), FVector(Config.MistColor.R, Config.MistColor.G, Config.MistColor.B));
        Component->SetFloatParameter(FName("ParticleAlpha"), Config.MistColor.A);
    }
}

bool UEnvArt_ParticleManager::IsLocationInRange(const FVector& Location, const FVector& PlayerLocation) const
{
    return FVector::Dist(Location, PlayerLocation) <= UpdateRadius;
}

void UEnvArt_ParticleManager::RemoveOldestParticleSystem()
{
    if (ActiveParticleSystems.Num() > 0)
    {
        UNiagaraComponent* OldestSystem = ActiveParticleSystems[0];
        ActiveParticleSystems.RemoveAt(0);
        
        if (IsValid(OldestSystem))
        {
            OldestSystem->DestroyComponent();
        }
    }
}