#include "Build_IntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    LastPerformanceScore = 1.0f;
    TotalActorCount = 0;
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::ValidateAllSystems()
{
    LastValidationResults.Empty();
    
    // Validate core systems
    LastValidationResults.Add(ValidateCharacterSystem());
    LastValidationResults.Add(ValidateWorldGeneration());
    LastValidationResults.Add(ValidateDinosaurAI());
    LastValidationResults.Add(ValidateCombatSystem());
    LastValidationResults.Add(ValidateAudioSystem());
    LastValidationResults.Add(ValidateVFXSystem());
    
    // Calculate overall performance
    LastPerformanceScore = CalculatePerformanceScore();
    
    return LastValidationResults;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateSystemIntegration(const FString& SystemName)
{
    if (SystemName == TEXT("Character"))
    {
        return ValidateCharacterSystem();
    }
    else if (SystemName == TEXT("WorldGen"))
    {
        return ValidateWorldGeneration();
    }
    else if (SystemName == TEXT("DinosaurAI"))
    {
        return ValidateDinosaurAI();
    }
    else if (SystemName == TEXT("Combat"))
    {
        return ValidateCombatSystem();
    }
    else if (SystemName == TEXT("Audio"))
    {
        return ValidateAudioSystem();
    }
    else if (SystemName == TEXT("VFX"))
    {
        return ValidateVFXSystem();
    }
    
    FBuild_ValidationReport Report;
    Report.SystemName = SystemName;
    Report.Result = EBuild_ValidationResult::Error;
    Report.Message = TEXT("Unknown system name");
    return Report;
}

bool UBuild_IntegrationValidator::ValidateBiomeDistribution()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Define biome centers as per memory requirements
    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 0),           // Savana
        FVector(-50000, -45000, 0), // Pantano
        FVector(-45000, 40000, 0),  // Floresta
        FVector(55000, 0, 0),       // Deserto
        FVector(40000, 50000, 0)    // Montanha
    };
    
    float BiomeRadius = 20000.0f;
    int32 TotalInBiomes = 0;
    
    for (const FVector& Center : BiomeCenters)
    {
        int32 ActorsInBiome = CountActorsInBiome(Center, BiomeRadius);
        TotalInBiomes += ActorsInBiome;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();
    
    // Check if distribution is reasonable (at least 60% of actors in biomes)
    float DistributionRatio = TotalActorCount > 0 ? (float)TotalInBiomes / TotalActorCount : 0.0f;
    return DistributionRatio >= 0.6f;
}

float UBuild_IntegrationValidator::CalculatePerformanceScore()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();
    
    // Performance scoring based on actor count
    float Score = 1.0f;
    
    if (TotalActorCount > 50000)
    {
        Score = 0.3f; // Critical performance impact
    }
    else if (TotalActorCount > 20000)
    {
        Score = 0.6f; // High performance impact
    }
    else if (TotalActorCount > 10000)
    {
        Score = 0.8f; // Moderate performance impact
    }
    
    // Bonus for good biome distribution
    if (ValidateBiomeDistribution())
    {
        Score += 0.1f;
    }
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void UBuild_IntegrationValidator::GenerateIntegrationReport()
{
    ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), LastPerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("Biome Distribution Valid: %s"), ValidateBiomeDistribution() ? TEXT("YES") : TEXT("NO"));
    
    for (const FBuild_ValidationReport& Report : LastValidationResults)
    {
        FString ResultStr;
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Success: ResultStr = TEXT("SUCCESS"); break;
            case EBuild_ValidationResult::Warning: ResultStr = TEXT("WARNING"); break;
            case EBuild_ValidationResult::Error: ResultStr = TEXT("ERROR"); break;
            case EBuild_ValidationResult::Critical: ResultStr = TEXT("CRITICAL"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), *Report.SystemName, *ResultStr, *Report.Message);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateCharacterSystem()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("Character System");
    
    if (CheckClassLoaded(TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = TEXT("Character system loaded successfully");
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = TEXT("Character system failed to load");
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateWorldGeneration()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("World Generation");
    
    if (CheckClassLoaded(TEXT("/Script/TranspersonalGame.PCGWorldGenerator")))
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = TEXT("World generation system loaded successfully");
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = TEXT("World generation system not fully loaded");
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateDinosaurAI()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("Dinosaur AI");
    
    bool TRexLoaded = CheckClassLoaded(TEXT("/Script/TranspersonalGame.DinosaurTRex"));
    bool AILoaded = CheckClassLoaded(TEXT("/Script/TranspersonalGame.DinosaurCombatAIController"));
    
    if (TRexLoaded && AILoaded)
    {
        Report.Result = EBuild_ValidationResult::Success;
        Report.Message = TEXT("Dinosaur AI system fully operational");
    }
    else if (TRexLoaded || AILoaded)
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = TEXT("Dinosaur AI system partially loaded");
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Error;
        Report.Message = TEXT("Dinosaur AI system failed to load");
    }
    
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateCombatSystem()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("Combat System");
    Report.Result = EBuild_ValidationResult::Warning;
    Report.Message = TEXT("Combat system validation not implemented");
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateAudioSystem()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("Audio System");
    Report.Result = EBuild_ValidationResult::Warning;
    Report.Message = TEXT("Audio system validation not implemented");
    return Report;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateVFXSystem()
{
    FBuild_ValidationReport Report;
    Report.SystemName = TEXT("VFX System");
    Report.Result = EBuild_ValidationResult::Warning;
    Report.Message = TEXT("VFX system validation not implemented");
    return Report;
}

bool UBuild_IntegrationValidator::CheckClassLoaded(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

int32 UBuild_IntegrationValidator::CountActorsInBiome(const FVector& BiomeCenter, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 Count = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
            if (Distance <= Radius)
            {
                Count++;
            }
        }
    }
    
    return Count;
}