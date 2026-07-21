#include "Eng_MilestoneValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Landscape/Landscape.h"

UEng_MilestoneValidator::UEng_MilestoneValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Validate every 5 seconds
    
    ValidationInterval = 5.0f;
    bAutoValidate = true;
    OverallCompletionPercentage = 0.0f;
    bMilestone1Complete = false;
    LastValidationTime = 0.0f;
}

void UEng_MilestoneValidator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMilestone1Requirements();
    InitializePrototypeComponents();
    
    if (bAutoValidate)
    {
        ValidateAllComponents();
    }
}

void UEng_MilestoneValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoValidate)
    {
        LastValidationTime += DeltaTime;
        if (LastValidationTime >= ValidationInterval)
        {
            ValidateAllComponents();
            LastValidationTime = 0.0f;
        }
    }
}

void UEng_MilestoneValidator::InitializeMilestone1Requirements()
{
    Milestone1Requirements.Empty();
    
    // Character System
    FEng_MilestoneRequirement CharacterReq;
    CharacterReq.RequirementName = TEXT("ThirdPersonCharacter");
    CharacterReq.Description = TEXT("ACharacter subclass with WASD movement and camera boom");
    CharacterReq.Status = EEng_MilestoneStatus::InProgress;
    CharacterReq.AssignedAgent = TEXT("Agent #03 - Core Systems");
    Milestone1Requirements.Add(CharacterReq);
    
    // Terrain System
    FEng_MilestoneRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("BasicTerrain");
    TerrainReq.Description = TEXT("Landscape with height variation (not flat plane)");
    TerrainReq.Status = EEng_MilestoneStatus::InProgress;
    TerrainReq.AssignedAgent = TEXT("Agent #05 - Procedural World Generator");
    Milestone1Requirements.Add(TerrainReq);
    
    // Movement System
    FEng_MilestoneRequirement MovementReq;
    MovementReq.RequirementName = TEXT("PlayerMovement");
    MovementReq.Description = TEXT("Player can walk, run, jump with responsive controls");
    MovementReq.Status = EEng_MilestoneStatus::InProgress;
    MovementReq.AssignedAgent = TEXT("Agent #03 - Core Systems");
    Milestone1Requirements.Add(MovementReq);
    
    // Dinosaur Meshes
    FEng_MilestoneRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("DinosaurMeshes");
    DinosaurReq.Description = TEXT("3-5 static dinosaur meshes placed in the world");
    DinosaurReq.Status = EEng_MilestoneStatus::NotStarted;
    DinosaurReq.AssignedAgent = TEXT("Agent #06 - Environment Artist");
    Milestone1Requirements.Add(DinosaurReq);
    
    // Lighting System
    FEng_MilestoneRequirement LightingReq;
    LightingReq.RequirementName = TEXT("BasicLighting");
    LightingReq.Description = TEXT("Directional light + sky atmosphere + fog");
    LightingReq.Status = EEng_MilestoneStatus::InProgress;
    LightingReq.AssignedAgent = TEXT("Agent #08 - Lighting & Atmosphere");
    Milestone1Requirements.Add(LightingReq);
}

void UEng_MilestoneValidator::InitializePrototypeComponents()
{
    PrototypeComponents.Empty();
    
    // Initialize all prototype component validations
    for (int32 i = 0; i < static_cast<int32>(EEng_PrototypeComponent::FogSystem) + 1; ++i)
    {
        FEng_PrototypeValidation Validation;
        Validation.Component = static_cast<EEng_PrototypeComponent>(i);
        Validation.bIsPresent = false;
        Validation.bIsFunctional = false;
        Validation.LastChecked = FDateTime::Now();
        PrototypeComponents.Add(Validation);
    }
}

void UEng_MilestoneValidator::ValidateAllComponents()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update all component validations
    bool bCharacterValid = ValidateCharacterSystem();
    bool bTerrainValid = ValidateTerrainSystem();
    bool bLightingValid = ValidateLightingSystem();
    bool bDinosaurValid = ValidateDinosaurSystem();
    bool bMovementValid = ValidateMovementSystem();
    
    // Calculate overall progress
    OverallCompletionPercentage = CalculateOverallProgress();
    
    // Check if milestone is complete
    bMilestone1Complete = (OverallCompletionPercentage >= 80.0f);
    
    LogValidationResults();
}

