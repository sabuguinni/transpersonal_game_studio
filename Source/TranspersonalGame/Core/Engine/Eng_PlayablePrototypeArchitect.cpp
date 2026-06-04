#include "Eng_PlayablePrototypeArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PlayerStart.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"

UEng_PlayablePrototypeArchitect::UEng_PlayablePrototypeArchitect()
{
    bPrototypeValidated = false;
    LastValidationResult = TEXT("Not validated");
    LastValidationTime = 0.0f;
    
    InitializePrototypeRequirements();
}

void UEng_PlayablePrototypeArchitect::InitializePrototypeRequirements()
{
    PrototypeRequirements.Empty();

    // Third Person Character
    FEng_PrototypeRequirement CharacterReq;
    CharacterReq.Component = EEng_PrototypeComponent::ThirdPersonCharacter;
    CharacterReq.bIsRequired = true;
    CharacterReq.bIsImplemented = false;
    CharacterReq.Description = TEXT("ACharacter subclass with third-person setup");
    CharacterReq.Priority = 10.0f;
    PrototypeRequirements.Add(CharacterReq);

    // Camera Boom
    FEng_PrototypeRequirement CameraBoomReq;
    CameraBoomReq.Component = EEng_PrototypeComponent::CameraBoom;
    CameraBoomReq.bIsRequired = true;
    CameraBoomReq.bIsImplemented = false;
    CameraBoomReq.Description = TEXT("SpringArmComponent for camera positioning");
    CameraBoomReq.Priority = 9.0f;
    PrototypeRequirements.Add(CameraBoomReq);

    // Follow Camera
    FEng_PrototypeRequirement FollowCameraReq;
    FollowCameraReq.Component = EEng_PrototypeComponent::FollowCamera;
    FollowCameraReq.bIsRequired = true;
    FollowCameraReq.bIsImplemented = false;
    FollowCameraReq.Description = TEXT("CameraComponent attached to camera boom");
    FollowCameraReq.Priority = 9.0f;
    PrototypeRequirements.Add(FollowCameraReq);

    // Landscape
    FEng_PrototypeRequirement LandscapeReq;
    LandscapeReq.Component = EEng_PrototypeComponent::Landscape;
    LandscapeReq.bIsRequired = true;
    LandscapeReq.bIsImplemented = false;
    LandscapeReq.Description = TEXT("Landscape actor with height variation");
    LandscapeReq.Priority = 8.0f;
    PrototypeRequirements.Add(LandscapeReq);

    // Player Movement
    FEng_PrototypeRequirement MovementReq;
    MovementReq.Component = EEng_PrototypeComponent::PlayerMovement;
    MovementReq.bIsRequired = true;
    MovementReq.bIsImplemented = false;
    MovementReq.Description = TEXT("WASD movement, running, jumping");
    MovementReq.Priority = 10.0f;
    PrototypeRequirements.Add(MovementReq);

    // Dinosaur Meshes
    FEng_PrototypeRequirement DinosaurReq;
    DinosaurReq.Component = EEng_PrototypeComponent::DinosaurMeshes;
    DinosaurReq.bIsRequired = true;
    DinosaurReq.bIsImplemented = false;
    DinosaurReq.Description = TEXT("3-5 static dinosaur meshes placed in world");
    DinosaurReq.Priority = 6.0f;
    PrototypeRequirements.Add(DinosaurReq);

    // Directional Light
    FEng_PrototypeRequirement LightReq;
    LightReq.Component = EEng_PrototypeComponent::DirectionalLight;
    LightReq.bIsRequired = true;
    LightReq.bIsImplemented = false;
    LightReq.Description = TEXT("Directional light for sun illumination");
    LightReq.Priority = 7.0f;
    PrototypeRequirements.Add(LightReq);

    // Sky Atmosphere
    FEng_PrototypeRequirement SkyReq;
    SkyReq.Component = EEng_PrototypeComponent::SkyAtmosphere;
    SkyReq.bIsRequired = true;
    SkyReq.bIsImplemented = false;
    SkyReq.Description = TEXT("Sky atmosphere component for realistic sky");
    SkyReq.Priority = 5.0f;
    PrototypeRequirements.Add(SkyReq);

    // Exponential Height Fog
    FEng_PrototypeRequirement FogReq;
    FogReq.Component = EEng_PrototypeComponent::ExponentialHeightFog;
    FogReq.bIsRequired = true;
    FogReq.bIsImplemented = false;
    FogReq.Description = TEXT("Exponential height fog for atmosphere");
    FogReq.Priority = 4.0f;
    PrototypeRequirements.Add(FogReq);

    CurrentMilestone.Requirements = PrototypeRequirements;
    CurrentMilestone.TotalComponents = PrototypeRequirements.Num();
}

