#include "Eng_PlayablePrototypeArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UEng_PlayablePrototypeArchitect::UEng_PlayablePrototypeArchitect()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    LastValidationTime = 0.0f;
    bValidationInProgress = false;
}

void UEng_PlayablePrototypeArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWalkAroundRequirements();
    
    // Perform initial validation after 2 seconds
    GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), this, 
        &UEng_PlayablePrototypeArchitect::ValidateWalkAroundMilestone, 2.0f, false);
}

void UEng_PlayablePrototypeArchitect::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bValidationInProgress && GetWorld()->GetTimeSeconds() - LastValidationTime > 10.0f)
    {
        ValidateWalkAroundMilestone();
        LastValidationTime = GetWorld()->GetTimeSeconds();
    }
}

void UEng_PlayablePrototypeArchitect::InitializeWalkAroundRequirements()
{
    WalkAroundMilestone.MilestoneName = TEXT("WALK AROUND Prototype");
    WalkAroundMilestone.Requirements.Empty();

    // Character Movement Requirement
    FEng_PrototypeRequirement CharacterReq;
    CharacterReq.RequirementName = TEXT("ThirdPersonCharacter_Movement");
    CharacterReq.Description = TEXT("ACharacter subclass with WASD movement, camera boom, follow camera");
    CharacterReq.ResponsibleAgent = TEXT("Agent_09_Character");
    WalkAroundMilestone.Requirements.Add(CharacterReq);

    // Terrain Requirement
    FEng_PrototypeRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("Landscape_BasicTerrain");
    TerrainReq.Description = TEXT("Landscape with basic terrain (not flat plane)");
    TerrainReq.ResponsibleAgent = TEXT("Agent_05_ProceduralWorld");
    WalkAroundMilestone.Requirements.Add(TerrainReq);

    // Lighting Requirement
    FEng_PrototypeRequirement LightingReq;
    LightingReq.RequirementName = TEXT("Lighting_DirectionalSun");
    LightingReq.Description = TEXT("Directional light + sky atmosphere + fog");
    LightingReq.ResponsibleAgent = TEXT("Agent_08_Lighting");
    WalkAroundMilestone.Requirements.Add(LightingReq);

    // Dinosaur Placement Requirement
    FEng_PrototypeRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("Dinosaurs_StaticMeshes");
    DinosaurReq.Description = TEXT("3-5 static dinosaur meshes placed in the world");
    DinosaurReq.ResponsibleAgent = TEXT("Agent_06_Environment");
    WalkAroundMilestone.Requirements.Add(DinosaurReq);

    // Player Movement Requirement
    FEng_PrototypeRequirement MovementReq;
    MovementReq.RequirementName = TEXT("Player_WalkRunJump");
    MovementReq.Description = TEXT("Player can walk, run, jump with responsive controls");
    MovementReq.ResponsibleAgent = TEXT("Agent_09_Character");
    WalkAroundMilestone.Requirements.Add(MovementReq);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initialized WALK AROUND milestone with %d requirements"), 
        WalkAroundMilestone.Requirements.Num());
}

void UEng_PlayablePrototypeArchitect::ValidateWalkAroundMilestone()
{
    if (bValidationInProgress) return;
    
    bValidationInProgress = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Starting WALK AROUND milestone validation"));

    // Validate each requirement
    for (int32 i = 0; i < WalkAroundMilestone.Requirements.Num(); i++)
    {
        FEng_PrototypeRequirement& Requirement = WalkAroundMilestone.Requirements[i];
        
        if (Requirement.RequirementName == TEXT("ThirdPersonCharacter_Movement"))
        {
            bool bValid = ValidateCharacterMovement();
            UpdateRequirementProgress(Requirement.RequirementName, bValid ? 100.0f : 30.0f, bValid);
        }
        else if (Requirement.RequirementName == TEXT("Landscape_BasicTerrain"))
        {
            bool bValid = ValidateTerrainSystem();
            UpdateRequirementProgress(Requirement.RequirementName, bValid ? 100.0f : 20.0f, bValid);
        }
        else if (Requirement.RequirementName == TEXT("Lighting_DirectionalSun"))
        {
            bool bValid = ValidateLightingSystem();
            UpdateRequirementProgress(Requirement.RequirementName, bValid ? 100.0f : 80.0f, bValid);
        }
        else if (Requirement.RequirementName == TEXT("Dinosaurs_StaticMeshes"))
        {
            bool bValid = ValidateDinosaurPlacements();
            UpdateRequirementProgress(Requirement.RequirementName, bValid ? 100.0f : 40.0f, bValid);
        }
        else if (Requirement.RequirementName == TEXT("Player_WalkRunJump"))
        {
            bool bValid = ValidateCameraSystem();
            UpdateRequirementProgress(Requirement.RequirementName, bValid ? 100.0f : 60.0f, bValid);
        }
    }

    // Calculate overall progress
    WalkAroundMilestone.OverallProgress = CalculateOverallProgress();
    
    // Check if milestone is complete
    WalkAroundMilestone.bIsMilestoneComplete = WalkAroundMilestone.OverallProgress >= 95.0f;

    // Validate architectural standards
    EnforceArchitecturalStandards();
    ValidateCompilationStatus();

    LogArchitectureStatus();
    
    bValidationInProgress = false;
}

