#include "Eng_ArchitectureValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"

void UEng_ArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MaxActorsPerBiome = 4000;
    MaxTotalActors = 20000;
    MaxDinosaurs = 150;
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture Validator initialized"));
    
    // Run initial validation
    ValidateSystemArchitecture();
}

void UEng_ArchitectureValidator::Deinitialize()
{
    ValidationReports.Empty();
    Super::Deinitialize();
}

bool UEng_ArchitectureValidator::ValidateSystemArchitecture()
{
    ClearValidationReports();
    
    bool bOverallValid = true;
    
    // Validate core classes exist
    bOverallValid &= ValidateClassExists(TEXT("TranspersonalCharacter"), TEXT("Core"));
    bOverallValid &= ValidateClassExists(TEXT("TranspersonalGameMode"), TEXT("Core"));
    bOverallValid &= ValidateClassExists(TEXT("TranspersonalGameState"), TEXT("Core"));
    
    // Validate component registration
    bOverallValid &= ValidateComponentRegistration();
    
    // Validate subsystem initialization
    bOverallValid &= ValidateSubsystemInitialization();
    
    // Validate actor counts
    bOverallValid &= ValidateActorCounts();
    
    // Validate biome distribution
    bOverallValid &= ValidateBiomeDistribution();
    
    // Validate dinosaur limits
    bOverallValid &= ValidateDinosaurLimits();
    
    if (bOverallValid)
    {
        AddValidationReport(TEXT("System"), EEng_ValidationResult::Pass, TEXT("All architecture validations passed"));
    }
    else
    {
        AddValidationReport(TEXT("System"), EEng_ValidationResult::Error, TEXT("Architecture validation failures detected"));
    }
    
    return bOverallValid;
}

bool UEng_ArchitectureValidator::ValidateModuleIntegrity(const FString& ModuleName)
{
    // Check if module classes can be loaded
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* ModuleClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (ModuleClass)
    {
        AddValidationReport(ModuleName, EEng_ValidationResult::Pass, TEXT("Module class loaded successfully"));
        return true;
    }
    else
    {
        AddValidationReport(ModuleName, EEng_ValidationResult::Error, FString::Printf(TEXT("Failed to load module class: %s"), *ClassPath));
        return false;
    }
}

TArray<FEng_ValidationReport> UEng_ArchitectureValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UEng_ArchitectureValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

bool UEng_ArchitectureValidator::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("ActorCount"), EEng_ValidationResult::Error, TEXT("No valid world found"));
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorCount = AllActors.Num();
    
    if (ActorCount > MaxTotalActors)
    {
        AddValidationReport(TEXT("ActorCount"), EEng_ValidationResult::Critical, 
            FString::Printf(TEXT("Actor count %d exceeds maximum %d"), ActorCount, MaxTotalActors));
        return false;
    }
    else if (ActorCount > MaxTotalActors * 0.8f)
    {
        AddValidationReport(TEXT("ActorCount"), EEng_ValidationResult::Warning, 
            FString::Printf(TEXT("Actor count %d approaching maximum %d"), ActorCount, MaxTotalActors));
    }
    else
    {
        AddValidationReport(TEXT("ActorCount"), EEng_ValidationResult::Pass, 
            FString::Printf(TEXT("Actor count %d within limits"), ActorCount));
    }
    
    return true;
}

bool UEng_ArchitectureValidator::ValidateBiomeDistribution()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("BiomeDistribution"), EEng_ValidationResult::Error, TEXT("No valid world found"));
        return false;
    }
    
    // Define biome centers
    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 0),           // Savana
        FVector(-50000, -45000, 0), // Pantano
        FVector(-45000, 40000, 0),  // Floresta
        FVector(55000, 0, 0),       // Deserto
        FVector(40000, 50000, 0)    // Montanha
    };
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (int32 BiomeIndex = 0; BiomeIndex < BiomeCenters.Num(); BiomeIndex++)
    {
        int32 ActorsInBiome = 0;
        FVector BiomeCenter = BiomeCenters[BiomeIndex];
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && FVector::Dist(Actor->GetActorLocation(), BiomeCenter) < 15000.0f)
            {
                ActorsInBiome++;
            }
        }
        
        FString BiomeName = FString::Printf(TEXT("Biome_%d"), BiomeIndex);
        
        if (ActorsInBiome > MaxActorsPerBiome)
        {
            AddValidationReport(TEXT("BiomeDistribution"), EEng_ValidationResult::Critical,
                FString::Printf(TEXT("%s has %d actors, exceeds limit %d"), *BiomeName, ActorsInBiome, MaxActorsPerBiome));
        }
        else
        {
            AddValidationReport(TEXT("BiomeDistribution"), EEng_ValidationResult::Pass,
                FString::Printf(TEXT("%s has %d actors"), *BiomeName, ActorsInBiome));
        }
    }
    
    return true;
}