bool UEng_PlayablePrototypeArchitect::ValidatePlayablePrototype()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        LastValidationResult = TEXT("FAILED: No valid world context");
        return false;
    }

    bool bAllComponentsValid = true;
    int32 CompletedCount = 0;

    // Validate each component
    for (FEng_PrototypeRequirement& Requirement : PrototypeRequirements)
    {
        bool bComponentValid = false;

        switch (Requirement.Component)
        {
            case EEng_PrototypeComponent::ThirdPersonCharacter:
                bComponentValid = CheckThirdPersonCharacter();
                break;
            case EEng_PrototypeComponent::CameraBoom:
            case EEng_PrototypeComponent::FollowCamera:
                bComponentValid = CheckCameraSetup();
                break;
            case EEng_PrototypeComponent::Landscape:
                bComponentValid = CheckLandscapeTerrain();
                break;
            case EEng_PrototypeComponent::PlayerMovement:
                bComponentValid = CheckPlayerMovement();
                break;
            case EEng_PrototypeComponent::DinosaurMeshes:
                bComponentValid = CheckDinosaurMeshes();
                break;
            case EEng_PrototypeComponent::DirectionalLight:
            case EEng_PrototypeComponent::SkyAtmosphere:
            case EEng_PrototypeComponent::ExponentialHeightFog:
                bComponentValid = CheckLightingComponents();
                break;
            default:
                bComponentValid = false;
                break;
        }

        Requirement.bIsImplemented = bComponentValid;
        if (bComponentValid)
        {
            CompletedCount++;
        }
        else if (Requirement.bIsRequired)
        {
            bAllComponentsValid = false;
        }

        LogValidationResult(Requirement.Description, bComponentValid);
    }

    CurrentMilestone.CompletedComponents = CompletedCount;
    CurrentMilestone.CompletionPercentage = CalculateCompletionPercentage();
    CurrentMilestone.bMilestoneComplete = bAllComponentsValid;
    
    if (bAllComponentsValid)
    {
        CurrentMilestone.StatusMessage = TEXT("WALK AROUND milestone complete - playable prototype ready");
    }
    else
    {
        CurrentMilestone.StatusMessage = FString::Printf(TEXT("WALK AROUND milestone %d/%d components complete"), 
            CompletedCount, CurrentMilestone.TotalComponents);
    }

    bPrototypeValidated = bAllComponentsValid;
    LastValidationResult = CurrentMilestone.StatusMessage;
    LastValidationTime = FPlatformTime::Seconds();

    return bAllComponentsValid;
}

bool UEng_PlayablePrototypeArchitect::CheckThirdPersonCharacter()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return false;

    ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerCharacter) return false;

    // Check for required components
    if (!PlayerCharacter->GetCapsuleComponent()) return false;
    if (!PlayerCharacter->GetMesh()) return false;
    if (!PlayerCharacter->GetCharacterMovement()) return false;

    return true;
}

bool UEng_PlayablePrototypeArchitect::CheckCameraSetup()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return false;

    ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerCharacter) return false;

    // Look for SpringArm and Camera components
    USpringArmComponent* SpringArm = PlayerCharacter->FindComponentByClass<USpringArmComponent>();
    UCameraComponent* Camera = PlayerCharacter->FindComponentByClass<UCameraComponent>();

    return (SpringArm != nullptr && Camera != nullptr);
}

bool UEng_PlayablePrototypeArchitect::CheckLandscapeTerrain()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    // Look for landscape actors
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && !Landscape->IsPendingKill())
        {
            return true;
        }
    }

    return false;
}

bool UEng_PlayablePrototypeArchitect::CheckPlayerMovement()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return false;

    ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerCharacter) return false;

    UCharacterMovementComponent* MovementComp = PlayerCharacter->GetCharacterMovement();
    if (!MovementComp) return false;

    // Check movement capabilities
    return (MovementComp->GetMaxSpeed() > 0.0f && 
            MovementComp->JumpZVelocity > 0.0f);
}