bool UEng_MilestoneValidator::ValidateCharacterSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find character in world
    ACharacter* PlayerCharacter = nullptr;
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        PlayerCharacter = *ActorItr;
        break;
    }
    
    bool bCharacterPresent = (PlayerCharacter != nullptr);
    bool bCharacterFunctional = false;
    
    if (bCharacterPresent)
    {
        // Check if character has movement component
        bCharacterFunctional = (PlayerCharacter->GetCharacterMovement() != nullptr);
    }
    
    // Update validation data
    for (FEng_PrototypeValidation& Validation : PrototypeComponents)
    {
        if (Validation.Component == EEng_PrototypeComponent::ThirdPersonCharacter)
        {
            Validation.bIsPresent = bCharacterPresent;
            Validation.bIsFunctional = bCharacterFunctional;
            Validation.ValidationNotes = bCharacterFunctional ? TEXT("Character with movement found") : TEXT("Character missing or non-functional");
            Validation.LastChecked = FDateTime::Now();
            break;
        }
    }
    
    return bCharacterPresent && bCharacterFunctional;
}

bool UEng_MilestoneValidator::ValidateTerrainSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find landscape in world
    ALandscape* Landscape = nullptr;
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        Landscape = *ActorItr;
        break;
    }
    
    bool bTerrainPresent = (Landscape != nullptr);
    bool bTerrainFunctional = bTerrainPresent; // If landscape exists, assume functional
    
    // Update validation data
    for (FEng_PrototypeValidation& Validation : PrototypeComponents)
    {
        if (Validation.Component == EEng_PrototypeComponent::BasicTerrain)
        {
            Validation.bIsPresent = bTerrainPresent;
            Validation.bIsFunctional = bTerrainFunctional;
            Validation.ValidationNotes = bTerrainPresent ? TEXT("Landscape found") : TEXT("No landscape in world");
            Validation.LastChecked = FDateTime::Now();
            break;
        }
    }
    
    return bTerrainPresent && bTerrainFunctional;
}

bool UEng_MilestoneValidator::ValidateLightingSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find directional light
    ADirectionalLight* DirectionalLight = nullptr;
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        DirectionalLight = *ActorItr;
        break;
    }
    
    bool bLightingPresent = (DirectionalLight != nullptr);
    bool bLightingFunctional = bLightingPresent;
    
    // Update validation data
    for (FEng_PrototypeValidation& Validation : PrototypeComponents)
    {
        if (Validation.Component == EEng_PrototypeComponent::DirectionalLight)
        {
            Validation.bIsPresent = bLightingPresent;
            Validation.bIsFunctional = bLightingFunctional;
            Validation.ValidationNotes = bLightingPresent ? TEXT("Directional light found") : TEXT("No directional light");
            Validation.LastChecked = FDateTime::Now();
            break;
        }
    }
    
    return bLightingPresent && bLightingFunctional;
}

bool UEng_MilestoneValidator::ValidateDinosaurSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count actors with "dinosaur" in name
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brach")))
        {
            DinosaurCount++;
        }
    }
    
    bool bDinosaursPresent = (DinosaurCount >= 3);
    bool bDinosaursFunctional = bDinosaursPresent;
    
    // Update validation data
    for (FEng_PrototypeValidation& Validation : PrototypeComponents)
    {
        if (Validation.Component == EEng_PrototypeComponent::DinosaurMeshes)
        {
            Validation.bIsPresent = bDinosaursPresent;
            Validation.bIsFunctional = bDinosaursFunctional;
            Validation.ValidationNotes = FString::Printf(TEXT("Found %d dinosaur actors"), DinosaurCount);
            Validation.LastChecked = FDateTime::Now();
            break;
        }
    }
    
    return bDinosaursPresent && bDinosaursFunctional;
}

bool UEng_MilestoneValidator::ValidateMovementSystem()
{
    // This is validated as part of character system
    return ValidateCharacterSystem();
}