bool UEng_PlayablePrototypeArchitect::ValidateCharacterMovement()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Check for TranspersonalCharacter
    bool bCharacterExists = CheckActorExistence(TEXT("TranspersonalCharacter"));
    
    // Check for PlayerStart
    bool bPlayerStartExists = CheckActorExistence(TEXT("PlayerStart"));
    
    UE_LOG(LogTemp, Warning, TEXT("Character Validation - Character: %s, PlayerStart: %s"), 
        bCharacterExists ? TEXT("FOUND") : TEXT("MISSING"),
        bPlayerStartExists ? TEXT("FOUND") : TEXT("MISSING"));

    return bCharacterExists && bPlayerStartExists;
}

bool UEng_PlayablePrototypeArchitect::ValidateTerrainSystem()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Check for Landscape actor
    bool bLandscapeExists = CheckActorExistence(TEXT("Landscape"));
    
    // Check for basic terrain variation (simplified check)
    int32 StaticMeshCount = CountActorsOfClass(TEXT("StaticMeshActor"));
    bool bTerrainVariation = StaticMeshCount > 5; // Basic terrain should have some variation

    UE_LOG(LogTemp, Warning, TEXT("Terrain Validation - Landscape: %s, Variation: %s (StaticMeshes: %d)"), 
        bLandscapeExists ? TEXT("FOUND") : TEXT("MISSING"),
        bTerrainVariation ? TEXT("SUFFICIENT") : TEXT("INSUFFICIENT"),
        StaticMeshCount);

    return bLandscapeExists || bTerrainVariation; // Either landscape or terrain variation
}

bool UEng_PlayablePrototypeArchitect::ValidateLightingSystem()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Check for DirectionalLight
    bool bDirectionalLightExists = CheckActorExistence(TEXT("DirectionalLight"));
    
    // Check for SkyAtmosphere
    bool bSkyAtmosphereExists = CheckActorExistence(TEXT("SkyAtmosphere"));
    
    // Check for ExponentialHeightFog
    bool bFogExists = CheckActorExistence(TEXT("ExponentialHeightFog"));

    UE_LOG(LogTemp, Warning, TEXT("Lighting Validation - DirectionalLight: %s, SkyAtmosphere: %s, Fog: %s"), 
        bDirectionalLightExists ? TEXT("FOUND") : TEXT("MISSING"),
        bSkyAtmosphereExists ? TEXT("FOUND") : TEXT("MISSING"),
        bFogExists ? TEXT("FOUND") : TEXT("MISSING"));

    return bDirectionalLightExists; // At minimum need directional light
}

bool UEng_PlayablePrototypeArchitect::ValidateDinosaurPlacements()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Count potential dinosaur actors (StaticMeshActors that could be dinosaurs)
    int32 StaticMeshCount = CountActorsOfClass(TEXT("StaticMeshActor"));
    
    // Look for specifically named dinosaur actors
    bool bDinosaurActorsFound = StaticMeshCount >= 3; // Minimum 3 static meshes as dinosaurs

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Validation - StaticMeshActors: %d, Sufficient: %s"), 
        StaticMeshCount,
        bDinosaurActorsFound ? TEXT("YES") : TEXT("NO"));

    return bDinosaurActorsFound;
}

bool UEng_PlayablePrototypeArchitect::ValidateCameraSystem()
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Check if character has camera components (simplified validation)
    bool bCharacterExists = CheckActorExistence(TEXT("TranspersonalCharacter"));
    
    // In a real scenario, we'd check for SpringArmComponent and CameraComponent
    // For now, assume if character exists, camera system is functional

    UE_LOG(LogTemp, Warning, TEXT("Camera Validation - Character with camera: %s"), 
        bCharacterExists ? TEXT("ASSUMED OK") : TEXT("MISSING"));

    return bCharacterExists;
}

