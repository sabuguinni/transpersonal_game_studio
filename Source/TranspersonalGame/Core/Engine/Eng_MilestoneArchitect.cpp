#include "Eng_MilestoneArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UEng_MilestoneArchitect::UEng_MilestoneArchitect()
{
    CurrentActiveMilestone = EEng_MilestoneType::WalkAround;
    WalkAroundProgress = 0.0f;
    bTerrainValid = false;
    bCharacterValid = false;
    bDinosaursValid = false;
    bLightingValid = false;
}

void UEng_MilestoneArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Initializing milestone validation system"));
    
    InitializeMilestones();
    
    // Start validation timer
    if (UWorld* World = GetWorld())
    {
        FTimerHandle ValidationTimer;
        World->GetTimerManager().SetTimer(ValidationTimer, this, &UEng_MilestoneArchitect::LogMilestoneStatus, 5.0f, true);
    }
}

void UEng_MilestoneArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Shutting down milestone validation system"));
    Super::Deinitialize();
}

void UEng_MilestoneArchitect::InitializeMilestones()
{
    Milestones.Empty();
    
    SetupWalkAroundMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Initialized %d milestones"), Milestones.Num());
}

void UEng_MilestoneArchitect::SetupWalkAroundMilestone()
{
    FEng_MilestoneData WalkAroundMilestone;
    WalkAroundMilestone.MilestoneType = EEng_MilestoneType::WalkAround;
    WalkAroundMilestone.MilestoneName = TEXT("Walk Around Prototype");
    WalkAroundMilestone.bIsActive = true;
    WalkAroundMilestone.Priority = 1;
    
    // Terrain requirement
    FEng_MilestoneRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("Varied Terrain");
    TerrainReq.Description = TEXT("Landscape with hills, valleys, and height variation");
    TerrainReq.bIsCompleted = false;
    TerrainReq.CompletionPercentage = 0.0f;
    WalkAroundMilestone.Requirements.Add(TerrainReq);
    
    // Character movement requirement
    FEng_MilestoneRequirement CharacterReq;
    CharacterReq.RequirementName = TEXT("Character Movement");
    CharacterReq.Description = TEXT("Player character with WASD movement, camera, jump");
    CharacterReq.bIsCompleted = false;
    CharacterReq.CompletionPercentage = 0.0f;
    WalkAroundMilestone.Requirements.Add(CharacterReq);
    
    // Dinosaur placement requirement
    FEng_MilestoneRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("Dinosaur Actors");
    DinosaurReq.Description = TEXT("3-5 static dinosaur meshes placed in the world");
    DinosaurReq.bIsCompleted = false;
    DinosaurReq.CompletionPercentage = 0.0f;
    WalkAroundMilestone.Requirements.Add(DinosaurReq);
    
    // Lighting requirement
    FEng_MilestoneRequirement LightingReq;
    LightingReq.RequirementName = TEXT("Basic Lighting");
    LightingReq.Description = TEXT("Directional light, sky atmosphere, fog");
    LightingReq.bIsCompleted = false;
    LightingReq.CompletionPercentage = 0.0f;
    WalkAroundMilestone.Requirements.Add(LightingReq);
    
    Milestones.Add(WalkAroundMilestone);
}

bool UEng_MilestoneArchitect::ValidateWalkAroundMilestone()
{
    bool bTerrainOK = ValidateTerrainRequirements();
    bool bCharacterOK = ValidateCharacterRequirements();
    bool bDinosaurOK = ValidateDinosaurRequirements();
    bool bLightingOK = ValidateLightingRequirements();
    
    // Update milestone progress
    float Progress = 0.0f;
    if (bTerrainOK) Progress += 25.0f;
    if (bCharacterOK) Progress += 25.0f;
    if (bDinosaurOK) Progress += 25.0f;
    if (bLightingOK) Progress += 25.0f;
    
    UpdateMilestoneProgress(EEng_MilestoneType::WalkAround, Progress);
    
    return (Progress >= 100.0f);
}

