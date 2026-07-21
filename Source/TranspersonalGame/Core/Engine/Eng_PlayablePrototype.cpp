#include "Eng_PlayablePrototype.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UEng_PlayablePrototype::UEng_PlayablePrototype()
{
    PrototypeState = EEng_PrototypeState::NotStarted;
    LastValidationTime = 0.0f;
    bAutoValidateOnTick = false;
    
    InitializeRequirements();
}

void UEng_PlayablePrototype::InitializeRequirements()
{
    RequirementsList.Empty();

    FEng_PrototypeRequirement CharacterReq;
    CharacterReq.RequirementName = TEXT("Third Person Character");
    CharacterReq.Description = TEXT("ACharacter subclass with WASD movement");
    CharacterReq.Priority = 10.0f;
    CharacterReq.bIsCompleted = false;
    RequirementsList.Add(CharacterReq);

    FEng_PrototypeRequirement CameraReq;
    CameraReq.RequirementName = TEXT("Camera System");
    CameraReq.Description = TEXT("Camera boom + follow camera setup");
    CameraReq.Priority = 9.0f;
    CameraReq.bIsCompleted = false;
    RequirementsList.Add(CameraReq);

    FEng_PrototypeRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("Landscape Terrain");
    TerrainReq.Description = TEXT("Basic terrain with height variation");
    TerrainReq.Priority = 8.0f;
    TerrainReq.bIsCompleted = false;
    RequirementsList.Add(TerrainReq);

    FEng_PrototypeRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("Dinosaur Meshes");
    DinosaurReq.Description = TEXT("3-5 static dinosaur meshes in world");
    DinosaurReq.Priority = 7.0f;
    DinosaurReq.bIsCompleted = false;
    RequirementsList.Add(DinosaurReq);

    FEng_PrototypeRequirement LightingReq;
    LightingReq.RequirementName = TEXT("Lighting System");
    LightingReq.Description = TEXT("Directional light + sky atmosphere");
    LightingReq.Priority = 6.0f;
    LightingReq.bIsCompleted = false;
    RequirementsList.Add(LightingReq);
}

bool UEng_PlayablePrototype::ValidateWalkAroundMilestone()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_PlayablePrototype: Starting WALK AROUND milestone validation"));

    // Reset milestone status
    CurrentMilestone = FEng_WalkAroundMilestone();

    // Validate each component
    CurrentMilestone.bHasThirdPersonCharacter = ValidateThirdPersonCharacter();
    CurrentMilestone.bHasWASDMovement = ValidateCharacterMovement();
    CurrentMilestone.bHasCameraBoom = ValidateCameraSystem();
    CurrentMilestone.bHasLandscape = ValidateLandscapeTerrain();
    CurrentMilestone.bHasDinosaurMeshes = ValidateDinosaurMeshes();
    CurrentMilestone.bHasLighting = ValidateLightingSystem();

    // Update requirements based on validation
    UpdateMilestoneFromValidation();

    // Calculate completion
    bool bMilestoneComplete = CurrentMilestone.bHasThirdPersonCharacter &&
                             CurrentMilestone.bHasWASDMovement &&
                             CurrentMilestone.bHasCameraBoom &&
                             CurrentMilestone.bHasLandscape &&
                             CurrentMilestone.bHasDinosaurMeshes &&
                             CurrentMilestone.bHasLighting;

    PrototypeState = bMilestoneComplete ? EEng_PrototypeState::Completed : EEng_PrototypeState::InProgress;
    LastValidationTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Warning, TEXT("UEng_PlayablePrototype: Milestone validation complete. Status: %s"), 
           bMilestoneComplete ? TEXT("COMPLETED") : TEXT("IN PROGRESS"));

    return bMilestoneComplete;
}

bool UEng_PlayablePrototype::ValidateThirdPersonCharacter()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        LogValidationResults(TEXT("Character"), false);
        return false;
    }

    // Look for ACharacter actors in the world
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character && !Character->IsPendingKill())
        {
            UE_LOG(LogTemp, Warning, TEXT("Found Character: %s"), *Character->GetName());
            LogValidationResults(TEXT("Character"), true);
            return true;
        }
    }

    LogValidationResults(TEXT("Character"), false);
    return false;
}

bool UEng_PlayablePrototype::ValidateCharacterMovement()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character && Character->GetCharacterMovement())
        {
            UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
            if (MovementComp->GetMaxSpeed() > 0.0f)
            {
                LogValidationResults(TEXT("Movement"), true);
                return true;
            }
        }
    }

    LogValidationResults(TEXT("Movement"), false);
    return false;
}

bool UEng_PlayablePrototype::ValidateCameraSystem()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character)
        {
            // Check for SpringArm component
            USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
            UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
            
            if (SpringArm && Camera)
            {
                LogValidationResults(TEXT("Camera"), true);
                return true;
            }
        }
    }

    LogValidationResults(TEXT("Camera"), false);
    return false;
}

bool UEng_PlayablePrototype::ValidateLandscapeTerrain()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    // Check for Landscape actors
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && !Landscape->IsPendingKill())
        {
            // Check if landscape has some height variation
            FVector LandscapeScale = Landscape->GetActorScale3D();
            if (LandscapeScale.Z > 1.0f)
            {
                CurrentMilestone.TerrainVariation = LandscapeScale.Z;
                LogValidationResults(TEXT("Landscape"), true);
                return true;
            }
        }
    }

    LogValidationResults(TEXT("Landscape"), false);
    return false;
}

