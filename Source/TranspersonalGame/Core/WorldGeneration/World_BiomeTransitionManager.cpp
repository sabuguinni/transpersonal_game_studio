#include "World_BiomeTransitionManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

AWorld_BiomeTransitionManager::AWorld_BiomeTransitionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize biome transition settings
    TransitionDistance = 5000.0f;
    BlendRadius = 2000.0f;
    bEnableRealTimeTransitions = true;
    TransitionUpdateInterval = 1.0f;
    
    // Initialize biome zone definitions
    InitializeBiomeZones();
}

void AWorld_BiomeTransitionManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableRealTimeTransitions)
    {
        GetWorldTimerManager().SetTimer(
            TransitionUpdateTimerHandle,
            this,
            &AWorld_BiomeTransitionManager::UpdateBiomeTransitions,
            TransitionUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransitionManager: Started with %d biome zones"), BiomeZones.Num());
}

void AWorld_BiomeTransitionManager::InitializeBiomeZones()
{
    BiomeZones.Empty();
    
    // Define the 5 main biomes with their geographical boundaries
    FWorld_BiomeZone PantanoZone;
    PantanoZone.BiomeType = EWorld_BiomeType::Pantano;
    PantanoZone.CenterLocation = FVector(-50000, -45000, 0);
    PantanoZone.BoundaryMin = FVector(-77500, -76500, -100);
    PantanoZone.BoundaryMax = FVector(-25000, -15000, 200);
    PantanoZone.bIsActive = true;
    BiomeZones.Add(PantanoZone);
    
    FWorld_BiomeZone FlorestaZone;
    FlorestaZone.BiomeType = EWorld_BiomeType::Floresta;
    FlorestaZone.CenterLocation = FVector(-45000, 40000, 0);
    FlorestaZone.BoundaryMin = FVector(-77500, 15000, -50);
    FlorestaZone.BoundaryMax = FVector(-15000, 76500, 500);
    FlorestaZone.bIsActive = true;
    BiomeZones.Add(FlorestaZone);
    
    FWorld_BiomeZone SavanaZone;
    SavanaZone.BiomeType = EWorld_BiomeType::Savana;
    SavanaZone.CenterLocation = FVector(0, 0, 0);
    SavanaZone.BoundaryMin = FVector(-20000, -20000, -50);
    SavanaZone.BoundaryMax = FVector(20000, 20000, 300);
    SavanaZone.bIsActive = true;
    BiomeZones.Add(SavanaZone);
    
    FWorld_BiomeZone DesertoZone;
    DesertoZone.BiomeType = EWorld_BiomeType::Deserto;
    DesertoZone.CenterLocation = FVector(55000, 0, 0);
    DesertoZone.BoundaryMin = FVector(25000, -30000, -20);
    DesertoZone.BoundaryMax = FVector(79500, 30000, 400);
    DesertoZone.bIsActive = true;
    BiomeZones.Add(DesertoZone);
    
    FWorld_BiomeZone MontanhaZone;
    MontanhaZone.BiomeType = EWorld_BiomeType::MontanhaNevada;
    MontanhaZone.CenterLocation = FVector(40000, 50000, 500);
    MontanhaZone.BoundaryMin = FVector(15000, 20000, 200);
    MontanhaZone.BoundaryMax = FVector(79500, 76500, 2000);
    MontanhaZone.bIsActive = true;
    BiomeZones.Add(MontanhaZone);
}

EWorld_BiomeType AWorld_BiomeTransitionManager::GetBiomeAtLocation(const FVector& Location) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.bIsActive && IsLocationInBiome(Location, Zone))
        {
            return Zone.BiomeType;
        }
    }
    
    // Default to Savana if no specific biome found
    return EWorld_BiomeType::Savana;
}

bool AWorld_BiomeTransitionManager::IsLocationInBiome(const FVector& Location, const FWorld_BiomeZone& BiomeZone) const
{
    return Location.X >= BiomeZone.BoundaryMin.X && Location.X <= BiomeZone.BoundaryMax.X &&
           Location.Y >= BiomeZone.BoundaryMin.Y && Location.Y <= BiomeZone.BoundaryMax.Y &&
           Location.Z >= BiomeZone.BoundaryMin.Z && Location.Z <= BiomeZone.BoundaryMax.Z;
}

float AWorld_BiomeTransitionManager::GetTransitionWeight(const FVector& Location, EWorld_BiomeType TargetBiome) const
{
    const FWorld_BiomeZone* TargetZone = nullptr;
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == TargetBiome)
        {
            TargetZone = &Zone;
            break;
        }
    }
    
    if (!TargetZone)
    {
        return 0.0f;
    }
    
    // Calculate distance to biome center
    float DistanceToCenter = FVector::Dist(Location, TargetZone->CenterLocation);
    
    // Calculate transition weight based on distance
    if (DistanceToCenter <= BlendRadius)
    {
        return 1.0f; // Full biome influence
    }
    else if (DistanceToCenter <= TransitionDistance)
    {
        // Linear falloff in transition zone
        float TransitionFactor = (TransitionDistance - DistanceToCenter) / (TransitionDistance - BlendRadius);
        return FMath::Clamp(TransitionFactor, 0.0f, 1.0f);
    }
    
    return 0.0f; // No influence
}

