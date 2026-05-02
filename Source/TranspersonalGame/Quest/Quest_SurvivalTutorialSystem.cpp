#include "Quest_SurvivalTutorialSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AQuest_SurvivalTutorialSystem::AQuest_SurvivalTutorialSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize tutorial state
    CurrentStep = EQuest_TutorialStep::None;
    bTutorialActive = false;
    TutorialTimer = 0.0f;
    
    // Movement tracking
    LastPlayerPosition = FVector::ZeroVector;
    MovementDistance = 0.0f;
    bPlayerHasMoved = false;
    
    // Resource tracking
    RequiredResourceCount = 3;
    bFirstItemCrafted = false;
}

void AQuest_SurvivalTutorialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTutorialObjectives();
    
    // Start tutorial after a brief delay
    GetWorld()->GetTimerManager().SetTimer(TutorialTimerHandle, this, &AQuest_SurvivalTutorialSystem::StartTutorial, 2.0f, false);
}

void AQuest_SurvivalTutorialSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bTutorialActive)
        return;
    
    TutorialTimer += DeltaTime;
    
    // Check current step progress
    switch (CurrentStep)
    {
        case EQuest_TutorialStep::Movement:
            if (CheckMovementObjective())
            {
                CompleteCurrentStep();
            }
            break;
            
        case EQuest_TutorialStep::ResourceGathering:
            if (CheckResourceGatheringObjective())
            {
                CompleteCurrentStep();
            }
            break;
            
        case EQuest_TutorialStep::CraftingBasics:
            if (CheckCraftingObjective())
            {
                CompleteCurrentStep();
            }
            break;
            
        default:
            break;
    }
    
    UpdateTutorialProgress();
}

void AQuest_SurvivalTutorialSystem::InitializeTutorialObjectives()
{
    // Movement objective
    FQuest_TutorialObjective MovementObj;
    MovementObj.ObjectiveText = TEXT("Learn to move around the prehistoric world");
    MovementObj.HintText = TEXT("Use WASD keys to move. Walk at least 10 meters to continue.");
    MovementObj.bIsCompleted = false;
    MovementObj.TimeLimit = 60.0f;
    TutorialObjectives.Add(EQuest_TutorialStep::Movement, MovementObj);
    
    // Resource gathering objective
    FQuest_TutorialObjective ResourceObj;
    ResourceObj.ObjectiveText = TEXT("Gather basic survival resources");
    ResourceObj.HintText = TEXT("Collect rocks, sticks, and leaves. Look for them scattered around the area.");
    ResourceObj.bIsCompleted = false;
    ResourceObj.TimeLimit = 120.0f;
    TutorialObjectives.Add(EQuest_TutorialStep::ResourceGathering, ResourceObj);
    
    // Crafting objective
    FQuest_TutorialObjective CraftingObj;
    CraftingObj.ObjectiveText = TEXT("Craft your first survival tool");
    CraftingObj.HintText = TEXT("Press C to open crafting menu. Try making a stone axe with 2 rocks and 1 stick.");
    CraftingObj.bIsCompleted = false;
    CraftingObj.TimeLimit = 180.0f;
    TutorialObjectives.Add(EQuest_TutorialStep::CraftingBasics, CraftingObj);
    
    // Shelter building objective
    FQuest_TutorialObjective ShelterObj;
    ShelterObj.ObjectiveText = TEXT("Build a basic shelter");
    ShelterObj.HintText = TEXT("Use gathered materials to construct a simple lean-to shelter.");
    ShelterObj.bIsCompleted = false;
    ShelterObj.TimeLimit = 300.0f;
    TutorialObjectives.Add(EQuest_TutorialStep::ShelterBuilding, ShelterObj);
    
    // Dinosaur awareness objective
    FQuest_TutorialObjective DinosaurObj;
    DinosaurObj.ObjectiveText = TEXT("Observe dinosaur behavior safely");
    DinosaurObj.HintText = TEXT("Approach a herbivore carefully. Stay crouched and maintain distance.");
    DinosaurObj.bIsCompleted = false;
    DinosaurObj.TimeLimit = 240.0f;
    TutorialObjectives.Add(EQuest_TutorialStep::DinosaurAwareness, DinosaurObj);
}

void AQuest_SurvivalTutorialSystem::StartTutorial()
{
    bTutorialActive = true;
    CurrentStep = EQuest_TutorialStep::Movement;
    TutorialTimer = 0.0f;
    
    // Get player starting position
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        LastPlayerPosition = PC->GetPawn()->GetActorLocation();
    }
    
    // Display first tutorial message
    if (TutorialObjectives.Contains(CurrentStep))
    {
        FQuest_TutorialObjective& Objective = TutorialObjectives[CurrentStep];
        DisplayTutorialMessage(Objective.ObjectiveText);
        ShowTutorialHint(Objective.HintText);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Survival Tutorial Started - Step: Movement"));
}

