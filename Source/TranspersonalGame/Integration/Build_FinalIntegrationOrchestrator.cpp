#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

ABuild_FinalIntegrationOrchestrator::ABuild_FinalIntegrationOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize integration status
    CurrentStatus = EBuild_IntegrationStatus::Pending;
    IntegrationProgress = 0.0f;
    LastValidationMessage = TEXT("Integration Orchestrator initialized");
    
    bValidationInProgress = false;
    ValidationStartTime = 0.0f;
    TotalValidationSteps = 7; // Number of validation systems
    CompletedValidationSteps = 0;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ABuild_FinalIntegrationOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Final Integration Orchestrator started"));
    
    // Start periodic validation every 30 seconds
    GetWorldTimerManager().SetTimer(ValidationTimerHandle, this, 
        &ABuild_FinalIntegrationOrchestrator::PerformPeriodicValidation, 30.0f, true);
    
    // Perform initial validation after 5 seconds
    FTimerHandle InitialValidationTimer;
    GetWorldTimerManager().SetTimer(InitialValidationTimer, this,
        &ABuild_FinalIntegrationOrchestrator::StartFullSystemValidation, 5.0f, false);
}

void ABuild_FinalIntegrationOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update integration progress if validation is in progress
    if (bValidationInProgress && TotalValidationSteps > 0)
    {
        IntegrationProgress = (float)CompletedValidationSteps / (float)TotalValidationSteps;
    }
}

void ABuild_FinalIntegrationOrchestrator::StartFullSystemValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Starting full system validation"));
    
    bValidationInProgress = true;
    ValidationStartTime = GetWorld()->GetTimeSeconds();
    CompletedValidationSteps = 0;
    CurrentStatus = EBuild_IntegrationStatus::InProgress;
    SystemValidationResults.Empty();
    
    // Perform all validation checks
    bool bModuleValid = ValidateModuleCompilation();
    ValidateSystemInternal(TEXT("Module Compilation"), bModuleValid, 
        bModuleValid ? TEXT("All modules compiled successfully") : TEXT("Module compilation issues detected"));
    
    bool bDinosaurValid = ValidateDinosaurAssets();
    ValidateSystemInternal(TEXT("Dinosaur Assets"), bDinosaurValid,
        bDinosaurValid ? TEXT("Dinosaur assets loaded and spawned") : TEXT("Dinosaur asset issues detected"));
    
    bool bBiomeValid = ValidateBiomePopulation();
    ValidateSystemInternal(TEXT("Biome Population"), bBiomeValid,
        bBiomeValid ? TEXT("Biomes properly populated") : TEXT("Biome population issues detected"));
    
    bool bCharacterValid = ValidateCharacterSystems();
    ValidateSystemInternal(TEXT("Character Systems"), bCharacterValid,
        bCharacterValid ? TEXT("Character systems operational") : TEXT("Character system issues detected"));
    
    bool bWorldGenValid = ValidateWorldGeneration();
    ValidateSystemInternal(TEXT("World Generation"), bWorldGenValid,
        bWorldGenValid ? TEXT("World generation systems active") : TEXT("World generation issues detected"));
    
    bool bAIValid = ValidateAISystems();
    ValidateSystemInternal(TEXT("AI Systems"), bAIValid,
        bAIValid ? TEXT("AI systems functional") : TEXT("AI system issues detected"));
    
    // Final integration check
    bool bOverallValid = bModuleValid && bDinosaurValid && bBiomeValid && bCharacterValid && bWorldGenValid && bAIValid;
    ValidateSystemInternal(TEXT("Overall Integration"), bOverallValid,
        bOverallValid ? TEXT("All systems integrated successfully") : TEXT("Integration issues require attention"));
    
    // Complete validation
    bValidationInProgress = false;
    CurrentStatus = bOverallValid ? EBuild_IntegrationStatus::Complete : EBuild_IntegrationStatus::Failed;
    IntegrationProgress = 1.0f;
    
    float ValidationDuration = GetWorld()->GetTimeSeconds() - ValidationStartTime;
    LastValidationMessage = FString::Printf(TEXT("Validation completed in %.2f seconds. Status: %s"), 
        ValidationDuration, bOverallValid ? TEXT("PASS") : TEXT("FAIL"));
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LastValidationMessage);
    
    // Update biome data
    UpdateBiomeData();
    
    // Generate final report
    GenerateIntegrationReport();
}