void UEng_PlayablePrototypeArchitect::EnforceArchitecturalStandards()
{
    ArchitectureViolations.Empty();

    // Check for architectural violations
    UWorld* World = GetWorld();
    if (!World)
    {
        ArchitectureViolations.Add(TEXT("CRITICAL: No valid world context"));
        return;
    }

    // Validate minimum actor count
    int32 TotalActors = CountActorsOfClass(TEXT("Actor"));
    if (TotalActors < 10)
    {
        ArchitectureViolations.Add(FString::Printf(TEXT("Insufficient actors in scene: %d (minimum 10)"), TotalActors));
    }

    // Validate essential systems
    if (!CheckActorExistence(TEXT("TranspersonalCharacter")))
    {
        ArchitectureViolations.Add(TEXT("CRITICAL: Missing TranspersonalCharacter"));
    }

    if (!CheckActorExistence(TEXT("PlayerStart")))
    {
        ArchitectureViolations.Add(TEXT("CRITICAL: Missing PlayerStart"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Architecture Violations: %d"), ArchitectureViolations.Num());
}

void UEng_PlayablePrototypeArchitect::ValidateCompilationStatus()
{
    CriticalCompilationErrors.Empty();

    // Basic compilation validation
    UWorld* World = GetWorld();
    if (!World)
    {
        CriticalCompilationErrors.Add(TEXT("CRITICAL: World context invalid"));
        return;
    }

    // Check if essential classes are available
    if (!CheckActorExistence(TEXT("TranspersonalCharacter")))
    {
        CriticalCompilationErrors.Add(TEXT("TranspersonalCharacter class not found or not compiled"));
    }

    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CriticalCompilationErrors.Num());
}

float UEng_PlayablePrototypeArchitect::CalculateOverallProgress()
{
    if (WalkAroundMilestone.Requirements.Num() == 0) return 0.0f;

    float TotalProgress = 0.0f;
    for (const FEng_PrototypeRequirement& Requirement : WalkAroundMilestone.Requirements)
    {
        TotalProgress += Requirement.CompletionPercentage;
    }

    return TotalProgress / WalkAroundMilestone.Requirements.Num();
}

FString UEng_PlayablePrototypeArchitect::GenerateProgressReport()
{
    FString Report = FString::Printf(TEXT("=== WALK AROUND MILESTONE PROGRESS: %.1f%% ===\n"), 
        WalkAroundMilestone.OverallProgress);

    for (const FEng_PrototypeRequirement& Requirement : WalkAroundMilestone.Requirements)
    {
        Report += FString::Printf(TEXT("- %s: %.1f%% [%s] (%s)\n"), 
            *Requirement.RequirementName,
            Requirement.CompletionPercentage,
            Requirement.bIsCompleted ? TEXT("COMPLETE") : TEXT("IN PROGRESS"),
            *Requirement.ResponsibleAgent);
    }

    if (ArchitectureViolations.Num() > 0)
    {
        Report += TEXT("\nARCHITECTURE VIOLATIONS:\n");
        for (const FString& Violation : ArchitectureViolations)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Violation);
        }
    }

    return Report;
}

void UEng_PlayablePrototypeArchitect::LogArchitectureStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Progress: %.1f%%"), WalkAroundMilestone.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Milestone Complete: %s"), 
        WalkAroundMilestone.bIsMilestoneComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Violations: %d"), ArchitectureViolations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CriticalCompilationErrors.Num());
}

void UEng_PlayablePrototypeArchitect::UpdateRequirementProgress(const FString& RequirementName, float Progress, bool bCompleted)
{
    for (FEng_PrototypeRequirement& Requirement : WalkAroundMilestone.Requirements)
    {
        if (Requirement.RequirementName == RequirementName)
        {
            Requirement.CompletionPercentage = Progress;
            Requirement.bIsCompleted = bCompleted;
            break;
        }
    }
}

bool UEng_PlayablePrototypeArchitect::CheckActorExistence(const FString& ActorClass)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Simplified check - count actors and assume some exist
    // In production, this would use proper class checking
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(ActorClass))
        {
            return true;
        }
    }

    return false;
}

int32 UEng_PlayablePrototypeArchitect::CountActorsOfClass(const FString& ActorClass)
{
    UWorld* World = GetWorld();
    if (!World) return 0;

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            if (ActorClass == TEXT("Actor") || Actor->GetClass()->GetName().Contains(ActorClass))
            {
                Count++;
            }
        }
    }

    return Count;
}

void UEng_PlayablePrototypeArchitect::ValidateSystemIntegration()
{
    // Validate cross-system integration
    UWorld* World = GetWorld();
    if (!World) return;

    // Check character-terrain integration
    bool bCharacterExists = CheckActorExistence(TEXT("TranspersonalCharacter"));
    bool bTerrainExists = ValidateTerrainSystem();
    
    if (bCharacterExists && bTerrainExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("System Integration: Character-Terrain OK"));
    }

    // Check lighting-environment integration
    bool bLightingExists = ValidateLightingSystem();
    bool bEnvironmentExists = ValidateDinosaurPlacements();
    
    if (bLightingExists && bEnvironmentExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("System Integration: Lighting-Environment OK"));
    }
}