void AWorld_BiomeTransitionManager::UpdateBiomeTransitions()
{
    // Get player location for transition calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EWorld_BiomeType CurrentBiome = GetBiomeAtLocation(PlayerLocation);
    
    // Update current biome if changed
    if (CurrentBiome != CurrentPlayerBiome)
    {
        OnBiomeTransition(CurrentPlayerBiome, CurrentBiome, PlayerLocation);
        CurrentPlayerBiome = CurrentBiome;
    }
    
    // Calculate transition weights for all biomes
    TMap<EWorld_BiomeType, float> TransitionWeights;
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.bIsActive)
        {
            float Weight = GetTransitionWeight(PlayerLocation, Zone.BiomeType);
            if (Weight > 0.0f)
            {
                TransitionWeights.Add(Zone.BiomeType, Weight);
            }
        }
    }
    
    // Apply transition effects
    ApplyTransitionEffects(TransitionWeights, PlayerLocation);
}

void AWorld_BiomeTransitionManager::OnBiomeTransition(EWorld_BiomeType FromBiome, EWorld_BiomeType ToBiome, const FVector& Location)
{
    FString FromBiomeName = GetBiomeTypeName(FromBiome);
    FString ToBiomeName = GetBiomeTypeName(ToBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Player moved from %s to %s at location %s"), 
           *FromBiomeName, *ToBiomeName, *Location.ToString());
    
    // Trigger biome-specific events
    switch (ToBiome)
    {
        case EWorld_BiomeType::Pantano:
            OnEnterPantano();
            break;
        case EWorld_BiomeType::Floresta:
            OnEnterFloresta();
            break;
        case EWorld_BiomeType::Savana:
            OnEnterSavana();
            break;
        case EWorld_BiomeType::Deserto:
            OnEnterDeserto();
            break;
        case EWorld_BiomeType::MontanhaNevada:
            OnEnterMontanha();
            break;
    }
}

void AWorld_BiomeTransitionManager::ApplyTransitionEffects(const TMap<EWorld_BiomeType, float>& TransitionWeights, const FVector& Location)
{
    // Apply environmental effects based on transition weights
    for (const auto& WeightPair : TransitionWeights)
    {
        EWorld_BiomeType BiomeType = WeightPair.Key;
        float Weight = WeightPair.Value;
        
        // Apply biome-specific effects with weight influence
        ApplyBiomeEffects(BiomeType, Weight, Location);
    }
}

void AWorld_BiomeTransitionManager::ApplyBiomeEffects(EWorld_BiomeType BiomeType, float Weight, const FVector& Location)
{
    // Base implementation - can be overridden in Blueprint or extended
    switch (BiomeType)
    {
        case EWorld_BiomeType::Pantano:
            // Apply swamp effects: humidity, fog, slower movement
            break;
        case EWorld_BiomeType::Floresta:
            // Apply forest effects: shade, ambient sounds, visibility reduction
            break;
        case EWorld_BiomeType::Savana:
            // Apply savanna effects: open visibility, wind, temperature variation
            break;
        case EWorld_BiomeType::Deserto:
            // Apply desert effects: heat, sandstorms, dehydration
            break;
        case EWorld_BiomeType::MontanhaNevada:
            // Apply mountain effects: cold, altitude, snow, reduced oxygen
            break;
    }
}

FString AWorld_BiomeTransitionManager::GetBiomeTypeName(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Pantano: return TEXT("Pantano");
        case EWorld_BiomeType::Floresta: return TEXT("Floresta");
        case EWorld_BiomeType::Savana: return TEXT("Savana");
        case EWorld_BiomeType::Deserto: return TEXT("Deserto");
        case EWorld_BiomeType::MontanhaNevada: return TEXT("MontanhaNevada");
        default: return TEXT("Unknown");
    }
}

void AWorld_BiomeTransitionManager::OnEnterPantano()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Entered Pantano biome - applying swamp effects"));
    // Implement swamp-specific logic
}

void AWorld_BiomeTransitionManager::OnEnterFloresta()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Entered Floresta biome - applying forest effects"));
    // Implement forest-specific logic
}

void AWorld_BiomeTransitionManager::OnEnterSavana()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Entered Savana biome - applying savanna effects"));
    // Implement savanna-specific logic
}

void AWorld_BiomeTransitionManager::OnEnterDeserto()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Entered Deserto biome - applying desert effects"));
    // Implement desert-specific logic
}

void AWorld_BiomeTransitionManager::OnEnterMontanha()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Entered Montanha biome - applying mountain effects"));
    // Implement mountain-specific logic
}

void AWorld_BiomeTransitionManager::SetTransitionDistance(float NewDistance)
{
    TransitionDistance = FMath::Max(NewDistance, 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Set transition distance to %f"), TransitionDistance);
}

void AWorld_BiomeTransitionManager::SetBlendRadius(float NewRadius)
{
    BlendRadius = FMath::Max(NewRadius, 500.0f);
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Set blend radius to %f"), BlendRadius);
}

void AWorld_BiomeTransitionManager::EnableRealTimeTransitions(bool bEnable)
{
    bEnableRealTimeTransitions = bEnable;
    
    if (bEnable && !GetWorldTimerManager().IsTimerActive(TransitionUpdateTimerHandle))
    {
        GetWorldTimerManager().SetTimer(
            TransitionUpdateTimerHandle,
            this,
            &AWorld_BiomeTransitionManager::UpdateBiomeTransitions,
            TransitionUpdateInterval,
            true
        );
    }
    else if (!bEnable && GetWorldTimerManager().IsTimerActive(TransitionUpdateTimerHandle))
    {
        GetWorldTimerManager().ClearTimer(TransitionUpdateTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeTransition: Real-time transitions %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}