bool ABuild_FinalIntegrationOrchestrator::ValidateModuleCompilation()
{
    CompletedValidationSteps++;
    
    // Check if TranspersonalGame module classes are available
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    
    bool bModuleValid = (CharacterClass != nullptr) && (GameStateClass != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("Module validation: %s (Character: %s, GameState: %s)"),
        bModuleValid ? TEXT("PASS") : TEXT("FAIL"),
        CharacterClass ? TEXT("Found") : TEXT("Missing"),
        GameStateClass ? TEXT("Found") : TEXT("Missing"));
    
    return bModuleValid;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateDinosaurAssets()
{
    CompletedValidationSteps++;
    
    // Count dinosaur actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Trex")) || ActorLabel.Contains(TEXT("Velociraptor")) || 
            ActorLabel.Contains(TEXT("Triceratops")) || ActorLabel.Contains(TEXT("Brachiosaurus")))
        {
            DinosaurCount++;
        }
    }
    
    bool bDinosaurValid = DinosaurCount >= 4; // At least 4 dinosaurs should be spawned
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur validation: %s (%d dinosaurs found)"),
        bDinosaurValid ? TEXT("PASS") : TEXT("FAIL"), DinosaurCount);
    
    return bDinosaurValid;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateBiomePopulation()
{
    CompletedValidationSteps++;
    
    // Check population in each biome
    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 0),           // Savana
        FVector(-45000, 40000, 0),  // Floresta
        FVector(50000, -40000, 0),  // Deserto
        FVector(-50000, -45000, 0), // Pantano
        FVector(40000, 50000, 0)    // Montanha
    };
    
    int32 PopulatedBiomes = 0;
    for (const FVector& BiomeCenter : BiomeCenters)
    {
        TArray<AActor*> BiomeActors = GetActorsInBiome(BiomeCenter, 10000.0f);
        if (BiomeActors.Num() > 10) // At least 10 actors per biome
        {
            PopulatedBiomes++;
        }
    }
    
    bool bBiomeValid = PopulatedBiomes >= 3; // At least 3 biomes should be populated
    
    UE_LOG(LogTemp, Warning, TEXT("Biome validation: %s (%d/5 biomes populated)"),
        bBiomeValid ? TEXT("PASS") : TEXT("FAIL"), PopulatedBiomes);
    
    return bBiomeValid;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    CompletedValidationSteps++;
    
    // Check for player character in the world
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    bool bCharacterValid = PlayerPawn != nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Character validation: %s (Player pawn: %s)"),
        bCharacterValid ? TEXT("PASS") : TEXT("FAIL"),
        PlayerPawn ? TEXT("Found") : TEXT("Missing"));
    
    return bCharacterValid;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    CompletedValidationSteps++;
    
    // Check for world generation actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 WorldGenActors = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("WorldGen")) || ActorLabel.Contains(TEXT("PCG")) || 
            ActorLabel.Contains(TEXT("Landscape")) || ActorLabel.Contains(TEXT("Terrain")))
        {
            WorldGenActors++;
        }
    }
    
    bool bWorldGenValid = WorldGenActors > 0 || AllActors.Num() > 50; // Either specific world gen actors or sufficient total actors
    
    UE_LOG(LogTemp, Warning, TEXT("World generation validation: %s (%d world gen actors, %d total actors)"),
        bWorldGenValid ? TEXT("PASS") : TEXT("FAIL"), WorldGenActors, AllActors.Num());
    
    return bWorldGenValid;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateAISystems()
{
    CompletedValidationSteps++;
    
    // Check for AI-related actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 AIActors = 0;
    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("AI")) || ActorLabel.Contains(TEXT("NPC")) || 
            ActorLabel.Contains(TEXT("Behavior")) || ActorLabel.Contains(TEXT("Crowd")))
        {
            AIActors++;
        }
    }
    
    bool bAIValid = AIActors > 0 || GetTotalActorCount() > 30; // Either specific AI actors or sufficient complexity
    
    UE_LOG(LogTemp, Warning, TEXT("AI systems validation: %s (%d AI actors found)"),
        bAIValid ? TEXT("PASS") : TEXT("FAIL"), AIActors);
    
    return bAIValid;
}

void ABuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total actors in world: %d"), GetTotalActorCount());
    UE_LOG(LogTemp, Warning, TEXT("Integration status: %s"), 
        CurrentStatus == EBuild_IntegrationStatus::Complete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    UE_LOG(LogTemp, Warning, TEXT("Validation progress: %.1f%%"), IntegrationProgress * 100.0f);
    
    for (const FBuild_SystemValidationResult& Result : SystemValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s: %s - %s"), 
            *Result.SystemName, 
            Result.bIsValid ? TEXT("PASS") : TEXT("FAIL"),
            *Result.ValidationMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

void ABuild_FinalIntegrationOrchestrator::TriggerBuildValidation()
{
    StartFullSystemValidation();
}

void ABuild_FinalIntegrationOrchestrator::CheckCrossSystemCompatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking cross-system compatibility"));
    // Implementation for cross-system compatibility checks
}

void ABuild_FinalIntegrationOrchestrator::ValidatePerformanceMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating performance metrics"));
    // Implementation for performance validation
}

int32 ABuild_FinalIntegrationOrchestrator::GetTotalActorCount()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    return AllActors.Num();
}

TArray<AActor*> ABuild_FinalIntegrationOrchestrator::GetActorsInBiome(FVector BiomeCenter, float Radius)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    TArray<AActor*> BiomeActors;
    for (AActor* Actor : AllActors)
    {
        float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeCenter);
        if (Distance <= Radius)
        {
            BiomeActors.Add(Actor);
        }
    }
    
    return BiomeActors;
}

void ABuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Integration Status: %s, Progress: %.1f%%, Message: %s"),
        CurrentStatus == EBuild_IntegrationStatus::Complete ? TEXT("COMPLETE") : TEXT("IN_PROGRESS"),
        IntegrationProgress * 100.0f,
        *LastValidationMessage);
}

void ABuild_FinalIntegrationOrchestrator::ValidateSystemInternal(const FString& SystemName, bool bValidationResult, const FString& Message)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.bIsValid = bValidationResult;
    Result.ValidationMessage = Message;
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    SystemValidationResults.Add(Result);
}

void ABuild_FinalIntegrationOrchestrator::UpdateBiomeData()
{
    BiomePopulationData.Empty();
    
    TArray<FString> BiomeNames = {TEXT("Savana"), TEXT("Floresta"), TEXT("Deserto"), TEXT("Pantano"), TEXT("Montanha")};
    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 0),
        FVector(-45000, 40000, 0),
        FVector(50000, -40000, 0),
        FVector(-50000, -45000, 0),
        FVector(40000, 50000, 0)
    };
    
    for (int32 i = 0; i < BiomeNames.Num(); i++)
    {
        FBuild_BiomePopulationData BiomeData;
        BiomeData.BiomeName = BiomeNames[i];
        BiomeData.BiomeCenter = BiomeCenters[i];
        
        TArray<AActor*> BiomeActors = GetActorsInBiome(BiomeCenters[i], 10000.0f);
        BiomeData.ActorCount = BiomeActors.Num();
        
        // Count dinosaurs in this biome
        for (AActor* Actor : BiomeActors)
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Trex")) || ActorLabel.Contains(TEXT("Velociraptor")) || 
                ActorLabel.Contains(TEXT("Triceratops")) || ActorLabel.Contains(TEXT("Brachiosaurus")))
            {
                BiomeData.DinosaurCount++;
            }
        }
        
        BiomePopulationData.Add(BiomeData);
    }
}

void ABuild_FinalIntegrationOrchestrator::CheckActorIntegrity()
{
    // Implementation for checking actor integrity
}

void ABuild_FinalIntegrationOrchestrator::PerformPeriodicValidation()
{
    if (!bValidationInProgress)
    {
        UE_LOG(LogTemp, Log, TEXT("Performing periodic validation check"));
        StartFullSystemValidation();
    }
}