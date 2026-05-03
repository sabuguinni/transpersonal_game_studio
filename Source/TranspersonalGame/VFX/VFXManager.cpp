#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    EffectUpdateInterval = 0.1f;
    LastUpdateTime = 0.0f;
    CurrentBiome = EBiomeType::Savana;
    
    // Initialize biome effects configurations
    BiomeEffectsConfig.Empty();
    
    // Pantano (Swamp) configuration
    FVFX_BiomeEffects SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Pantano;
    SwampConfig.WindIntensity = 0.3f;
    SwampConfig.DustColor = FLinearColor(0.4f, 0.5f, 0.3f, 0.8f); // Greenish mist
    BiomeEffectsConfig.Add(SwampConfig);
    
    // Floresta (Forest) configuration  
    FVFX_BiomeEffects ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Floresta;
    ForestConfig.WindIntensity = 0.6f;
    ForestConfig.DustColor = FLinearColor(0.6f, 0.4f, 0.2f, 0.6f); // Brown leaves
    BiomeEffectsConfig.Add(ForestConfig);
    
    // Savana configuration
    FVFX_BiomeEffects SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.WindIntensity = 0.8f;
    SavanaConfig.DustColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Tan dust
    BiomeEffectsConfig.Add(SavanaConfig);
    
    // Deserto (Desert) configuration
    FVFX_BiomeEffects DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Deserto;
    DesertConfig.WindIntensity = 1.0f;
    DesertConfig.DustColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f); // Sandy
    BiomeEffectsConfig.Add(DesertConfig);
    
    // Montanha (Mountain) configuration
    FVFX_BiomeEffects MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Montanha;
    MountainConfig.WindIntensity = 1.2f;
    MountainConfig.DustColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.8f); // Snow/ice
    BiomeEffectsConfig.Add(MountainConfig);
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager: BeginPlay started"));
    
    // Initialize effect systems
    InitializeEffectSystems();
    
    // Detect initial biome
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        CurrentBiome = DetectBiomeFromLocation(PlayerLocation);
        UpdateBiomeEffects(CurrentBiome);
        
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Initial biome detected: %d"), (int32)CurrentBiome);
    }
}

void AVFX_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= EffectUpdateInterval)
    {
        UpdateActiveEffects(DeltaTime);
        CleanupExpiredEffects();
        LastUpdateTime = 0.0f;
        
        // Check for biome changes
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            EBiomeType DetectedBiome = DetectBiomeFromLocation(PlayerLocation);
            
            if (DetectedBiome != CurrentBiome)
            {
                CurrentBiome = DetectedBiome;
                UpdateBiomeEffects(CurrentBiome);
                UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Biome changed to: %d"), (int32)CurrentBiome);
            }
        }
    }
}

void AVFX_Manager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVFX_EffectConfig Config)
{
    UNiagaraSystem** FoundSystem = EffectSystems.Find(EffectType);
    if (!FoundSystem || !*FoundSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Effect system not found for type: %d"), (int32)EffectType);
        return;
    }
    
    UNiagaraComponent* NewEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        *FoundSystem,
        Location,
        Rotation
    );
    
    if (NewEffect)
    {
        // Apply configuration
        NewEffect->SetFloatParameter(TEXT("Intensity"), Config.Intensity);
        NewEffect->SetVectorParameter(TEXT("Scale"), Config.Scale);
        NewEffect->SetColorParameter(TEXT("Color"), Config.Color);
        
        ActiveEffects.Add(NewEffect);
        
        UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned effect type %d at location %s"), 
               (int32)EffectType, *Location.ToString());
    }
}

void AVFX_Manager::SpawnDinosaurFootstep(FVector Location, float DinosaurSize)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::DinosaurFootstep;
    Config.Duration = 3.0f;
    Config.Intensity = DinosaurSize;
    Config.Scale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    
    // Get biome-specific dust color
    FVFX_BiomeEffects* BiomeConfig = GetBiomeConfig(CurrentBiome);
    if (BiomeConfig)
    {
        Config.Color = BiomeConfig->DustColor;
    }
    
    SpawnEffect(EVFX_EffectType::DinosaurFootstep, Location, FRotator::ZeroRotator, Config);
    SpawnEffect(EVFX_EffectType::DustCloud, Location, FRotator::ZeroRotator, Config);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned dinosaur footstep at %s, size: %f"), 
           *Location.ToString(), DinosaurSize);
}

void AVFX_Manager::SpawnCampfire(FVector Location)
{
    FVFX_EffectConfig FlameConfig;
    FlameConfig.EffectType = EVFX_EffectType::CampfireFlames;
    FlameConfig.Duration = -1.0f; // Persistent
    FlameConfig.Intensity = 1.0f;
    FlameConfig.Color = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Orange flames
    
    FVFX_EffectConfig SmokeConfig;
    SmokeConfig.EffectType = EVFX_EffectType::Smoke;
    SmokeConfig.Duration = -1.0f; // Persistent
    SmokeConfig.Intensity = 0.7f;
    SmokeConfig.Color = FLinearColor(0.8f, 0.8f, 0.8f, 0.6f); // Gray smoke
    
    SpawnEffect(EVFX_EffectType::CampfireFlames, Location, FRotator::ZeroRotator, FlameConfig);
    SpawnEffect(EVFX_EffectType::Smoke, Location + FVector(0, 0, 50), FRotator::ZeroRotator, SmokeConfig);
    SpawnEffect(EVFX_EffectType::Sparks, Location, FRotator::ZeroRotator, FlameConfig);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned campfire at %s"), *Location.ToString());
}