void AQuest_SurvivalTutorialSystem::CompleteCurrentStep()
{
    if (TutorialObjectives.Contains(CurrentStep))
    {
        TutorialObjectives[CurrentStep].bIsCompleted = true;
        UE_LOG(LogTemp, Warning, TEXT("Tutorial Step Completed: %d"), (int32)CurrentStep);
    }
    
    AdvanceToNextStep();
}

void AQuest_SurvivalTutorialSystem::AdvanceToNextStep()
{
    switch (CurrentStep)
    {
        case EQuest_TutorialStep::Movement:
            CurrentStep = EQuest_TutorialStep::ResourceGathering;
            break;
        case EQuest_TutorialStep::ResourceGathering:
            CurrentStep = EQuest_TutorialStep::CraftingBasics;
            break;
        case EQuest_TutorialStep::CraftingBasics:
            CurrentStep = EQuest_TutorialStep::ShelterBuilding;
            break;
        case EQuest_TutorialStep::ShelterBuilding:
            CurrentStep = EQuest_TutorialStep::DinosaurAwareness;
            break;
        case EQuest_TutorialStep::DinosaurAwareness:
            CurrentStep = EQuest_TutorialStep::Completed;
            bTutorialActive = false;
            ShowTutorialComplete();
            UE_LOG(LogTemp, Warning, TEXT("Survival Tutorial Completed!"));
            return;
        default:
            break;
    }
    
    // Display new objective
    if (TutorialObjectives.Contains(CurrentStep))
    {
        FQuest_TutorialObjective& Objective = TutorialObjectives[CurrentStep];
        DisplayTutorialMessage(Objective.ObjectiveText);
        ShowTutorialHint(Objective.HintText);
    }
    
    TutorialTimer = 0.0f;
}

void AQuest_SurvivalTutorialSystem::ShowTutorialHint(const FString& HintText)
{
    // Display hint for 5 seconds
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("HINT: %s"), *HintText));
    }
}

bool AQuest_SurvivalTutorialSystem::CheckMovementObjective()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
        return false;
    
    FVector CurrentPosition = PC->GetPawn()->GetActorLocation();
    float DistanceMoved = FVector::Dist(LastPlayerPosition, CurrentPosition);
    
    if (DistanceMoved > 50.0f) // 50cm movement threshold
    {
        MovementDistance += DistanceMoved;
        LastPlayerPosition = CurrentPosition;
        bPlayerHasMoved = true;
    }
    
    return MovementDistance >= 1000.0f; // 10 meters total movement
}

bool AQuest_SurvivalTutorialSystem::CheckResourceGatheringObjective()
{
    return CollectedResources.Num() >= RequiredResourceCount;
}

bool AQuest_SurvivalTutorialSystem::CheckCraftingObjective()
{
    return bFirstItemCrafted;
}

void AQuest_SurvivalTutorialSystem::OnPlayerMovement()
{
    if (CurrentStep == EQuest_TutorialStep::Movement)
    {
        bPlayerHasMoved = true;
    }
}

void AQuest_SurvivalTutorialSystem::OnResourceCollected(const FString& ResourceType)
{
    if (CurrentStep == EQuest_TutorialStep::ResourceGathering)
    {
        CollectedResources.AddUnique(ResourceType);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Collected: %s (%d/%d)"), *ResourceType, CollectedResources.Num(), RequiredResourceCount));
        }
    }
}

void AQuest_SurvivalTutorialSystem::OnItemCrafted(const FString& ItemName)
{
    if (CurrentStep == EQuest_TutorialStep::CraftingBasics)
    {
        CraftedItems.Add(ItemName);
        bFirstItemCrafted = true;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                FString::Printf(TEXT("Crafted: %s"), *ItemName));
        }
    }
}

void AQuest_SurvivalTutorialSystem::UpdateTutorialProgress()
{
    float Progress = 0.0f;
    
    switch (CurrentStep)
    {
        case EQuest_TutorialStep::Movement:
            Progress = FMath::Clamp(MovementDistance / 1000.0f, 0.0f, 1.0f);
            break;
        case EQuest_TutorialStep::ResourceGathering:
            Progress = FMath::Clamp((float)CollectedResources.Num() / (float)RequiredResourceCount, 0.0f, 1.0f);
            break;
        case EQuest_TutorialStep::CraftingBasics:
            Progress = bFirstItemCrafted ? 1.0f : 0.0f;
            break;
        default:
            Progress = 0.0f;
            break;
    }
    
    UpdateTutorialProgress(Progress);
}

void AQuest_SurvivalTutorialSystem::CheckTutorialCompletion()
{
    if (CurrentStep == EQuest_TutorialStep::Completed)
    {
        bTutorialActive = false;
        ShowTutorialComplete();
    }
}