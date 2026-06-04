#include "Eng_PlayablePrototypeValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TimerManager.h"

UEng_PlayablePrototypeValidator::UEng_PlayablePrototypeValidator()
{
    ValidationInterval = 5.0f;
    bAutoValidateOnPlay = true;
    bLogValidationDetails = true;
}

void UEng_PlayablePrototypeValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PlayablePrototypeValidator: Subsystem initialized"));
    
    // Initialize milestone data
    CurrentMilestone.MilestoneName = TEXT("Walk Around Prototype");
    CurrentMilestone.Requirements.Empty();
    
    // Add core requirements
    FEng_PrototypeRequirement CharacterReq;
    CharacterReq.RequirementName = TEXT("Character Movement");
    CharacterReq.RequiredCount = 1;
    CurrentMilestone.Requirements.Add(CharacterReq);
    
    FEng_PrototypeRequirement TerrainReq;
    TerrainReq.RequirementName = TEXT("Landscape Terrain");
    TerrainReq.RequiredCount = 1;
    CurrentMilestone.Requirements.Add(TerrainReq);
    
    FEng_PrototypeRequirement DinosaurReq;
    DinosaurReq.RequirementName = TEXT("Dinosaur Actors");
    DinosaurReq.RequiredCount = 5;
    CurrentMilestone.Requirements.Add(DinosaurReq);
    
    FEng_PrototypeRequirement LightingReq;
    LightingReq.RequirementName = TEXT("Scene Lighting");
    LightingReq.RequiredCount = 3;
    CurrentMilestone.Requirements.Add(LightingReq);
    
    FEng_PrototypeRequirement CameraReq;
    CameraReq.RequirementName = TEXT("Camera System");
    CameraReq.RequiredCount = 1;
    CurrentMilestone.Requirements.Add(CameraReq);
    
    if (bAutoValidateOnPlay)
    {
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, this, 
            &UEng_PlayablePrototypeValidator::ValidateWalkAroundMilestone, 
            ValidationInterval, true);
    }
}

void UEng_PlayablePrototypeValidator::Deinitialize()
{
    if (GetWorld() && GetWorld()->GetTimerManager().IsValidHandle(ValidationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    Super::Deinitialize();
}

void UEng_PlayablePrototypeValidator::ValidateWalkAroundMilestone()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PlayablePrototypeValidator: Running milestone validation"));
    
    ValidateCharacterMovement();
    ValidateTerrain();
    ValidateDinosaurActors();
    ValidateLighting();
    ValidatePlayerStart();
    ValidateCamera();
    
    // Calculate overall completion
    float TotalCompletion = 0.0f;
    int32 CompletedRequirements = 0;
    
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        TotalCompletion += Requirement.CompletionPercentage;
        if (Requirement.bIsCompleted)
        {
            CompletedRequirements++;
        }
    }
    
    CurrentMilestone.OverallCompletion = CurrentMilestone.Requirements.Num() > 0 ? 
        TotalCompletion / CurrentMilestone.Requirements.Num() : 0.0f;
    
    CurrentMilestone.bIsMilestoneComplete = (CompletedRequirements == CurrentMilestone.Requirements.Num());
    CurrentMilestone.LastValidationTime = FDateTime::Now();
    
    if (bLogValidationDetails)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone Completion: %.1f%% (%d/%d requirements)"), 
            CurrentMilestone.OverallCompletion, CompletedRequirements, CurrentMilestone.Requirements.Num());
    }
}

void UEng_PlayablePrototypeValidator::ValidateCharacterMovement()
{
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        if (Requirement.RequirementName == TEXT("Character Movement"))
        {
            int32 CharacterCount = 0;
            bool bHasMovementComponent = false;
            
            for (TActorIterator<ACharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                ACharacter* Character = *ActorItr;
                if (Character && Character->GetCharacterMovement())
                {
                    CharacterCount++;
                    bHasMovementComponent = true;
                    break;
                }
            }
            
            Requirement.CurrentCount = CharacterCount;
            Requirement.CompletionPercentage = bHasMovementComponent ? 100.0f : 0.0f;
            Requirement.bIsCompleted = bHasMovementComponent;
            Requirement.ValidationDetails = FString::Printf(TEXT("Characters with movement: %d"), CharacterCount);
            break;
        }
    }
}

void UEng_PlayablePrototypeValidator::ValidateTerrain()
{
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        if (Requirement.RequirementName == TEXT("Landscape Terrain"))
        {
            int32 LandscapeCount = CountActorsOfClass(ALandscape::StaticClass());
            
            Requirement.CurrentCount = LandscapeCount;
            Requirement.CompletionPercentage = LandscapeCount > 0 ? 100.0f : 0.0f;
            Requirement.bIsCompleted = LandscapeCount > 0;
            Requirement.ValidationDetails = FString::Printf(TEXT("Landscape actors: %d"), LandscapeCount);
            break;
        }
    }
}

