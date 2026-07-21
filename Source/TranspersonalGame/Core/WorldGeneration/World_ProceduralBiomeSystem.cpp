#include "World_ProceduralBiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"

AWorld_ProceduralBiomeSystem::AWorld_ProceduralBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    MaxSpawnDistance = 100000.0f;
    MaxActorsPerBiome = 500;
    
    // Initialize default biome definitions
    InitializeBiomes();
}

void AWorld_ProceduralBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-generate biomes on play if they don't exist
    if (SpawnedActors.Num() == 0)
    {
        GenerateAllBiomes();
    }
}

void AWorld_ProceduralBiomeSystem::InitializeBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana Biome
    FWorld_BiomeDefinition Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 25000.0f;
    Savana.BaseHeight = 100.0f;
    Savana.Temperature = 30.0f;
    Savana.Humidity = 0.3f;
    Savana.NativeDinosaurs.Add(TEXT("TRex"));
    Savana.NativeDinosaurs.Add(TEXT("Triceratops"));
    Savana.NativeDinosaurs.Add(TEXT("Velociraptor"));
    BiomeDefinitions.Add(Savana);
    
    // Floresta Biome
    FWorld_BiomeDefinition Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 150.0f);
    Floresta.Radius = 20000.0f;
    Floresta.BaseHeight = 150.0f;
    Floresta.Temperature = 25.0f;
    Floresta.Humidity = 0.8f;
    Floresta.NativeDinosaurs.Add(TEXT("Brachiosaurus"));
    Floresta.NativeDinosaurs.Add(TEXT("Velociraptor"));
    BiomeDefinitions.Add(Floresta);
    
    // Deserto Biome
    FWorld_BiomeDefinition Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 80.0f);
    Deserto.Radius = 18000.0f;
    Deserto.BaseHeight = 80.0f;
    Deserto.Temperature = 40.0f;
    Deserto.Humidity = 0.1f;
    Deserto.NativeDinosaurs.Add(TEXT("Ankylosaurus"));
    Deserto.NativeDinosaurs.Add(TEXT("Triceratops"));
    BiomeDefinitions.Add(Deserto);
    
    // Pantano Biome
    FWorld_BiomeDefinition Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 50.0f);
    Pantano.Radius = 15000.0f;
    Pantano.BaseHeight = 50.0f;
    Pantano.Temperature = 28.0f;
    Pantano.Humidity = 0.9f;
    Pantano.NativeDinosaurs.Add(TEXT("Brachiosaurus"));
    Pantano.NativeDinosaurs.Add(TEXT("Velociraptor"));
    BiomeDefinitions.Add(Pantano);
    
    // Montanha Biome
    FWorld_BiomeDefinition Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 300.0f);
    Montanha.Radius = 22000.0f;
    Montanha.BaseHeight = 300.0f;
    Montanha.Temperature = 15.0f;
    Montanha.Humidity = 0.6f;
    Montanha.NativeDinosaurs.Add(TEXT("Ankylosaurus"));
    Montanha.NativeDinosaurs.Add(TEXT("TRex"));
    BiomeDefinitions.Add(Montanha);
}

void AWorld_ProceduralBiomeSystem::GenerateAllBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Generating all biomes"));
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        GenerateBiome(Biome.BiomeName);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Generated %d total actors across all biomes"), SpawnedActors.Num());
}

void AWorld_ProceduralBiomeSystem::GenerateBiome(const FString& BiomeName)
{
    const FWorld_BiomeDefinition* BiomePtr = BiomeDefinitions.FindByPredicate([&BiomeName](const FWorld_BiomeDefinition& Biome)
    {
        return Biome.BiomeName == BiomeName;
    });
    
    if (!BiomePtr)
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralBiomeSystem: Biome '%s' not found"), *BiomeName);
        return;
    }
    
    const FWorld_BiomeDefinition& Biome = *BiomePtr;
    
    // Clear existing actors in this biome first
    ClearBiome(BiomeName);
    
    // Generate different types of content
    SpawnRockFormations(Biome);
    SpawnVegetation(Biome);
    SpawnDinosaurs(Biome);
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Generated biome '%s' with %d actors"), 
           *BiomeName, GetActorCountInBiome(BiomeName));
}

