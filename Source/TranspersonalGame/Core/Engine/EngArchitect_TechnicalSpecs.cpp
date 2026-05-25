#include "EngArchitect_TechnicalSpecs.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"

UEngArchitect_TechnicalSpecs::UEngArchitect_TechnicalSpecs()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default specifications
    PerformanceSpecs = FEng_PerformanceSpecs();
    BiomeCoordinates = FEng_BiomeCoordinates();
    SurvivalSpecs = FEng_SurvivalSpecs();
}

void UEngArchitect_TechnicalSpecs::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Technical Specs initialized"));
    ValidatePerformanceMetrics();
}

bool UEngArchitect_TechnicalSpecs::ValidatePerformanceMetrics()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("EngArchitect: No world context for performance validation"));
        return false;
    }
    
    // Count total actors in world
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArchitect Performance Check: %d total actors in world"), TotalActors);
    
    // Validate against performance specs
    bool bPerformanceOK = true;
    
    if (TotalActors > (PerformanceSpecs.MaxActorsPerBiome * 5)) // 5 biomes
    {
        UE_LOG(LogTemp, Error, TEXT("EngArchitect: Actor count exceeds maximum (%d > %d)"), 
               TotalActors, PerformanceSpecs.MaxActorsPerBiome * 5);
        bPerformanceOK = false;
    }
    
    // Check current framerate (if available)
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    if (CurrentFPS < PerformanceSpecs.TargetFPS_PC * 0.8f) // 80% threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArchitect: FPS below target (%.1f < %d)"), 
               CurrentFPS, PerformanceSpecs.TargetFPS_PC);
    }
    
    return bPerformanceOK;
}

FVector UEngArchitect_TechnicalSpecs::GetBiomeCenter(const FString& BiomeName)
{
    if (BiomeName == TEXT("Savana"))
        return BiomeCoordinates.Savana;
    else if (BiomeName == TEXT("Pantano"))
        return BiomeCoordinates.Pantano;
    else if (BiomeName == TEXT("Floresta"))
        return BiomeCoordinates.Floresta;
    else if (BiomeName == TEXT("Deserto"))
        return BiomeCoordinates.Deserto;
    else if (BiomeName == TEXT("Montanha"))
        return BiomeCoordinates.Montanha;
    
    UE_LOG(LogTemp, Warning, TEXT("EngArchitect: Unknown biome name '%s', returning Savana"), *BiomeName);
    return BiomeCoordinates.Savana;
}

bool UEngArchitect_TechnicalSpecs::IsLocationInBiome(const FVector& Location, const FString& BiomeName)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeName);
    float BiomeRadius = 20000.0f; // 20km radius per biome
    
    float Distance = FVector::Dist2D(Location, BiomeCenter);
    return Distance <= BiomeRadius;
}

void UEngArchitect_TechnicalSpecs::ValidateWorldSetup()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("EngArchitect: No world context for validation"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT WORLD VALIDATION ==="));
    
    // Check biome population
    TArray<FString> BiomeNames = {TEXT("Savana"), TEXT("Pantano"), TEXT("Floresta"), TEXT("Deserto"), TEXT("Montanha")};
    
    for (const FString& BiomeName : BiomeNames)
    {
        FVector BiomeCenter = GetBiomeCenter(BiomeName);
        int32 ActorsInBiome = 0;
        
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsLocationInBiome(Actor->GetActorLocation(), BiomeName))
            {
                ActorsInBiome++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors at center (%.0f, %.0f, %.0f)"), 
               *BiomeName, ActorsInBiome, BiomeCenter.X, BiomeCenter.Y, BiomeCenter.Z);
    }
    
    // Count dinosaurs specifically
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Rex")) || ActorLabel.Contains(TEXT("raptor")) || 
            ActorLabel.Contains(TEXT("Brachio")) || ActorLabel.Contains(TEXT("Tricera")))
        {
            DinosaurCount++;
            UE_LOG(LogTemp, Warning, TEXT("Found dinosaur: %s at (%.0f, %.0f, %.0f)"), 
                   *ActorLabel, Actor->GetActorLocation().X, Actor->GetActorLocation().Y, Actor->GetActorLocation().Z);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total dinosaurs in world: %d"), DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION COMPLETE ==="));
}

void UEngArchitect_TechnicalSpecs::EnforceArchitecturalConstraints()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("EngArchitect: No world context for constraint enforcement"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENFORCING ARCHITECTURAL CONSTRAINTS ==="));
    
    // Remove excess actors if over limit
    int32 TotalActors = 0;
    TArray<AActor*> AllActors;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
        TotalActors++;
    }
    
    int32 MaxAllowedActors = PerformanceSpecs.MaxActorsPerBiome * 5;
    if (TotalActors > MaxAllowedActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArchitect: Removing %d excess actors"), TotalActors - MaxAllowedActors);
        
        // Remove actors starting from the end (usually less important)
        for (int32 i = AllActors.Num() - 1; i >= MaxAllowedActors; i--)
        {
            if (AllActors[i] && !AllActors[i]->GetActorLabel().Contains(TEXT("Rex")) && 
                !AllActors[i]->GetActorLabel().Contains(TEXT("raptor"))) // Preserve dinosaurs
            {
                AllActors[i]->Destroy();
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArchitect: Constraints enforced. Final actor count: %d"), TotalActors);
}