void UEng_PlayablePrototypeValidator::ValidateDinosaurActors()
{
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        if (Requirement.RequirementName == TEXT("Dinosaur Actors"))
        {
            int32 DinosaurCount = 0;
            
            // Count actors with "Dinosaur" or specific dinosaur names in their name
            for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor)
                {
                    FString ActorName = Actor->GetName().ToLower();
                    if (ActorName.Contains(TEXT("dinosaur")) || 
                        ActorName.Contains(TEXT("trex")) || 
                        ActorName.Contains(TEXT("raptor")) || 
                        ActorName.Contains(TEXT("brachio")))
                    {
                        DinosaurCount++;
                    }
                }
            }
            
            Requirement.CurrentCount = DinosaurCount;
            Requirement.CompletionPercentage = FMath::Clamp(
                (float)DinosaurCount / (float)Requirement.RequiredCount * 100.0f, 0.0f, 100.0f);
            Requirement.bIsCompleted = DinosaurCount >= Requirement.RequiredCount;
            Requirement.ValidationDetails = FString::Printf(TEXT("Dinosaur actors: %d/%d"), 
                DinosaurCount, Requirement.RequiredCount);
            break;
        }
    }
}

void UEng_PlayablePrototypeValidator::ValidateLighting()
{
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        if (Requirement.RequirementName == TEXT("Scene Lighting"))
        {
            int32 LightCount = 0;
            
            LightCount += CountActorsOfClass(ADirectionalLight::StaticClass());
            LightCount += CountActorsOfClass(ASkyLight::StaticClass());
            LightCount += CountActorsOfClass(AExponentialHeightFog::StaticClass());
            
            Requirement.CurrentCount = LightCount;
            Requirement.CompletionPercentage = FMath::Clamp(
                (float)LightCount / (float)Requirement.RequiredCount * 100.0f, 0.0f, 100.0f);
            Requirement.bIsCompleted = LightCount >= Requirement.RequiredCount;
            Requirement.ValidationDetails = FString::Printf(TEXT("Lighting actors: %d/%d"), 
                LightCount, Requirement.RequiredCount);
            break;
        }
    }
}

void UEng_PlayablePrototypeValidator::ValidatePlayerStart()
{
    int32 PlayerStartCount = CountActorsOfClass(APlayerStart::StaticClass());
    
    if (bLogValidationDetails)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerStart validation: %d found"), PlayerStartCount);
    }
}

void UEng_PlayablePrototypeValidator::ValidateCamera()
{
    for (FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        if (Requirement.RequirementName == TEXT("Camera System"))
        {
            bool bHasCameraSystem = false;
            
            for (TActorIterator<ACharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                ACharacter* Character = *ActorItr;
                if (Character)
                {
                    USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
                    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
                    
                    if (SpringArm && Camera)
                    {
                        bHasCameraSystem = true;
                        break;
                    }
                }
            }
            
            Requirement.CurrentCount = bHasCameraSystem ? 1 : 0;
            Requirement.CompletionPercentage = bHasCameraSystem ? 100.0f : 0.0f;
            Requirement.bIsCompleted = bHasCameraSystem;
            Requirement.ValidationDetails = bHasCameraSystem ? 
                TEXT("Camera system found") : TEXT("No camera system detected");
            break;
        }
    }
}

FEng_MilestoneData UEng_PlayablePrototypeValidator::GetCurrentMilestoneStatus()
{
    return CurrentMilestone;
}

float UEng_PlayablePrototypeValidator::GetOverallPrototypeCompletion()
{
    return CurrentMilestone.OverallCompletion;
}

bool UEng_PlayablePrototypeValidator::IsPrototypePlayable()
{
    return CurrentMilestone.OverallCompletion >= 80.0f;
}

void UEng_PlayablePrototypeValidator::GenerateValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PLAYABLE PROTOTYPE VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Milestone: %s"), *CurrentMilestone.MilestoneName);
    UE_LOG(LogTemp, Warning, TEXT("Overall Completion: %.1f%%"), CurrentMilestone.OverallCompletion);
    UE_LOG(LogTemp, Warning, TEXT("Is Complete: %s"), CurrentMilestone.bIsMilestoneComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Is Playable: %s"), IsPrototypePlayable() ? TEXT("YES") : TEXT("NO"));
    
    for (const FEng_PrototypeRequirement& Requirement : CurrentMilestone.Requirements)
    {
        UE_LOG(LogTemp, Warning, TEXT("- %s: %.1f%% (%s)"), 
            *Requirement.RequirementName, 
            Requirement.CompletionPercentage,
            *Requirement.ValidationDetails);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %s"), 
        *CurrentMilestone.LastValidationTime.ToString());
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UEng_PlayablePrototypeValidator::RunFullValidation()
{
    ValidateWalkAroundMilestone();
    GenerateValidationReport();
}

bool UEng_PlayablePrototypeValidator::CheckActorExists(UClass* ActorClass)
{
    if (!ActorClass || !GetWorld())
    {
        return false;
    }
    
    for (TActorIterator<AActor> ActorItr(GetWorld(), ActorClass); ActorItr; ++ActorItr)
    {
        return true; // Found at least one
    }
    
    return false;
}

int32 UEng_PlayablePrototypeValidator::CountActorsOfClass(UClass* ActorClass)
{
    if (!ActorClass || !GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld(), ActorClass); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

bool UEng_PlayablePrototypeValidator::ValidateActorComponent(AActor* Actor, UClass* ComponentClass)
{
    if (!Actor || !ComponentClass)
    {
        return false;
    }
    
    return Actor->FindComponentByClass(ComponentClass) != nullptr;
}