void AWorld_ProceduralBiomeSystem::ClearBiome(const FString& BiomeName)
{
    CleanupActorsInBiome(BiomeName);
}

void AWorld_ProceduralBiomeSystem::ClearAllBiomes()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Cleared all biomes"));
}

FWorld_BiomeDefinition AWorld_ProceduralBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Distance <= Biome.Radius)
        {
            return Biome;
        }
    }
    
    // Return default biome if not found
    FWorld_BiomeDefinition DefaultBiome;
    DefaultBiome.BiomeName = TEXT("Unknown");
    return DefaultBiome;
}

TArray<FString> AWorld_ProceduralBiomeSystem::GetAllBiomeNames() const
{
    TArray<FString> Names;
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        Names.Add(Biome.BiomeName);
    }
    return Names;
}

int32 AWorld_ProceduralBiomeSystem::GetActorCountInBiome(const FString& BiomeName) const
{
    const FWorld_BiomeDefinition* BiomePtr = BiomeDefinitions.FindByPredicate([&BiomeName](const FWorld_BiomeDefinition& Biome)
    {
        return Biome.BiomeName == BiomeName;
    });
    
    if (!BiomePtr)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor) && IsActorInBiome(Actor, *BiomePtr))
        {
            Count++;
        }
    }
    
    return Count;
}

void AWorld_ProceduralBiomeSystem::OptimizeBiomePerformance(const FString& BiomeName, float PlayerDistance)
{
    const FWorld_BiomeDefinition* BiomePtr = BiomeDefinitions.FindByPredicate([&BiomeName](const FWorld_BiomeDefinition& Biome)
    {
        return Biome.BiomeName == BiomeName;
    });
    
    if (!BiomePtr)
    {
        return;
    }
    
    // Apply LOD based on distance
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor) && IsActorInBiome(Actor, *BiomePtr))
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    if (PlayerDistance > 50000.0f)
                    {
                        // Very far - disable collision and shadows
                        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                        MeshComp->SetCastShadow(false);
                    }
                    else if (PlayerDistance > 25000.0f)
                    {
                        // Far - disable shadows only
                        MeshComp->SetCastShadow(false);
                    }
                    else
                    {
                        // Close - full quality
                        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                        MeshComp->SetCastShadow(true);
                    }
                }
            }
        }
    }
}

void AWorld_ProceduralBiomeSystem::EditorGenerateAllBiomes()
{
    GenerateAllBiomes();
}

void AWorld_ProceduralBiomeSystem::EditorClearAllBiomes()
{
    ClearAllBiomes();
}

void AWorld_ProceduralBiomeSystem::SpawnRockFormations(const FWorld_BiomeDefinition& Biome)
{
    int32 RockCount = FMath::RandRange(RockSpawnParams.MinCount, RockSpawnParams.MaxCount);
    
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(Biome, RockSpawnParams.SpawnRadius);
        FRotator SpawnRotation = RockSpawnParams.bRandomRotation ? GetRandomRotation() : FRotator::ZeroRotator;
        
        // Spawn basic cube as rock placeholder
        if (UWorld* World = GetWorld())
        {
            AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, SpawnRotation);
            if (RockActor)
            {
                float Scale = GetRandomScale(RockSpawnParams.MinScale, RockSpawnParams.MaxScale);
                RockActor->SetActorScale3D(FVector(Scale));
                RockActor->SetActorLabel(FString::Printf(TEXT("Rock_%s_%03d"), *Biome.BiomeName, i + 1));
                
                SpawnedActors.Add(RockActor);
            }
        }
    }
}

void AWorld_ProceduralBiomeSystem::SpawnVegetation(const FWorld_BiomeDefinition& Biome)
{
    int32 VegCount = FMath::RandRange(VegetationSpawnParams.MinCount, VegetationSpawnParams.MaxCount);
    
    for (int32 i = 0; i < VegCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(Biome, VegetationSpawnParams.SpawnRadius);
        FRotator SpawnRotation = VegetationSpawnParams.bRandomRotation ? GetRandomRotation() : FRotator::ZeroRotator;
        
        // Spawn basic cylinder as vegetation placeholder
        if (UWorld* World = GetWorld())
        {
            AStaticMeshActor* VegActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, SpawnRotation);
            if (VegActor)
            {
                float Scale = GetRandomScale(VegetationSpawnParams.MinScale, VegetationSpawnParams.MaxScale);
                VegActor->SetActorScale3D(FVector(Scale * 0.5f, Scale * 0.5f, Scale * 2.0f)); // Tall and thin
                VegActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%03d"), *Biome.BiomeName, i + 1));
                
                SpawnedActors.Add(VegActor);
            }
        }
    }
}