bool UEng_PlayablePrototype::ValidateDinosaurMeshes()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    int32 DinosaurCount = 0;

    // Look for StaticMeshActors that could be dinosaurs
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (MeshActor && !MeshActor->IsPendingKill())
        {
            FString ActorName = MeshActor->GetName();
            // Check if actor name contains dinosaur-related keywords
            if (ActorName.Contains(TEXT("Dinosaur")) || 
                ActorName.Contains(TEXT("TRex")) || 
                ActorName.Contains(TEXT("Raptor")) || 
                ActorName.Contains(TEXT("Brach")))
            {
                DinosaurCount++;
            }
        }
    }

    CurrentMilestone.DinosaurCount = DinosaurCount;
    bool bHasEnoughDinosaurs = DinosaurCount >= 3;
    
    LogValidationResults(TEXT("Dinosaurs"), bHasEnoughDinosaurs);
    return bHasEnoughDinosaurs;
}

bool UEng_PlayablePrototype::ValidateLightingSystem()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    bool bHasDirectionalLight = false;
    bool bHasSkyLight = false;

    // Check for DirectionalLight
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirLight = *ActorItr;
        if (DirLight && !DirLight->IsPendingKill())
        {
            bHasDirectionalLight = true;
            break;
        }
    }

    // Check for SkyLight
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ASkyLight* SkyLight = *ActorItr;
        if (SkyLight && !SkyLight->IsPendingKill())
        {
            bHasSkyLight = true;
            break;
        }
    }

    bool bLightingValid = bHasDirectionalLight || bHasSkyLight;
    LogValidationResults(TEXT("Lighting"), bLightingValid);
    return bLightingValid;
}

void UEng_PlayablePrototype::UpdateMilestoneFromValidation()
{
    // Update requirements based on milestone status
    for (FEng_PrototypeRequirement& Req : RequirementsList)
    {
        if (Req.RequirementName == TEXT("Third Person Character"))
        {
            Req.bIsCompleted = CurrentMilestone.bHasThirdPersonCharacter;
        }
        else if (Req.RequirementName == TEXT("Camera System"))
        {
            Req.bIsCompleted = CurrentMilestone.bHasCameraBoom;
        }
        else if (Req.RequirementName == TEXT("Landscape Terrain"))
        {
            Req.bIsCompleted = CurrentMilestone.bHasLandscape;
        }
        else if (Req.RequirementName == TEXT("Dinosaur Meshes"))
        {
            Req.bIsCompleted = CurrentMilestone.bHasDinosaurMeshes;
        }
        else if (Req.RequirementName == TEXT("Lighting System"))
        {
            Req.bIsCompleted = CurrentMilestone.bHasLighting;
        }
    }
}

FEng_WalkAroundMilestone UEng_PlayablePrototype::GetCurrentMilestoneStatus()
{
    return CurrentMilestone;
}

TArray<FEng_PrototypeRequirement> UEng_PlayablePrototype::GetMissingRequirements()
{
    TArray<FEng_PrototypeRequirement> MissingReqs;
    
    for (const FEng_PrototypeRequirement& Req : RequirementsList)
    {
        if (!Req.bIsCompleted)
        {
            MissingReqs.Add(Req);
        }
    }
    
    return MissingReqs;
}

float UEng_PlayablePrototype::GetMilestoneCompletionPercentage()
{
    if (RequirementsList.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = 0;
    for (const FEng_PrototypeRequirement& Req : RequirementsList)
    {
        if (Req.bIsCompleted)
        {
            CompletedCount++;
        }
    }

    return (float)CompletedCount / (float)RequirementsList.Num() * 100.0f;
}

void UEng_PlayablePrototype::GeneratePrototypeReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== WALK AROUND MILESTONE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), GetMilestoneCompletionPercentage());
    UE_LOG(LogTemp, Warning, TEXT("State: %d"), (int32)PrototypeState);
    
    UE_LOG(LogTemp, Warning, TEXT("Components Status:"));
    UE_LOG(LogTemp, Warning, TEXT("- Character: %s"), CurrentMilestone.bHasThirdPersonCharacter ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Movement: %s"), CurrentMilestone.bHasWASDMovement ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Camera: %s"), CurrentMilestone.bHasCameraBoom ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Landscape: %s"), CurrentMilestone.bHasLandscape ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaurs: %s (%d found)"), CurrentMilestone.bHasDinosaurMeshes ? TEXT("PASS") : TEXT("FAIL"), CurrentMilestone.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("- Lighting: %s"), CurrentMilestone.bHasLighting ? TEXT("PASS") : TEXT("FAIL"));
    
    TArray<FEng_PrototypeRequirement> Missing = GetMissingRequirements();
    if (Missing.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing Requirements:"));
        for (const FEng_PrototypeRequirement& Req : Missing)
        {
            UE_LOG(LogTemp, Warning, TEXT("- %s: %s"), *Req.RequirementName, *Req.Description);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UEng_PlayablePrototype::LogMilestoneStatus()
{
    ValidateWalkAroundMilestone();
    GeneratePrototypeReport();
}

void UEng_PlayablePrototype::LogValidationResults(const FString& ComponentName, bool bIsValid)
{
    UE_LOG(LogTemp, Warning, TEXT("Validation [%s]: %s"), *ComponentName, bIsValid ? TEXT("PASS") : TEXT("FAIL"));
}