bool UEng_MilestoneArchitect::ValidateTerrainRequirements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bTerrainValid = false;
        return false;
    }
    
    // Check for landscape actors
    bool bHasLandscape = false;
    bool bHasVariation = false;
    
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && IsValid(Landscape))
        {
            bHasLandscape = true;
            
            // Check for height variation by examining landscape bounds
            FBox LandscapeBounds = Landscape->GetComponentsBoundingBox();
            float HeightVariation = LandscapeBounds.Max.Z - LandscapeBounds.Min.Z;
            
            if (HeightVariation > 100.0f) // At least 1 meter of height variation
            {
                bHasVariation = true;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Found landscape with height variation: %.2f"), HeightVariation);
            break;
        }
    }
    
    bTerrainValid = bHasLandscape && bHasVariation;
    
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Terrain validation - Landscape: %s, Variation: %s"), 
           bHasLandscape ? TEXT("YES") : TEXT("NO"),
           bHasVariation ? TEXT("YES") : TEXT("NO"));
    
    return bTerrainValid;
}

bool UEng_MilestoneArchitect::ValidateCharacterRequirements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bCharacterValid = false;
        return false;
    }
    
    // Check for player character
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        bCharacterValid = false;
        UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: No PlayerController found"));
        return false;
    }
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        bCharacterValid = false;
        UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: No PlayerPawn found"));
        return false;
    }
    
    // Check if it's a Character (has movement component)
    ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
    if (!PlayerCharacter)
    {
        bCharacterValid = false;
        UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: PlayerPawn is not a Character"));
        return false;
    }
    
    // Check movement component
    if (!PlayerCharacter->GetCharacterMovement())
    {
        bCharacterValid = false;
        UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Character has no movement component"));
        return false;
    }
    
    bCharacterValid = true;
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Character validation PASSED - Found valid player character"));
    
    return bCharacterValid;
}

bool UEng_MilestoneArchitect::ValidateDinosaurRequirements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bDinosaursValid = false;
        return false;
    }
    
    int32 DinosaurCount = 0;
    
    // Count actors with "Dinosaur" or "TRex" or "Raptor" or "Brach" in name
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Dinosaur")) || 
                ActorName.Contains(TEXT("TRex")) || 
                ActorName.Contains(TEXT("Raptor")) || 
                ActorName.Contains(TEXT("Brach")) ||
                ActorName.Contains(TEXT("Dino")))
            {
                DinosaurCount++;
                UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Found dinosaur actor: %s"), *ActorName);
            }
        }
    }
    
    bDinosaursValid = (DinosaurCount >= 3);
    
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Dinosaur validation - Found %d dinosaurs (need 3+): %s"), 
           DinosaurCount, bDinosaursValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bDinosaursValid;
}

bool UEng_MilestoneArchitect::ValidateLightingRequirements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        bLightingValid = false;
        return false;
    }
    
    bool bHasDirectionalLight = false;
    bool bHasSkyLight = false;
    bool bHasFog = false;
    
    // Check for directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirLight = *ActorItr;
        if (DirLight && IsValid(DirLight))
        {
            bHasDirectionalLight = true;
            UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Found directional light"));
            break;
        }
    }
    
    // Check for sky light
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            if (Actor->FindComponentByClass<USkyLightComponent>())
            {
                bHasSkyLight = true;
                UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Found sky light"));
                break;
            }
        }
    }
    
    // Check for fog
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
            {
                bHasFog = true;
                UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Found exponential height fog"));
                break;
            }
        }
    }
    
    bLightingValid = bHasDirectionalLight && (bHasSkyLight || bHasFog);
    
    UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Lighting validation - DirLight: %s, SkyLight: %s, Fog: %s"), 
           bHasDirectionalLight ? TEXT("YES") : TEXT("NO"),
           bHasSkyLight ? TEXT("YES") : TEXT("NO"),
           bHasFog ? TEXT("YES") : TEXT("NO"));
    
    return bLightingValid;
}

