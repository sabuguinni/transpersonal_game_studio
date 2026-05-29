#include "Eng_DinosaurSystemCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EditorLevelLibrary.h"
#include "EditorAssetLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "GameFramework/Actor.h"

UEng_DinosaurSystemCore::UEng_DinosaurSystemCore()
{
    bAutoInitializeOnStartup = true;
    bEnableDebugLogging = true;
}

void UEng_DinosaurSystemCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Initializing dinosaur system"));
    }
    
    if (bAutoInitializeOnStartup)
    {
        InitializeSpeciesDatabase();
    }
}

void UEng_DinosaurSystemCore::Deinitialize()
{
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Shutting down dinosaur system"));
    }
    
    CleanupInvalidDinosaurs();
    SpeciesDatabase.Empty();
    SpawnedDinosaurs.Empty();
    
    Super::Deinitialize();
}

void UEng_DinosaurSystemCore::InitializeSpeciesDatabase()
{
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Setting up species database"));
    }
    
    SpeciesDatabase.Empty();
    SetupDefaultSpecies();
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Species database initialized with %d species"), SpeciesDatabase.Num());
    }
}

void UEng_DinosaurSystemCore::SetupDefaultSpecies()
{
    // T-Rex - Apex predator
    FEng_DinosaurSpecies TRex;
    TRex.SpeciesName = TEXT("Tyrannosaurus Rex");
    TRex.DinosaurType = EEng_DinosaurType::Carnivore;
    TRex.MeshPath = TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin");
    TRex.AnimBlueprintPath = TEXT("/Game/Dinosaur_Pack/Trex/Animation/ABP_Trex");
    TRex.BaseHealth = 800.0f;
    TRex.MovementSpeed = 400.0f;
    TRex.AttackDamage = 150.0f;
    TRex.DetectionRange = 2000.0f;
    TRex.PreferredBiome = EEng_BiomeType::Savanna;
    SpeciesDatabase.Add(TRex);

    // Velociraptor - Pack hunter
    FEng_DinosaurSpecies Raptor;
    Raptor.SpeciesName = TEXT("Velociraptor");
    Raptor.DinosaurType = EEng_DinosaurType::Carnivore;
    Raptor.MeshPath = TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin");
    Raptor.AnimBlueprintPath = TEXT("/Game/Dinosaur_Pack/Velociraptor/Animation/ABP_Velociraptor");
    Raptor.BaseHealth = 200.0f;
    Raptor.MovementSpeed = 600.0f;
    Raptor.AttackDamage = 75.0f;
    Raptor.DetectionRange = 1500.0f;
    Raptor.PreferredBiome = EEng_BiomeType::Forest;
    SpeciesDatabase.Add(Raptor);

    // Triceratops - Defensive herbivore
    FEng_DinosaurSpecies Triceratops;
    Triceratops.SpeciesName = TEXT("Triceratops");
    Triceratops.DinosaurType = EEng_DinosaurType::Herbivore;
    Triceratops.MeshPath = TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops");
    Triceratops.AnimBlueprintPath = TEXT("/Game/Dinosaur_Pack/Triceratops/Animation/ABP_Triceratops");
    Triceratops.BaseHealth = 600.0f;
    Triceratops.MovementSpeed = 250.0f;
    Triceratops.AttackDamage = 100.0f;
    Triceratops.DetectionRange = 800.0f;
    Triceratops.PreferredBiome = EEng_BiomeType::Savanna;
    SpeciesDatabase.Add(Triceratops);

    // Brachiosaurus - Gentle giant
    FEng_DinosaurSpecies Brachio;
    Brachio.SpeciesName = TEXT("Brachiosaurus");
    Brachio.DinosaurType = EEng_DinosaurType::Herbivore;
    Brachio.MeshPath = TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus");
    Brachio.AnimBlueprintPath = TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Animation/ABP_Brachiosaurus");
    Brachio.BaseHealth = 1200.0f;
    Brachio.MovementSpeed = 150.0f;
    Brachio.AttackDamage = 50.0f;
    Brachio.DetectionRange = 1000.0f;
    Brachio.PreferredBiome = EEng_BiomeType::Forest;
    SpeciesDatabase.Add(Brachio);

    // Ankylosaurus - Armored herbivore
    FEng_DinosaurSpecies Ankylo;
    Ankylo.SpeciesName = TEXT("Ankylosaurus");
    Ankylo.DinosaurType = EEng_DinosaurType::Herbivore;
    Ankylo.MeshPath = TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh");
    Ankylo.AnimBlueprintPath = TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Animation/ABP_Ankylosaurus");
    Ankylo.BaseHealth = 500.0f;
    Ankylo.MovementSpeed = 200.0f;
    Ankylo.AttackDamage = 80.0f;
    Ankylo.DetectionRange = 600.0f;
    Ankylo.PreferredBiome = EEng_BiomeType::Desert;
    SpeciesDatabase.Add(Ankylo);
}

