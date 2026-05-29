#include "Build_FinalIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UBuild_FinalIntegrationManager::UBuild_FinalIntegrationManager()
{
    bIntegrationComplete = false;
    LastFullValidationTime = 0.0f;
    TotalActorsInWorld = 0;
    LoadedClassCount = 0;
}

void UBuild_FinalIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Initializing integration subsystem"));
    
    // Initialize biome registry with predefined locations
    InitializeBiomeRegistry();
    
    // Register core systems
    RegisterSystemStatus(TEXT("GameMode"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("PlayerCharacter"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("Lighting"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("Navigation"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("DinosaurAssets"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("WorldGeneration"), false, TEXT("Not validated"));
    RegisterSystemStatus(TEXT("BiomePopulation"), false, TEXT("Not validated"));
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Initialization complete"));
}

void UBuild_FinalIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Deinitializing integration subsystem"));
    
    BiomeRegistry.Empty();
    SystemRegistry.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationManager::InitializeBiomeRegistry()
{
    // Initialize biome data with correct coordinates from memory
    FBuild_BiomeData SavanaData;
    SavanaData.BiomeType = EBuild_BiomeType::Savana;
    SavanaData.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaData.Radius = 25000.0f;
    BiomeRegistry.Add(EBuild_BiomeType::Savana, SavanaData);
    
    FBuild_BiomeData PantanoData;
    PantanoData.BiomeType = EBuild_BiomeType::Pantano;
    PantanoData.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoData.Radius = 25000.0f;
    BiomeRegistry.Add(EBuild_BiomeType::Pantano, PantanoData);
    
    FBuild_BiomeData FlorestaData;
    FlorestaData.BiomeType = EBuild_BiomeType::Floresta;
    FlorestaData.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaData.Radius = 25000.0f;
    BiomeRegistry.Add(EBuild_BiomeType::Floresta, FlorestaData);
    
    FBuild_BiomeData DesertoData;
    DesertoData.BiomeType = EBuild_BiomeType::Deserto;
    DesertoData.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoData.Radius = 25000.0f;
    BiomeRegistry.Add(EBuild_BiomeType::Deserto, DesertoData);
    
    FBuild_BiomeData MontanhaData;
    MontanhaData.BiomeType = EBuild_BiomeType::Montanha;
    MontanhaData.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaData.Radius = 25000.0f;
    BiomeRegistry.Add(EBuild_BiomeType::Montanha, MontanhaData);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Biome registry initialized with 5 biomes"));
}

void UBuild_FinalIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Starting comprehensive system validation"));
    
    LastFullValidationTime = GetWorld()->GetTimeSeconds();
    
    // Validate all core systems
    ValidateWorldActors();
    ValidateGameMode();
    ValidatePlayerCharacter();
    ValidateLighting();
    ValidateNavigation();
    ValidateDinosaurAssets();
    ValidateEnvironmentAssets();
    ValidateBiomePopulation();
    ValidateGameplaySystems();
    ValidateAssetIntegrity();
    
    // Check if all systems are operational
    bool bAllSystemsOperational = true;
    for (const auto& SystemPair : SystemRegistry)
    {
        if (!SystemPair.Value.bIsOperational)
        {
            bAllSystemsOperational = false;
            UE_LOG(LogTemp, Error, TEXT("System not operational: %s - %s"), 
                *SystemPair.Key, *SystemPair.Value.LastError);
        }
    }
    
    bIntegrationComplete = bAllSystemsOperational;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: System validation complete - Integration: %s"), 
        bIntegrationComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
}

void UBuild_FinalIntegrationManager::ValidateWorldActors()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        TotalActorsInWorld = AllActors.Num();
        
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Found %d actors in world"), TotalActorsInWorld);
        
        if (TotalActorsInWorld > 0)
        {
            RegisterSystemStatus(TEXT("WorldActors"), true, TEXT(""));
        }
        else
        {
            RegisterSystemStatus(TEXT("WorldActors"), false, TEXT("No actors found in world"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidateGameMode()
{
    if (UWorld* World = GetWorld())
    {
        if (AGameModeBase* GameMode = World->GetAuthGameMode())
        {
            FString GameModeName = GameMode->GetClass()->GetName();
            RegisterSystemStatus(TEXT("GameMode"), true, TEXT(""));
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: GameMode validated: %s"), *GameModeName);
        }
        else
        {
            RegisterSystemStatus(TEXT("GameMode"), false, TEXT("No GameMode found"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidatePlayerCharacter()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
        
        if (Characters.Num() > 0)
        {
            RegisterSystemStatus(TEXT("PlayerCharacter"), true, TEXT(""));
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Found %d character(s)"), Characters.Num());
        }
        else
        {
            RegisterSystemStatus(TEXT("PlayerCharacter"), false, TEXT("No character actors found"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidateLighting()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        
        if (DirectionalLights.Num() > 0 || SkyLights.Num() > 0)
        {
            RegisterSystemStatus(TEXT("Lighting"), true, TEXT(""));
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Lighting validated - Dir: %d, Sky: %d"), 
                DirectionalLights.Num(), SkyLights.Num());
        }
        else
        {
            RegisterSystemStatus(TEXT("Lighting"), false, TEXT("No lighting actors found"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidateNavigation()
{
    if (UWorld* World = GetWorld())
    {
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
        {
            RegisterSystemStatus(TEXT("Navigation"), true, TEXT(""));
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Navigation system validated"));
        }
        else
        {
            RegisterSystemStatus(TEXT("Navigation"), false, TEXT("Navigation system not found"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidateDinosaurAssets()
{
    // Check for dinosaur assets in the asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    TArray<FAssetData> DinosaurAssets;
    FARFilter Filter;
    Filter.PackagePaths.Add("/Game/Dinosaur_Pack");
    Filter.bRecursivePaths = true;
    
    AssetRegistry.GetAssets(Filter, DinosaurAssets);
    
    if (DinosaurAssets.Num() > 0)
    {
        RegisterSystemStatus(TEXT("DinosaurAssets"), true, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Found %d dinosaur assets"), DinosaurAssets.Num());
    }
    else
    {
        RegisterSystemStatus(TEXT("DinosaurAssets"), false, TEXT("No dinosaur assets found in /Game/Dinosaur_Pack"));
    }
}

void UBuild_FinalIntegrationManager::ValidateEnvironmentAssets()
{
    // Check for environment assets
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    TArray<FAssetData> EnvironmentAssets;
    FARFilter Filter;
    Filter.PackagePaths.Add("/Game/LandscapePackOne");
    Filter.PackagePaths.Add("/Game/LandscapePackTwo");
    Filter.PackagePaths.Add("/Game/Tropical_Jungle_Pack");
    Filter.bRecursivePaths = true;
    
    AssetRegistry.GetAssets(Filter, EnvironmentAssets);
    
    if (EnvironmentAssets.Num() > 0)
    {
        RegisterSystemStatus(TEXT("EnvironmentAssets"), true, TEXT(""));
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Found %d environment assets"), EnvironmentAssets.Num());
    }
    else
    {
        RegisterSystemStatus(TEXT("EnvironmentAssets"), false, TEXT("No environment assets found"));
    }
}

void UBuild_FinalIntegrationManager::ValidateBiomePopulation()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        // Reset biome actor counts
        for (auto& BiomePair : BiomeRegistry)
        {
            BiomePair.Value.ActorCount = 0;
            BiomePair.Value.SpawnedActors.Empty();
        }
        
        // Count actors in each biome
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FVector ActorLocation = Actor->GetActorLocation();
                
                for (auto& BiomePair : BiomeRegistry)
                {
                    FVector BiomeCenter = BiomePair.Value.CenterLocation;
                    float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
                    
                    if (Distance <= BiomePair.Value.Radius)
                    {
                        BiomePair.Value.ActorCount++;
                        BiomePair.Value.SpawnedActors.Add(Actor);
                        break; // Actor belongs to this biome
                    }
                }
            }
        }
        
        // Check if biomes meet minimum population requirements
        bool bBiomesPopulated = true;
        int32 MinActorsPerBiome = 10; // Minimum threshold
        
        for (const auto& BiomePair : BiomeRegistry)
        {
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Biome %d has %d actors"), 
                (int32)BiomePair.Key, BiomePair.Value.ActorCount);
                
            if (BiomePair.Value.ActorCount < MinActorsPerBiome)
            {
                bBiomesPopulated = false;
            }
        }
        
        if (bBiomesPopulated)
        {
            RegisterSystemStatus(TEXT("BiomePopulation"), true, TEXT(""));
        }
        else
        {
            RegisterSystemStatus(TEXT("BiomePopulation"), false, TEXT("Some biomes below minimum population"));
        }
    }
}

void UBuild_FinalIntegrationManager::ValidateGameplaySystems()
{
    // Validate that core gameplay systems are functional
    bool bGameplayValid = true;
    FString ErrorMessage;
    
    if (UWorld* World = GetWorld())
    {
        // Check for PlayerStart
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
        
        if (PlayerStarts.Num() == 0)
        {
            bGameplayValid = false;
            ErrorMessage += TEXT("No PlayerStart found; ");
        }
        
        // Check for basic world geometry
        TArray<AActor*> StaticMeshActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
        
        if (StaticMeshActors.Num() < 5)
        {
            bGameplayValid = false;
            ErrorMessage += TEXT("Insufficient world geometry; ");
        }
    }
    
    RegisterSystemStatus(TEXT("GameplaySystems"), bGameplayValid, ErrorMessage);
}

void UBuild_FinalIntegrationManager::ValidateAssetIntegrity()
{
    // Validate that critical assets are available and loadable
    bool bAssetsValid = true;
    FString ErrorMessage;
    
    // This is a placeholder for more comprehensive asset validation
    // In a full implementation, this would check asset loading, dependencies, etc.
    
    RegisterSystemStatus(TEXT("AssetIntegrity"), bAssetsValid, ErrorMessage);
}

void UBuild_FinalIntegrationManager::RegisterBiomeActor(EBuild_BiomeType BiomeType, AActor* Actor)
{
    if (Actor && BiomeRegistry.Contains(BiomeType))
    {
        FBuild_BiomeData& BiomeData = BiomeRegistry[BiomeType];
        BiomeData.SpawnedActors.AddUnique(Actor);
        BiomeData.ActorCount = BiomeData.SpawnedActors.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Build_FinalIntegrationManager: Registered actor %s to biome %d"), 
            *Actor->GetName(), (int32)BiomeType);
    }
}

int32 UBuild_FinalIntegrationManager::GetBiomeActorCount(EBuild_BiomeType BiomeType) const
{
    if (const FBuild_BiomeData* BiomeData = BiomeRegistry.Find(BiomeType))
    {
        return BiomeData->ActorCount;
    }
    return 0;
}

FVector UBuild_FinalIntegrationManager::GetBiomeCenterLocation(EBuild_BiomeType BiomeType) const
{
    if (const FBuild_BiomeData* BiomeData = BiomeRegistry.Find(BiomeType))
    {
        return BiomeData->CenterLocation;
    }
    return FVector::ZeroVector;
}

void UBuild_FinalIntegrationManager::RegisterSystemStatus(const FString& SystemName, bool bOperational, const FString& ErrorMessage)
{
    FBuild_SystemStatus& Status = SystemRegistry.FindOrAdd(SystemName);
    Status.SystemName = SystemName;
    Status.bIsOperational = bOperational;
    Status.LastError = ErrorMessage;
    Status.LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

bool UBuild_FinalIntegrationManager::IsSystemOperational(const FString& SystemName) const
{
    if (const FBuild_SystemStatus* Status = SystemRegistry.Find(SystemName))
    {
        return Status->bIsOperational;
    }
    return false;
}

TArray<FBuild_SystemStatus> UBuild_FinalIntegrationManager::GetSystemStatusReport() const
{
    TArray<FBuild_SystemStatus> Report;
    for (const auto& SystemPair : SystemRegistry)
    {
        Report.Add(SystemPair.Value);
    }
    return Report;
}

TArray<FBuild_BiomeData> UBuild_FinalIntegrationManager::GetBiomeStatusReport() const
{
    TArray<FBuild_BiomeData> Report;
    for (const auto& BiomePair : BiomeRegistry)
    {
        Report.Add(BiomePair.Value);
    }
    return Report;
}

void UBuild_FinalIntegrationManager::RunComprehensiveDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Running comprehensive diagnostics"));
    
    ValidateAllSystems();
    LogIntegrationReport();
    ValidateMinimumPlayableRequirements();
}

void UBuild_FinalIntegrationManager::LogIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Integration Complete: %s"), bIntegrationComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %.2f"), LastFullValidationTime);
    
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS ==="));
    for (const auto& SystemPair : SystemRegistry)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), 
            *SystemPair.Key, 
            SystemPair.Value.bIsOperational ? TEXT("OK") : TEXT("FAIL"),
            *SystemPair.Value.LastError);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME STATUS ==="));
    for (const auto& BiomePair : BiomeRegistry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome %d: %d actors at (%.0f, %.0f)"), 
            (int32)BiomePair.Key,
            BiomePair.Value.ActorCount,
            BiomePair.Value.CenterLocation.X,
            BiomePair.Value.CenterLocation.Y);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UBuild_FinalIntegrationManager::ValidateMinimumPlayableRequirements()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: Validating minimum playable requirements"));
    
    bool bPlayable = true;
    TArray<FString> MissingRequirements;
    
    // Check for essential systems
    if (!IsSystemOperational(TEXT("GameMode")))
    {
        bPlayable = false;
        MissingRequirements.Add(TEXT("GameMode"));
    }
    
    if (!IsSystemOperational(TEXT("PlayerCharacter")))
    {
        bPlayable = false;
        MissingRequirements.Add(TEXT("PlayerCharacter"));
    }
    
    if (!IsSystemOperational(TEXT("Lighting")))
    {
        bPlayable = false;
        MissingRequirements.Add(TEXT("Lighting"));
    }
    
    if (TotalActorsInWorld < 10)
    {
        bPlayable = false;
        MissingRequirements.Add(TEXT("Insufficient world content"));
    }
    
    if (bPlayable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationManager: MINIMUM PLAYABLE REQUIREMENTS MET"));
        RegisterSystemStatus(TEXT("MinimumPlayable"), true, TEXT(""));
    }
    else
    {
        FString ErrorMsg = FString::Join(MissingRequirements, TEXT(", "));
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationManager: MINIMUM PLAYABLE REQUIREMENTS NOT MET: %s"), *ErrorMsg);
        RegisterSystemStatus(TEXT("MinimumPlayable"), false, ErrorMsg);
    }
}