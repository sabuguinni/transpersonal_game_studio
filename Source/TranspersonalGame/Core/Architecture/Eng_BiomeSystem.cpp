#include "Eng_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEng_BiomeComponent::UEng_BiomeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default biome setup
    BiomeData.BiomeType = EEng_BiomeType::Grassland;
    BiomeData.Temperature = 25.0f;
    BiomeData.Humidity = 0.5f;
    BiomeData.Fertility = 0.7f;
    BiomeData.SpawnDensity = 1.0f;
    
    // Default species for grassland
    BiomeData.DinosaurSpecies.Add(TEXT("Triceratops"));
    BiomeData.DinosaurSpecies.Add(TEXT("Parasaurolophus"));
    BiomeData.DinosaurSpecies.Add(TEXT("Ankylosaurus"));
    
    BiomeData.VegetationTypes.Add(TEXT("Grass"));
    BiomeData.VegetationTypes.Add(TEXT("Ferns"));
    BiomeData.VegetationTypes.Add(TEXT("SmallTrees"));
    
    BiomeRadius = 5000.0f;
}

void UEng_BiomeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with biome manager
    if (UWorld* World = GetWorld())
    {
        if (AEng_BiomeManager* BiomeManager = Cast<AEng_BiomeManager>(UGameplayStatics::GetActorOfClass(World, AEng_BiomeManager::StaticClass())))
        {
            BiomeManager->RegisterBiome(this);
        }
    }
}

void UEng_BiomeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBiomeInfluence(DeltaTime);
}

EEng_BiomeType UEng_BiomeComponent::GetBiomeType() const
{
    return BiomeData.BiomeType;
}

float UEng_BiomeComponent::GetTemperature() const
{
    return BiomeData.Temperature;
}

float UEng_BiomeComponent::GetHumidity() const
{
    return BiomeData.Humidity;
}

bool UEng_BiomeComponent::IsLocationInBiome(const FVector& Location) const
{
    if (!GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
    return Distance <= BiomeRadius;
}

void UEng_BiomeComponent::UpdateBiomeInfluence(float DeltaTime)
{
    // Update biome influence based on time of day, weather, etc.
    // This is where dynamic biome changes would be calculated
    
    // Example: Temperature variation based on time
    if (UWorld* World = GetWorld())
    {
        float TimeOfDay = World->GetTimeSeconds();
        float DayNightCycle = FMath::Sin(TimeOfDay * 0.001f); // Slow cycle
        
        // Slight temperature variation
        float BaseTemp = BiomeData.Temperature;
        float TempVariation = DayNightCycle * 5.0f; // ±5 degrees
        BiomeData.Temperature = BaseTemp + TempVariation;
    }
}

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    BiomeUpdateInterval = 5.0f;
    LastUpdateTime = 0.0f;
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Started managing biomes"));
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= BiomeUpdateInterval)
    {
        UpdateAllBiomes(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

EEng_BiomeType AEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    UEng_BiomeComponent* ClosestBiome = FindClosestBiome(Location);
    if (ClosestBiome && ClosestBiome->IsLocationInBiome(Location))
    {
        return ClosestBiome->GetBiomeType();
    }
    
    return EEng_BiomeType::Grassland; // Default fallback
}

FEng_BiomeData AEng_BiomeManager::GetBiomeDataAtLocation(const FVector& Location) const
{
    UEng_BiomeComponent* ClosestBiome = FindClosestBiome(Location);
    if (ClosestBiome && ClosestBiome->IsLocationInBiome(Location))
    {
        return ClosestBiome->BiomeData;
    }
    
    // Return default biome data
    FEng_BiomeData DefaultData;
    DefaultData.BiomeType = EEng_BiomeType::Grassland;
    DefaultData.Temperature = 25.0f;
    DefaultData.Humidity = 0.5f;
    DefaultData.Fertility = 0.7f;
    DefaultData.SpawnDensity = 1.0f;
    return DefaultData;
}

void AEng_BiomeManager::RegisterBiome(UEng_BiomeComponent* BiomeComponent)
{
    if (BiomeComponent && !ActiveBiomes.Contains(BiomeComponent))
    {
        ActiveBiomes.Add(BiomeComponent);
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Registered biome %s"), 
               *UEnum::GetValueAsString(BiomeComponent->GetBiomeType()));
    }
}

void AEng_BiomeManager::UnregisterBiome(UEng_BiomeComponent* BiomeComponent)
{
    if (BiomeComponent)
    {
        ActiveBiomes.Remove(BiomeComponent);
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Unregistered biome"));
    }
}

TArray<UEng_BiomeComponent*> AEng_BiomeManager::GetBiomesInRange(const FVector& Location, float Range) const
{
    TArray<UEng_BiomeComponent*> BiomesInRange;
    
    for (UEng_BiomeComponent* Biome : ActiveBiomes)
    {
        if (Biome && Biome->GetOwner())
        {
            float Distance = FVector::Dist(Biome->GetOwner()->GetActorLocation(), Location);
            if (Distance <= Range)
            {
                BiomesInRange.Add(Biome);
            }
        }
    }
    
    return BiomesInRange;
}

void AEng_BiomeManager::UpdateAllBiomes(float DeltaTime)
{
    // Clean up invalid biomes
    ActiveBiomes.RemoveAll([](UEng_BiomeComponent* Biome) {
        return !IsValid(Biome) || !IsValid(Biome->GetOwner());
    });
    
    // Update biome interactions and environmental effects
    for (UEng_BiomeComponent* Biome : ActiveBiomes)
    {
        if (Biome)
        {
            Biome->UpdateBiomeInfluence(DeltaTime);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Updated %d active biomes"), ActiveBiomes.Num());
}

UEng_BiomeComponent* AEng_BiomeManager::FindClosestBiome(const FVector& Location) const
{
    UEng_BiomeComponent* ClosestBiome = nullptr;
    float ClosestDistance = FLT_MAX;
    
    for (UEng_BiomeComponent* Biome : ActiveBiomes)
    {
        if (Biome && Biome->GetOwner())
        {
            float Distance = FVector::Dist(Biome->GetOwner()->GetActorLocation(), Location);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestBiome = Biome;
            }
        }
    }
    
    return ClosestBiome;
}