bool UEng_PlayablePrototypeArchitect::CheckDinosaurMeshes()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    int32 DinosaurCount = 0;

    // Count static mesh actors that could represent dinosaurs
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (MeshActor && !MeshActor->IsPendingKill())
        {
            FString ActorName = MeshActor->GetName().ToLower();
            if (ActorName.Contains(TEXT("dinosaur")) || 
                ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("raptor")) || 
                ActorName.Contains(TEXT("brachio")))
            {
                DinosaurCount++;
            }
        }
    }

    return DinosaurCount >= 3; // Minimum 3 dinosaur meshes
}

bool UEng_PlayablePrototypeArchitect::CheckLightingComponents()
{
    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return false;

    bool bHasDirectionalLight = false;
    bool bHasSkyAtmosphere = false;
    bool bHasFog = false;

    // Check for directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (*ActorItr && !(*ActorItr)->IsPendingKill())
        {
            bHasDirectionalLight = true;
            break;
        }
    }

    // Check for sky atmosphere (component-based)
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
            {
                bHasSkyAtmosphere = true;
            }
            if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
            {
                bHasFog = true;
            }
        }
    }

    return bHasDirectionalLight && bHasSkyAtmosphere && bHasFog;
}

FEng_WalkAroundMilestone UEng_PlayablePrototypeArchitect::GetWalkAroundMilestoneStatus()
{
    return CurrentMilestone;
}

float UEng_PlayablePrototypeArchitect::CalculateCompletionPercentage()
{
    if (CurrentMilestone.TotalComponents == 0) return 0.0f;
    
    return (float)CurrentMilestone.CompletedComponents / (float)CurrentMilestone.TotalComponents * 100.0f;
}

TArray<FString> UEng_PlayablePrototypeArchitect::GetMissingComponents()
{
    TArray<FString> MissingComponents;
    
    for (const FEng_PrototypeRequirement& Requirement : PrototypeRequirements)
    {
        if (Requirement.bIsRequired && !Requirement.bIsImplemented)
        {
            MissingComponents.Add(Requirement.Description);
        }
    }
    
    return MissingComponents;
}

void UEng_PlayablePrototypeArchitect::UpdateMilestoneProgress()
{
    ValidatePlayablePrototype();
}

void UEng_PlayablePrototypeArchitect::GeneratePrototypeReport()
{
    FString Report = TEXT("=== WALK AROUND MILESTONE REPORT ===\n");
    Report += FString::Printf(TEXT("Completion: %.1f%% (%d/%d components)\n"), 
        CurrentMilestone.CompletionPercentage, 
        CurrentMilestone.CompletedComponents, 
        CurrentMilestone.TotalComponents);
    
    Report += TEXT("\nComponent Status:\n");
    for (const FEng_PrototypeRequirement& Requirement : PrototypeRequirements)
    {
        FString Status = Requirement.bIsImplemented ? TEXT("✓ COMPLETE") : TEXT("✗ MISSING");
        Report += FString::Printf(TEXT("- %s: %s\n"), *Requirement.Description, *Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void UEng_PlayablePrototypeArchitect::RunFullPrototypeValidation()
{
    ValidatePlayablePrototype();
    GeneratePrototypeReport();
}

bool UEng_PlayablePrototypeArchitect::ValidateCharacterController()
{
    return CheckThirdPersonCharacter() && CheckCameraSetup() && CheckPlayerMovement();
}

bool UEng_PlayablePrototypeArchitect::ValidateWorldEnvironment()
{
    return CheckLandscapeTerrain();
}

bool UEng_PlayablePrototypeArchitect::ValidateDinosaurPresence()
{
    return CheckDinosaurMeshes();
}

bool UEng_PlayablePrototypeArchitect::ValidateLightingSetup()
{
    return CheckLightingComponents();
}

bool UEng_PlayablePrototypeArchitect::EnforceMinimumViablePrototype()
{
    return ValidatePlayablePrototype();
}

bool UEng_PlayablePrototypeArchitect::ValidateGameplayReadiness()
{
    return ValidateCharacterController() && ValidateWorldEnvironment() && ValidateLightingSetup();
}

void UEng_PlayablePrototypeArchitect::LogValidationResult(const FString& Component, bool bResult)
{
    if (bResult)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ %s: VALID"), *Component);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ %s: MISSING/INVALID"), *Component);
    }
}