FEng_DinosaurSpecies UEng_DinosaurSystemCore::GetSpeciesByType(EEng_DinosaurType DinosaurType)
{
    for (const FEng_DinosaurSpecies& Species : SpeciesDatabase)
    {
        if (Species.DinosaurType == DinosaurType)
        {
            return Species;
        }
    }
    
    // Return default if not found
    return FEng_DinosaurSpecies();
}

TArray<FEng_DinosaurSpecies> UEng_DinosaurSystemCore::GetSpeciesForBiome(EEng_BiomeType BiomeType)
{
    TArray<FEng_DinosaurSpecies> BiomeSpecies;
    
    for (const FEng_DinosaurSpecies& Species : SpeciesDatabase)
    {
        if (Species.PreferredBiome == BiomeType)
        {
            BiomeSpecies.Add(Species);
        }
    }
    
    return BiomeSpecies;
}

bool UEng_DinosaurSystemCore::RegisterSpecies(const FEng_DinosaurSpecies& NewSpecies)
{
    if (!ValidateSpecies(NewSpecies))
    {
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Failed to validate species %s"), *NewSpecies.SpeciesName);
        }
        return false;
    }
    
    SpeciesDatabase.Add(NewSpecies);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Registered new species %s"), *NewSpecies.SpeciesName);
    }
    
    return true;
}

AActor* UEng_DinosaurSystemCore::SpawnDinosaur(const FEng_DinosaurSpawnData& SpawnData)
{
    if (!SpawnData.bIsActive)
    {
        return nullptr;
    }
    
    USkeletalMesh* Mesh = LoadDinosaurMesh(SpawnData.Species.MeshPath);
    if (!Mesh)
    {
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Failed to load mesh for %s"), *SpawnData.Species.SpeciesName);
        }
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn actor with mesh
    AActor* SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnData.SpawnLocation, SpawnData.SpawnRotation);
    if (!SpawnedActor)
    {
        return nullptr;
    }
    
    // Add skeletal mesh component
    USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(SpawnedActor);
    if (MeshComponent)
    {
        MeshComponent->SetSkeletalMesh(Mesh);
        SpawnedActor->SetRootComponent(MeshComponent);
        MeshComponent->RegisterComponent();
    }
    
    // Set actor label
    SpawnedActor->SetActorLabel(SpawnData.ActorLabel);
    
    // Track spawned dinosaur
    SpawnedDinosaurs.Add(SpawnedActor);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Spawned %s at %s"), *SpawnData.Species.SpeciesName, *SpawnData.SpawnLocation.ToString());
    }
    
    return SpawnedActor;
}

TArray<AActor*> UEng_DinosaurSystemCore::SpawnDinosaursInBiome(EEng_BiomeType BiomeType, int32 Count)
{
    TArray<AActor*> SpawnedActors;
    TArray<FEng_DinosaurSpecies> BiomeSpecies = GetSpeciesForBiome(BiomeType);
    
    if (BiomeSpecies.Num() == 0)
    {
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: No species found for biome type"));
        }
        return SpawnedActors;
    }
    
    FVector BiomeLocation = GetBiomeSpawnLocation(BiomeType);
    
    for (int32 i = 0; i < Count; i++)
    {
        // Select random species for this biome
        int32 SpeciesIndex = FMath::RandRange(0, BiomeSpecies.Num() - 1);
        const FEng_DinosaurSpecies& SelectedSpecies = BiomeSpecies[SpeciesIndex];
        
        // Create spawn data
        FEng_DinosaurSpawnData SpawnData;
        SpawnData.Species = SelectedSpecies;
        SpawnData.SpawnLocation = BiomeLocation + FVector(FMath::RandRange(-2000.0f, 2000.0f), FMath::RandRange(-2000.0f, 2000.0f), 0.0f);
        SpawnData.SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        SpawnData.ActorLabel = FString::Printf(TEXT("%s_%d"), *SelectedSpecies.SpeciesName, i);
        SpawnData.bIsActive = true;
        
        AActor* SpawnedActor = SpawnDinosaur(SpawnData);
        if (SpawnedActor)
        {
            SpawnedActors.Add(SpawnedActor);
        }
    }
    
    return SpawnedActors;
}