void AVFX_Manager::SpawnBloodSplatter(FVector Location, FVector Direction)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::BloodSplatter;
    Config.Duration = 5.0f;
    Config.Intensity = 1.0f;
    Config.Color = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red
    
    FRotator DirectionRotation = Direction.Rotation();
    SpawnEffect(EVFX_EffectType::BloodSplatter, Location, DirectionRotation, Config);
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Spawned blood splatter at %s"), *Location.ToString());
}

void AVFX_Manager::UpdateBiomeEffects(EBiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    
    FVFX_BiomeEffects* BiomeConfig = GetBiomeConfig(NewBiome);
    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Manager: No config found for biome: %d"), (int32)NewBiome);
        return;
    }
    
    // Spawn biome-specific ambient effects
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        switch (NewBiome)
        {
            case EBiomeType::Pantano:
                SpawnSwampMist(PlayerLocation);
                break;
            case EBiomeType::Floresta:
                SpawnForestParticles(PlayerLocation);
                break;
            case EBiomeType::Deserto:
                SpawnDesertDust(PlayerLocation);
                break;
            case EBiomeType::Montanha:
                SpawnSnowfall(PlayerLocation);
                break;
            default:
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Updated biome effects for biome: %d"), (int32)NewBiome);
}

void AVFX_Manager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

EBiomeType AVFX_Manager::DetectBiomeFromLocation(FVector Location)
{
    // Based on biome coordinates from brain memory
    float X = Location.X;
    float Y = Location.Y;
    
    // Pantano (sudoeste): X(-77500 a -25000), Y(-76500 a -15000)
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return EBiomeType::Pantano;
    }
    
    // Floresta (noroeste): X(-77500 a -15000), Y(15000 a 76500)
    if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return EBiomeType::Floresta;
    }
    
    // Deserto (leste): X(25000 a 79500), Y(-30000 a 30000)
    if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return EBiomeType::Deserto;
    }
    
    // Montanha (nordeste): X(15000 a 79500), Y(20000 a 76500)
    if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return EBiomeType::Montanha;
    }
    
    // Savana (centro) - default
    return EBiomeType::Savana;
}

void AVFX_Manager::InitializeEffectSystems()
{
    // Note: In a real implementation, these would load actual Niagara systems
    // For now, we initialize the map structure
    EffectSystems.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Effect systems initialized"));
}

void AVFX_Manager::SetGlobalVFXQuality(int32 QualityLevel)
{
    // Adjust effect quality based on performance settings
    float QualityMultiplier = FMath::Clamp(QualityLevel / 3.0f, 0.5f, 2.0f);
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect)
        {
            Effect->SetFloatParameter(TEXT("QualityMultiplier"), QualityMultiplier);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager: Set global VFX quality to level: %d"), QualityLevel);
}

void AVFX_Manager::SpawnSwampMist(FVector Location)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::Smoke;
    Config.Duration = 10.0f;
    Config.Intensity = 0.5f;
    Config.Color = FLinearColor(0.4f, 0.5f, 0.3f, 0.3f); // Greenish mist
    Config.Scale = FVector(3.0f, 3.0f, 1.0f);
    
    SpawnEffect(EVFX_EffectType::Smoke, Location, FRotator::ZeroRotator, Config);
}

void AVFX_Manager::SpawnDesertDust(FVector Location)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::DustCloud;
    Config.Duration = 8.0f;
    Config.Intensity = 1.0f;
    Config.Color = FLinearColor(0.9f, 0.7f, 0.5f, 0.8f); // Sandy
    Config.Scale = FVector(2.0f, 2.0f, 1.0f);
    
    SpawnEffect(EVFX_EffectType::DustCloud, Location, FRotator::ZeroRotator, Config);
    SpawnEffect(EVFX_EffectType::WindParticles, Location, FRotator::ZeroRotator, Config);
}

void AVFX_Manager::SpawnSnowfall(FVector Location)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::WindParticles;
    Config.Duration = 15.0f;
    Config.Intensity = 0.8f;
    Config.Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.9f); // White snow
    Config.Scale = FVector(4.0f, 4.0f, 2.0f);
    
    SpawnEffect(EVFX_EffectType::WindParticles, Location, FRotator::ZeroRotator, Config);
}

void AVFX_Manager::SpawnForestParticles(FVector Location)
{
    FVFX_EffectConfig Config;
    Config.EffectType = EVFX_EffectType::WindParticles;
    Config.Duration = 12.0f;
    Config.Intensity = 0.6f;
    Config.Color = FLinearColor(0.6f, 0.4f, 0.2f, 0.7f); // Brown leaves
    Config.Scale = FVector(2.5f, 2.5f, 1.5f);
    
    SpawnEffect(EVFX_EffectType::WindParticles, Location, FRotator::ZeroRotator, Config);
}

void AVFX_Manager::UpdateActiveEffects(float DeltaTime)
{
    // Update any time-based effect parameters
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && Effect->IsActive())
        {
            // Update wind effects based on current biome
            FVFX_BiomeEffects* BiomeConfig = GetBiomeConfig(CurrentBiome);
            if (BiomeConfig)
            {
                Effect->SetFloatParameter(TEXT("WindIntensity"), BiomeConfig->WindIntensity);
            }
        }
    }
}

void AVFX_Manager::LoadEffectSystems()
{
    // This would load actual Niagara systems from content
    // Implementation depends on available Niagara assets
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager: Loading effect systems..."));
}

FVFX_BiomeEffects* AVFX_Manager::GetBiomeConfig(EBiomeType BiomeType)
{
    for (FVFX_BiomeEffects& Config : BiomeEffectsConfig)
    {
        if (Config.BiomeType == BiomeType)
        {
            return &Config;
        }
    }
    return nullptr;
}