void AWorld_ProceduralBiomeSystem::SpawnDinosaurs(const FWorld_BiomeDefinition& Biome)
{
    int32 DinoCount = FMath::RandRange(DinosaurSpawnParams.MinCount, DinosaurSpawnParams.MaxCount);
    DinoCount = FMath::Min(DinoCount, Biome.NativeDinosaurs.Num() * 2); // Limit based on available species
    
    for (int32 i = 0; i < DinoCount && i < Biome.NativeDinosaurs.Num(); i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(Biome, DinosaurSpawnParams.SpawnRadius);
        FRotator SpawnRotation = DinosaurSpawnParams.bRandomRotation ? GetRandomRotation() : FRotator::ZeroRotator;
        
        // Spawn basic sphere as dinosaur placeholder (real dinosaurs spawned via Python)
        if (UWorld* World = GetWorld())
        {
            AStaticMeshActor* DinoActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, SpawnRotation);
            if (DinoActor)
            {
                float Scale = GetRandomScale(DinosaurSpawnParams.MinScale, DinosaurSpawnParams.MaxScale);
                DinoActor->SetActorScale3D(FVector(Scale * 2.0f, Scale * 1.0f, Scale * 1.5f)); // Dinosaur-like proportions
                DinoActor->SetActorLabel(FString::Printf(TEXT("Dino_%s_%s_%03d"), 
                                        *Biome.NativeDinosaurs[i % Biome.NativeDinosaurs.Num()], 
                                        *Biome.BiomeName, i + 1));
                
                SpawnedActors.Add(DinoActor);
            }
        }
    }
}

FVector AWorld_ProceduralBiomeSystem::GetRandomLocationInBiome(const FWorld_BiomeDefinition& Biome, float RadiusMultiplier) const
{
    float Angle = UKismetMathLibrary::RandomFloatInRange(0.0f, 2.0f * PI);
    float Distance = UKismetMathLibrary::RandomFloatInRange(Biome.Radius * 0.3f, Biome.Radius * RadiusMultiplier);
    
    float X = Biome.CenterLocation.X + Distance * FMath::Cos(Angle);
    float Y = Biome.CenterLocation.Y + Distance * FMath::Sin(Angle);
    float Z = Biome.BaseHeight + UKismetMathLibrary::RandomFloatInRange(-20.0f, 50.0f);
    
    return FVector(X, Y, Z);
}

FRotator AWorld_ProceduralBiomeSystem::GetRandomRotation() const
{
    return FRotator(0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f), 0.0f);
}

float AWorld_ProceduralBiomeSystem::GetRandomScale(float MinScale, float MaxScale) const
{
    return UKismetMathLibrary::RandomFloatInRange(MinScale, MaxScale);
}

void AWorld_ProceduralBiomeSystem::CleanupActorsInBiome(const FString& BiomeName)
{
    const FWorld_BiomeDefinition* BiomePtr = BiomeDefinitions.FindByPredicate([&BiomeName](const FWorld_BiomeDefinition& Biome)
    {
        return Biome.BiomeName == BiomeName;
    });
    
    if (!BiomePtr)
    {
        return;
    }
    
    TArray<AActor*> ActorsToRemove;
    for (AActor* Actor : SpawnedActors)
    {
        if (IsValid(Actor) && IsActorInBiome(Actor, *BiomePtr))
        {
            Actor->Destroy();
            ActorsToRemove.Add(Actor);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        SpawnedActors.Remove(Actor);
    }
}

bool AWorld_ProceduralBiomeSystem::IsActorInBiome(AActor* Actor, const FWorld_BiomeDefinition& Biome) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    float Distance = FVector::Dist2D(ActorLocation, Biome.CenterLocation);
    return Distance <= Biome.Radius;
}