void UEng_MilestoneArchitect::UpdateMilestoneProgress(EEng_MilestoneType MilestoneType, float Progress)
{
    for (FEng_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneType == MilestoneType)
        {
            Milestone.OverallProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            if (MilestoneType == EEng_MilestoneType::WalkAround)
            {
                WalkAroundProgress = Milestone.OverallProgress;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("MilestoneArchitect: Updated %s progress to %.1f%%"), 
                   *Milestone.MilestoneName, Milestone.OverallProgress);
            break;
        }
    }
}

float UEng_MilestoneArchitect::GetMilestoneProgress(EEng_MilestoneType MilestoneType)
{
    for (const FEng_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneType == MilestoneType)
        {
            return Milestone.OverallProgress;
        }
    }
    return 0.0f;
}

TArray<FEng_MilestoneData> UEng_MilestoneArchitect::GetAllMilestones()
{
    return Milestones;
}

FEng_MilestoneData UEng_MilestoneArchitect::GetCurrentMilestone()
{
    for (const FEng_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneType == CurrentActiveMilestone)
        {
            return Milestone;
        }
    }
    
    // Return default if not found
    FEng_MilestoneData DefaultMilestone;
    return DefaultMilestone;
}

bool UEng_MilestoneArchitect::CheckPlayerCanMove()
{
    return ValidateCharacterRequirements();
}

bool UEng_MilestoneArchitect::CheckTerrainHasVariation()
{
    return ValidateTerrainRequirements();
}

bool UEng_MilestoneArchitect::CheckDinosaursExist()
{
    return ValidateDinosaurRequirements();
}

bool UEng_MilestoneArchitect::CheckLightingSetup()
{
    return ValidateLightingRequirements();
}

FString UEng_MilestoneArchitect::GenerateMilestoneReport()
{
    FString Report = TEXT("=== MILESTONE VALIDATION REPORT ===\n\n");
    
    for (const FEng_MilestoneData& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("MILESTONE: %s\n"), *Milestone.MilestoneName);
        Report += FString::Printf(TEXT("Progress: %.1f%%\n"), Milestone.OverallProgress);
        Report += FString::Printf(TEXT("Active: %s\n"), Milestone.bIsActive ? TEXT("YES") : TEXT("NO"));
        Report += TEXT("Requirements:\n");
        
        for (const FEng_MilestoneRequirement& Req : Milestone.Requirements)
        {
            Report += FString::Printf(TEXT("  - %s: %s (%.1f%%)\n"), 
                                    *Req.RequirementName, 
                                    Req.bIsCompleted ? TEXT("COMPLETE") : TEXT("PENDING"),
                                    Req.CompletionPercentage);
        }
        Report += TEXT("\n");
    }
    
    Report += TEXT("=== VALIDATION STATUS ===\n");
    Report += FString::Printf(TEXT("Terrain: %s\n"), bTerrainValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Character: %s\n"), bCharacterValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Dinosaurs: %s\n"), bDinosaursValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Lighting: %s\n"), bLightingValid ? TEXT("VALID") : TEXT("INVALID"));
    
    return Report;
}

void UEng_MilestoneArchitect::LogMilestoneStatus()
{
    ValidateWalkAroundMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Walk Around Progress: %.1f%%"), WalkAroundProgress);
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %s | Character: %s | Dinosaurs: %s | Lighting: %s"), 
           bTerrainValid ? TEXT("OK") : TEXT("FAIL"),
           bCharacterValid ? TEXT("OK") : TEXT("FAIL"),
           bDinosaursValid ? TEXT("OK") : TEXT("FAIL"),
           bLightingValid ? TEXT("OK") : TEXT("FAIL"));
}