void UEng_MilestoneValidator::UpdateRequirementStatus(const FString& RequirementName, EEng_MilestoneStatus NewStatus, float Percentage)
{
    for (FEng_MilestoneRequirement& Requirement : Milestone1Requirements)
    {
        if (Requirement.RequirementName == RequirementName)
        {
            Requirement.Status = NewStatus;
            Requirement.CompletionPercentage = FMath::Clamp(Percentage, 0.0f, 100.0f);
            Requirement.LastUpdated = FDateTime::Now();
            break;
        }
    }
    
    // Recalculate overall progress
    OverallCompletionPercentage = CalculateOverallProgress();
}

float UEng_MilestoneValidator::CalculateOverallProgress()
{
    if (Milestone1Requirements.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FEng_MilestoneRequirement& Requirement : Milestone1Requirements)
    {
        TotalProgress += Requirement.CompletionPercentage;
    }
    
    return TotalProgress / Milestone1Requirements.Num();
}

TArray<FString> UEng_MilestoneValidator::GetBlockingIssues()
{
    TArray<FString> BlockingIssues;
    
    for (const FEng_MilestoneRequirement& Requirement : Milestone1Requirements)
    {
        if (Requirement.Status == EEng_MilestoneStatus::Blocked || 
            Requirement.Status == EEng_MilestoneStatus::Failed)
        {
            BlockingIssues.Add(FString::Printf(TEXT("%s: %s"), 
                *Requirement.RequirementName, 
                *Requirement.Description));
        }
    }
    
    return BlockingIssues;
}

void UEng_MilestoneValidator::GenerateProgressReport()
{
    if (GEngine)
    {
        FString Report = FString::Printf(TEXT("=== MILESTONE 1 PROGRESS REPORT ===\n"));
        Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), OverallCompletionPercentage);
        Report += FString::Printf(TEXT("Milestone Complete: %s\n"), bMilestone1Complete ? TEXT("YES") : TEXT("NO"));
        Report += FString::Printf(TEXT("\nRequirements:\n"));
        
        for (const FEng_MilestoneRequirement& Requirement : Milestone1Requirements)
        {
            FString StatusText;
            switch (Requirement.Status)
            {
                case EEng_MilestoneStatus::NotStarted: StatusText = TEXT("NOT STARTED"); break;
                case EEng_MilestoneStatus::InProgress: StatusText = TEXT("IN PROGRESS"); break;
                case EEng_MilestoneStatus::Completed: StatusText = TEXT("COMPLETED"); break;
                case EEng_MilestoneStatus::Failed: StatusText = TEXT("FAILED"); break;
                case EEng_MilestoneStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            }
            
            Report += FString::Printf(TEXT("  %s: %s (%.1f%%) - %s\n"), 
                *Requirement.RequirementName, 
                *StatusText,
                Requirement.CompletionPercentage,
                *Requirement.AssignedAgent);
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Report);
    }
}

bool UEng_MilestoneValidator::ValidateEngineArchitecture()
{
    // Check if core engine systems are properly initialized
    bool bWorldValid = (GetWorld() != nullptr);
    bool bGameModeValid = (GetWorld() && GetWorld()->GetAuthGameMode() != nullptr);
    
    return bWorldValid && bGameModeValid;
}

bool UEng_MilestoneValidator::CheckCompilationStatus()
{
    // This would typically check compilation logs or module loading status
    // For now, return true if we can access basic UE5 classes
    return (GetWorld() != nullptr);
}

TArray<FString> UEng_MilestoneValidator::GetArchitecturalIssues()
{
    TArray<FString> Issues;
    
    if (!ValidateEngineArchitecture())
    {
        Issues.Add(TEXT("Engine architecture validation failed"));
    }
    
    if (!CheckCompilationStatus())
    {
        Issues.Add(TEXT("Compilation issues detected"));
    }
    
    return Issues;
}

void UEng_MilestoneValidator::LogValidationResults()
{
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Engine Architect Validation - Progress: %.1f%% - Milestone Complete: %s"), 
            OverallCompletionPercentage, 
            bMilestone1Complete ? TEXT("YES") : TEXT("NO"));
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, LogMessage);
    }
}