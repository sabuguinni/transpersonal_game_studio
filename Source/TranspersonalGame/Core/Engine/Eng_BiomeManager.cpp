#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    UpdateInterval = 5.0f;
    LastUpdateTime = 0.0f;
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (BiomeConfigs.Num() == 0)
    {
        InitializeBiomeConfigs();
    }
    
    UpdateBiomeStats();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biomes"), BiomeConfigs.Num());
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateBiomeStats();
        ValidateBiomeLimits();
        LastUpdateTime = 0.0f;
    }
}

void AEng_BiomeManager::InitializeBiomeConfigs()
{
    BiomeConfigs.Empty();
    
    // Savanna (center of map)
    FEng_BiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.CenterLocation = FVector(0, 0, 100);
    SavannaConfig.Radius = 30000.0f;
    SavannaConfig.Temperature = 28.0f;
    SavannaConfig.Humidity = 40.0f;
    SavannaConfig.MaxActors = 4000;
    SavannaConfig.MaxDinosaurs = 30;
    SavannaConfig.AllowedDinosaurTypes = {"TRex", "Velociraptor", "Triceratops", "Parasaurolophus"};
    BiomeConfigs.Add(SavannaConfig);
    
    // Swamp
    FEng_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.CenterLocation = FVector(-50000, -45000, 100);
    SwampConfig.Radius = 25000.0f;
    SwampConfig.Temperature = 24.0f;
    SwampConfig.Humidity = 85.0f;
    SwampConfig.MaxActors = 4000;
    SwampConfig.MaxDinosaurs = 25;
    SwampConfig.AllowedDinosaurTypes = {"Parasaurolophus", "Ankylosaurus", "Velociraptor"};
    BiomeConfigs.Add(SwampConfig);
    
    // Forest
    FEng_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.CenterLocation = FVector(-45000, 40000, 100);
    ForestConfig.Radius = 28000.0f;
    ForestConfig.Temperature = 22.0f;
    ForestConfig.Humidity = 70.0f;
    ForestConfig.MaxActors = 4000;
    ForestConfig.MaxDinosaurs = 35;
    ForestConfig.AllowedDinosaurTypes = {"Brachiosaurus", "Triceratops", "Parasaurolophus", "Velociraptor"};
    BiomeConfigs.Add(ForestConfig);
    
    // Desert
    FEng_BiomeConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.CenterLocation = FVector(55000, 0, 100);
    DesertConfig.Radius = 32000.0f;
    DesertConfig.Temperature = 35.0f;
    DesertConfig.Humidity = 15.0f;
    DesertConfig.MaxActors = 4000;
    DesertConfig.MaxDinosaurs = 20;
    DesertConfig.AllowedDinosaurTypes = {"TRex", "Ankylosaurus", "Velociraptor"};
    BiomeConfigs.Add(DesertConfig);
    
    // Mountain
    FEng_BiomeConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.CenterLocation = FVector(40000, 50000, 100);
    MountainConfig.Radius = 26000.0f;
    MountainConfig.Temperature = 18.0f;
    MountainConfig.Humidity = 55.0f;
    MountainConfig.MaxActors = 4000;
    MountainConfig.MaxDinosaurs = 25;
    MountainConfig.AllowedDinosaurTypes = {"TRex", "Triceratops", "Ankylosaurus"};
    BiomeConfigs.Add(MountainConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d default biome configurations"), BiomeConfigs.Num());
}

EBiomeType AEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    for (const FEng_BiomeConfig& Config : BiomeConfigs)
    {
        float Distance = FVector::Dist2D(Location, Config.CenterLocation);
        if (Distance < Config.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Config.BiomeType;
        }
    }
    
    return ClosestBiome;
}

bool AEng_BiomeManager::CanSpawnActorInBiome(EBiomeType BiomeType, bool bIsDinosaur) const
{
    const FEng_BiomeStats* Stats = BiomeStats.Find(BiomeType);
    if (!Stats)
    {
        return false;
    }
    
    const FEng_BiomeConfig* Config = nullptr;
    for (const FEng_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        if (BiomeConfig.BiomeType == BiomeType)
        {
            Config = &BiomeConfig;
            break;
        }
    }
    
    if (!Config)
    {
        return false;
    }
    
    if (bIsDinosaur)
    {
        return Stats->CurrentDinosaurs < Config->MaxDinosaurs;
    }
    else
    {
        return Stats->CurrentActors < Config->MaxActors;
    }
}