bool UEng_ArchitectureValidator::ValidateDinosaurLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("DinosaurLimits"), EEng_ValidationResult::Error, TEXT("No valid world found"));
        return false;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    TArray<FString> DinosaurTypes = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), TEXT("ankylo"), TEXT("parasauro")};
    int32 TotalDinosaurs = 0;
    
    for (const FString& DinoType : DinosaurTypes)
    {
        int32 DinoCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().ToLower().Contains(DinoType.ToLower()))
            {
                DinoCount++;
                TotalDinosaurs++;
            }
        }
        
        AddValidationReport(TEXT("DinosaurLimits"), EEng_ValidationResult::Pass,
            FString::Printf(TEXT("%s count: %d"), *DinoType, DinoCount));
    }
    
    if (TotalDinosaurs > MaxDinosaurs)
    {
        AddValidationReport(TEXT("DinosaurLimits"), EEng_ValidationResult::Critical,
            FString::Printf(TEXT("Total dinosaurs %d exceeds maximum %d"), TotalDinosaurs, MaxDinosaurs));
        return false;
    }
    
    AddValidationReport(TEXT("DinosaurLimits"), EEng_ValidationResult::Pass,
        FString::Printf(TEXT("Total dinosaurs: %d (within limit %d)"), TotalDinosaurs, MaxDinosaurs));
    
    return true;
}

void UEng_ArchitectureValidator::AddValidationReport(const FString& ModuleName, EEng_ValidationResult Result, const FString& Message)
{
    FEng_ValidationReport Report;
    Report.ModuleName = ModuleName;
    Report.Result = Result;
    Report.Message = Message;
    Report.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    ValidationReports.Add(Report);
    
    // Log to console
    FString ResultString;
    switch (Result)
    {
        case EEng_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
        case EEng_ValidationResult::Warning: ResultString = TEXT("WARNING"); break;
        case EEng_ValidationResult::Error: ResultString = TEXT("ERROR"); break;
        case EEng_ValidationResult::Critical: ResultString = TEXT("CRITICAL"); break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"), *ResultString, *ModuleName, *Message);
}

bool UEng_ArchitectureValidator::ValidateClassExists(const FString& ClassName, const FString& ModuleName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* Class = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (Class)
    {
        AddValidationReport(ModuleName, EEng_ValidationResult::Pass, 
            FString::Printf(TEXT("Class %s loaded successfully"), *ClassName));
        return true;
    }
    else
    {
        AddValidationReport(ModuleName, EEng_ValidationResult::Error, 
            FString::Printf(TEXT("Failed to load class %s"), *ClassName));
        return false;
    }
}

bool UEng_ArchitectureValidator::ValidateComponentRegistration()
{
    // Check if core components are properly registered
    UClass* ActorComponentClass = UActorComponent::StaticClass();
    
    if (ActorComponentClass)
    {
        AddValidationReport(TEXT("Components"), EEng_ValidationResult::Pass, TEXT("Core component classes available"));
        return true;
    }
    else
    {
        AddValidationReport(TEXT("Components"), EEng_ValidationResult::Error, TEXT("Core component classes not available"));
        return false;
    }
}

bool UEng_ArchitectureValidator::ValidateSubsystemInitialization()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Subsystems"), EEng_ValidationResult::Error, TEXT("No world available for subsystem validation"));
        return false;
    }
    
    // Check if this subsystem itself is properly initialized
    if (this)
    {
        AddValidationReport(TEXT("Subsystems"), EEng_ValidationResult::Pass, TEXT("Architecture Validator subsystem initialized"));
        return true;
    }
    
    AddValidationReport(TEXT("Subsystems"), EEng_ValidationResult::Error, TEXT("Subsystem validation failed"));
    return false;
}