void UEng_DinosaurSystemCore::SpawnDefaultDinosaurs()
{
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Spawning default dinosaurs"));
    }
    
    // Spawn in Savanna biome
    SpawnDinosaursInBiome(EEng_BiomeType::Savanna, 3);
    
    // Spawn in Forest biome
    SpawnDinosaursInBiome(EEng_BiomeType::Forest, 2);
    
    // Spawn in Desert biome
    SpawnDinosaursInBiome(EEng_BiomeType::Desert, 1);
}

USkeletalMesh* UEng_DinosaurSystemCore::LoadDinosaurMesh(const FString& MeshPath)
{
    if (MeshPath.IsEmpty())
    {
        return nullptr;
    }
    
    USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    if (!LoadedMesh)
    {
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Failed to load mesh at path: %s"), *MeshPath);
        }
    }
    
    return LoadedMesh;
}

UAnimBlueprint* UEng_DinosaurSystemCore::LoadDinosaurAnimBlueprint(const FString& AnimPath)
{
    if (AnimPath.IsEmpty())
    {
        return nullptr;
    }
    
    UAnimBlueprint* LoadedAnim = LoadObject<UAnimBlueprint>(nullptr, *AnimPath);
    if (!LoadedAnim)
    {
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Failed to load animation blueprint at path: %s"), *AnimPath);
        }
    }
    
    return LoadedAnim;
}

void UEng_DinosaurSystemCore::ValidateSpeciesDatabase()
{
    int32 ValidSpecies = 0;
    int32 InvalidSpecies = 0;
    
    for (const FEng_DinosaurSpecies& Species : SpeciesDatabase)
    {
        if (ValidateSpecies(Species))
        {
            ValidSpecies++;
        }
        else
        {
            InvalidSpecies++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Species validation complete - Valid: %d, Invalid: %d"), ValidSpecies, InvalidSpecies);
}

void UEng_DinosaurSystemCore::TestDinosaurSpawning()
{
    if (SpeciesDatabase.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Cannot test spawning - species database is empty"));
        return;
    }
    
    // Test spawn one of each type
    for (const FEng_DinosaurSpecies& Species : SpeciesDatabase)
    {
        FEng_DinosaurSpawnData TestSpawn;
        TestSpawn.Species = Species;
        TestSpawn.SpawnLocation = FVector(FMath::RandRange(-5000.0f, 5000.0f), FMath::RandRange(-5000.0f, 5000.0f), 200.0f);
        TestSpawn.SpawnRotation = FRotator::ZeroRotator;
        TestSpawn.ActorLabel = FString::Printf(TEXT("Test_%s"), *Species.SpeciesName);
        TestSpawn.bIsActive = true;
        
        AActor* TestActor = SpawnDinosaur(TestSpawn);
        if (TestActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Test spawn successful for %s"), *Species.SpeciesName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Eng_DinosaurSystemCore: Test spawn failed for %s"), *Species.SpeciesName);
        }
    }
}

void UEng_DinosaurSystemCore::LogSpeciesDatabase()
{
    UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Species Database (%d entries):"), SpeciesDatabase.Num());
    
    for (int32 i = 0; i < SpeciesDatabase.Num(); i++)
    {
        const FEng_DinosaurSpecies& Species = SpeciesDatabase[i];
        UE_LOG(LogTemp, Warning, TEXT("  [%d] %s - Type: %d, Health: %.1f, Speed: %.1f"), 
               i, *Species.SpeciesName, (int32)Species.DinosaurType, Species.BaseHealth, Species.MovementSpeed);
    }
}

int32 UEng_DinosaurSystemCore::GetActiveSpeciesCount() const
{
    return SpeciesDatabase.Num();
}

FVector UEng_DinosaurSystemCore::GetBiomeSpawnLocation(EEng_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EEng_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 200.0f);
        case EEng_BiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 200.0f);
        case EEng_BiomeType::Desert:
            return FVector(50000.0f, -40000.0f, 200.0f);
        case EEng_BiomeType::Mountain:
            return FVector(0.0f, 50000.0f, 1000.0f);
        case EEng_BiomeType::Swamp:
            return FVector(-30000.0f, -30000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

bool UEng_DinosaurSystemCore::ValidateSpecies(const FEng_DinosaurSpecies& Species)
{
    if (Species.SpeciesName.IsEmpty())
    {
        return false;
    }
    
    if (Species.MeshPath.IsEmpty())
    {
        return false;
    }
    
    if (Species.BaseHealth <= 0.0f || Species.MovementSpeed <= 0.0f)
    {
        return false;
    }
    
    return true;
}

void UEng_DinosaurSystemCore::CleanupInvalidDinosaurs()
{
    SpawnedDinosaurs.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_DinosaurSystemCore: Cleanup complete - %d active dinosaurs"), SpawnedDinosaurs.Num());
    }
}