FVector AEng_BiomeManager::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    const FEng_BiomeConfig* Config = nullptr;
    for (const FEng_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        if (BiomeConfig.BiomeType == BiomeType)
        {
            Config = &BiomeConfig;
            break;
        }
    }
    
    if (!Config)
    {
        return FVector::ZeroVector;
    }
    
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Config->Radius * 0.8f);
    
    FVector RandomLocation = Config->CenterLocation;
    RandomLocation.X += FMath::Cos(RandomAngle) * RandomDistance;
    RandomLocation.Y += FMath::Sin(RandomAngle) * RandomDistance;
    
    return RandomLocation;
}

void AEng_BiomeManager::UpdateBiomeStats()
{
    BiomeStats.Empty();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (const FEng_BiomeConfig& Config : BiomeConfigs)
    {
        FEng_BiomeStats Stats;
        
        for (AActor* Actor : AllActors)
        {
            if (!Actor || Actor == this)
            {
                continue;
            }
            
            EBiomeType ActorBiome = GetBiomeAtLocation(Actor->GetActorLocation());
            if (ActorBiome == Config.BiomeType)
            {
                Stats.CurrentActors++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains("trex") || ActorName.Contains("veloci") || 
                    ActorName.Contains("tricera") || ActorName.Contains("brachi") ||
                    ActorName.Contains("ankylo") || ActorName.Contains("parasauro"))
                {
                    Stats.CurrentDinosaurs++;
                }
            }
        }
        
        Stats.ActorDensity = (float)Stats.CurrentActors / (PI * Config.Radius * Config.Radius / 1000000.0f);
        Stats.bIsOverloaded = Stats.CurrentActors > Config.MaxActors || Stats.CurrentDinosaurs > Config.MaxDinosaurs;
        
        BiomeStats.Add(Config.BiomeType, Stats);
    }
}

void AEng_BiomeManager::CleanupOverloadedBiomes()
{
    for (const auto& StatsPair : BiomeStats)
    {
        EBiomeType BiomeType = StatsPair.Key;
        const FEng_BiomeStats& Stats = StatsPair.Value;
        
        if (Stats.bIsOverloaded)
        {
            const FEng_BiomeConfig* Config = GetBiomeConfig(BiomeType);
            if (Config)
            {
                int32 ExcessActors = Stats.CurrentActors - Config->MaxActors;
                if (ExcessActors > 0)
                {
                    RemoveOldestActorsFromBiome(BiomeType, ExcessActors);
                }
            }
        }
    }
}

TArray<FString> AEng_BiomeManager::GetAllowedDinosaursForBiome(EBiomeType BiomeType) const
{
    const FEng_BiomeConfig* Config = GetBiomeConfig(BiomeType);
    if (Config)
    {
        return Config->AllowedDinosaurTypes;
    }
    return TArray<FString>();
}

void AEng_BiomeManager::InitializeDefaultBiomes()
{
    InitializeBiomeConfigs();
    UpdateBiomeStats();
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Default biomes initialized via editor"));
}

void AEng_BiomeManager::LogBiomeStatus()
{
    UpdateBiomeStats();
    
    for (const auto& StatsPair : BiomeStats)
    {
        EBiomeType BiomeType = StatsPair.Key;
        const FEng_BiomeStats& Stats = StatsPair.Value;
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors, %d dinosaurs, density %.2f, overloaded: %s"),
            *BiomeName, Stats.CurrentActors, Stats.CurrentDinosaurs, Stats.ActorDensity,
            Stats.bIsOverloaded ? TEXT("YES") : TEXT("NO"));
    }
}

void AEng_BiomeManager::ValidateBiomeLimits()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 20000)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager: CRITICAL - Total actors (%d) exceeds global limit (20000)"), AllActors.Num());
        CleanupOverloadedBiomes();
    }
}

FEng_BiomeConfig* AEng_BiomeManager::GetBiomeConfig(EBiomeType BiomeType)
{
    for (FEng_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return &Config;
        }
    }
    return nullptr;
}

void AEng_BiomeManager::RemoveOldestActorsFromBiome(EBiomeType BiomeType, int32 NumToRemove)
{
    TArray<AActor*> BiomeActors;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != this && GetBiomeAtLocation(Actor->GetActorLocation()) == BiomeType)
        {
            FString ActorName = Actor->GetName().ToLower();
            // Don't remove important actors
            if (!ActorName.Contains("playerstart") && !ActorName.Contains("gamemode") && !ActorName.Contains("biome"))
            {
                BiomeActors.Add(Actor);
            }
        }
    }
    
    // Sort by creation time (oldest first) - using actor ID as proxy
    BiomeActors.Sort([](const AActor& A, const AActor& B) {
        return A.GetUniqueID() < B.GetUniqueID();
    });
    
    int32 RemovedCount = 0;
    for (int32 i = 0; i < BiomeActors.Num() && RemovedCount < NumToRemove; i++)
    {
        if (BiomeActors[i])
        {
            BiomeActors[i]->Destroy();
            RemovedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Removed %d excess actors from biome"), RemovedCount);
}