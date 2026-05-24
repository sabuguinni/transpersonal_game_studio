#include "Quest_SurvivalTutorialSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UQuest_SurvivalTutorialSystem::UQuest_SurvivalTutorialSystem()
{
    CurrentPhase = EQuest_TutorialPhase::NotStarted;
    bTutorialActive = false;
    TutorialStartTime = 0.0f;
    CurrentObjectiveIndex = 0;
}

void UQuest_SurvivalTutorialSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeTutorialObjectives();
    SetupResourceRequirements();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalTutorialSystem initialized"));
}

void UQuest_SurvivalTutorialSystem::StartTutorial()
{
    if (bTutorialActive)
    {
        return;
    }

    bTutorialActive = true;
    CurrentPhase = EQuest_TutorialPhase::BasicMovement;
    TutorialStartTime = GetWorld()->GetTimeSeconds();
    CurrentObjectiveIndex = 0;

    // Reset all objectives
    for (auto& Objective : TutorialObjectives)
    {
        Objective.bCompleted = false;
    }

    // Reset resource requirements
    for (auto& Resource : ResourceRequirements)
    {
        Resource.CurrentAmount = 0;
    }

    UpdateObjectiveMarkers();
    ShowHint(TEXT("Welcome to the Cretaceous Period! Use WASD to move around and explore your surroundings."));
    
    UE_LOG(LogTemp, Warning, TEXT("Survival tutorial started"));
}

void UQuest_SurvivalTutorialSystem::AdvanceToNextPhase()
{
    if (!bTutorialActive)
    {
        return;
    }

    // Complete current objective
    if (CurrentObjectiveIndex < TutorialObjectives.Num())
    {
        TutorialObjectives[CurrentObjectiveIndex].bCompleted = true;
    }

    // Move to next phase
    int32 NextPhaseIndex = static_cast<int32>(CurrentPhase) + 1;
    if (NextPhaseIndex < static_cast<int32>(EQuest_TutorialPhase::Completed))
    {
        CurrentPhase = static_cast<EQuest_TutorialPhase>(NextPhaseIndex);
        CurrentObjectiveIndex++;
        
        UpdateObjectiveMarkers();
        LogTutorialProgress();
        
        // Phase-specific logic
        switch (CurrentPhase)
        {
            case EQuest_TutorialPhase::ResourceGathering:
                ShowHint(TEXT("Now you need to gather basic resources. Look for stones, wood, and edible plants."));
                break;
            case EQuest_TutorialPhase::CraftingIntroduction:
                ShowHint(TEXT("Time to learn crafting! Combine resources to create useful tools and items."));
                break;
            case EQuest_TutorialPhase::ShelterBuilding:
                ShowHint(TEXT("Build a basic shelter to protect yourself from the elements and predators."));
                break;
            case EQuest_TutorialPhase::WaterLocation:
                ShowHint(TEXT("Find a reliable water source. Dehydration is a serious threat in this environment."));
                break;
            case EQuest_TutorialPhase::DinosaurAwareness:
                ShowHint(TEXT("Learn to identify different dinosaur species and their behaviors. Some are friendly, others are not."));
                TriggerDangerWarning(TEXT("Predator"));
                break;
            case EQuest_TutorialPhase::SafetyProtocols:
                ShowHint(TEXT("Master survival protocols: stealth, escape routes, and emergency procedures."));
                break;
            default:
                break;
        }
    }
    else
    {
        // Tutorial completed
        CurrentPhase = EQuest_TutorialPhase::Completed;
        bTutorialActive = false;
        ShowHint(TEXT("Congratulations! You've completed the survival tutorial. You're now ready to explore the prehistoric world."));
        UE_LOG(LogTemp, Warning, TEXT("Survival tutorial completed"));
    }
}

void UQuest_SurvivalTutorialSystem::CompleteCurrentObjective()
{
    if (!bTutorialActive || CurrentObjectiveIndex >= TutorialObjectives.Num())
    {
        return;
    }

    TutorialObjectives[CurrentObjectiveIndex].bCompleted = true;
    AdvanceToNextPhase();
}

bool UQuest_SurvivalTutorialSystem::CheckObjectiveCompletion(const FVector& PlayerLocation)
{
    if (!bTutorialActive || CurrentObjectiveIndex >= TutorialObjectives.Num())
    {
        return false;
    }

    const FQuest_TutorialObjective& CurrentObjective = TutorialObjectives[CurrentObjectiveIndex];
    
    // Check if player is within completion radius of objective marker
    float Distance = FVector::Dist(PlayerLocation, CurrentObjective.MarkerLocation);
    if (Distance <= CurrentObjective.CompletionRadius)
    {
        CompleteCurrentObjective();
        return true;
    }

    return false;
}

void UQuest_SurvivalTutorialSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    for (auto& Resource : ResourceRequirements)
    {
        if (Resource.ResourceType == ResourceType)
        {
            Resource.CurrentAmount = FMath::Min(Resource.CurrentAmount + Amount, Resource.RequiredAmount);
            
            UE_LOG(LogTemp, Warning, TEXT("Added %d %s resources. Current: %d/%d"), 
                Amount, 
                *UEnum::GetValueAsString(ResourceType),
                Resource.CurrentAmount, 
                Resource.RequiredAmount);
            
            // Check if this completes the resource gathering objective
            if (CurrentPhase == EQuest_TutorialPhase::ResourceGathering && HasRequiredResources())
            {
                ShowHint(TEXT("Great! You've gathered all required resources. Ready for the next phase."));
                AdvanceToNextPhase();
            }
            break;
        }
    }
}

bool UQuest_SurvivalTutorialSystem::HasRequiredResources() const
{
    for (const auto& Resource : ResourceRequirements)
    {
        if (Resource.CurrentAmount < Resource.RequiredAmount)
        {
            return false;
        }
    }
    return true;
}

FString UQuest_SurvivalTutorialSystem::GetCurrentObjectiveText() const
{
    if (!bTutorialActive || CurrentObjectiveIndex >= TutorialObjectives.Num())
    {
        return TEXT("No active objective");
    }

    return TutorialObjectives[CurrentObjectiveIndex].ObjectiveText;
}

float UQuest_SurvivalTutorialSystem::GetTutorialProgress() const
{
    if (TutorialObjectives.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedObjectives = 0;
    for (const auto& Objective : TutorialObjectives)
    {
        if (Objective.bCompleted)
        {
            CompletedObjectives++;
        }
    }

    return static_cast<float>(CompletedObjectives) / static_cast<float>(TutorialObjectives.Num());
}

void UQuest_SurvivalTutorialSystem::ShowHint(const FString& HintText)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("TUTORIAL: %s"), *HintText));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Tutorial Hint: %s"), *HintText);
}

void UQuest_SurvivalTutorialSystem::TriggerDangerWarning(const FString& DangerType)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, 
            FString::Printf(TEXT("DANGER WARNING: %s detected nearby!"), *DangerType));
    }
    
    UE_LOG(LogTemp, Error, TEXT("Danger Warning: %s"), *DangerType);
}

void UQuest_SurvivalTutorialSystem::InitializeTutorialObjectives()
{
    TutorialObjectives.Empty();

    // Basic Movement
    FQuest_TutorialObjective MovementObj;
    MovementObj.ObjectiveText = TEXT("Learn basic movement controls (WASD)");
    MovementObj.Phase = EQuest_TutorialPhase::BasicMovement;
    MovementObj.MarkerLocation = FVector(100, 100, 100);
    TutorialObjectives.Add(MovementObj);

    // Resource Gathering
    FQuest_TutorialObjective ResourceObj;
    ResourceObj.ObjectiveText = TEXT("Gather 3 stones, 2 pieces of wood, and 1 edible plant");
    ResourceObj.Phase = EQuest_TutorialPhase::ResourceGathering;
    ResourceObj.MarkerLocation = FVector(500, 0, 150);
    TutorialObjectives.Add(ResourceObj);

    // Crafting Introduction
    FQuest_TutorialObjective CraftingObj;
    CraftingObj.ObjectiveText = TEXT("Craft your first tool using gathered resources");
    CraftingObj.Phase = EQuest_TutorialPhase::CraftingIntroduction;
    CraftingObj.MarkerLocation = FVector(-300, 400, 100);
    TutorialObjectives.Add(CraftingObj);

    // Shelter Building
    FQuest_TutorialObjective ShelterObj;
    ShelterObj.ObjectiveText = TEXT("Build a basic shelter for protection");
    ShelterObj.Phase = EQuest_TutorialPhase::ShelterBuilding;
    ShelterObj.MarkerLocation = FVector(200, -500, 120);
    TutorialObjectives.Add(ShelterObj);

    // Water Location
    FQuest_TutorialObjective WaterObj;
    WaterObj.ObjectiveText = TEXT("Find and secure a water source");
    WaterObj.Phase = EQuest_TutorialPhase::WaterLocation;
    WaterObj.MarkerLocation = FVector(800, 200, 100);
    TutorialObjectives.Add(WaterObj);

    // Dinosaur Awareness
    FQuest_TutorialObjective DinosaurObj;
    DinosaurObj.ObjectiveText = TEXT("Observe different dinosaur species from a safe distance");
    DinosaurObj.Phase = EQuest_TutorialPhase::DinosaurAwareness;
    DinosaurObj.MarkerLocation = FVector(1200, 800, 200);
    TutorialObjectives.Add(DinosaurObj);

    // Safety Protocols
    FQuest_TutorialObjective SafetyObj;
    SafetyObj.ObjectiveText = TEXT("Practice emergency escape procedures");
    SafetyObj.Phase = EQuest_TutorialPhase::SafetyProtocols;
    SafetyObj.MarkerLocation = FVector(0, 0, 300);
    TutorialObjectives.Add(SafetyObj);
}

void UQuest_SurvivalTutorialSystem::SetupResourceRequirements()
{
    ResourceRequirements.Empty();

    FQuest_ResourceRequirement StoneReq;
    StoneReq.ResourceType = EQuest_ResourceType::Stone;
    StoneReq.RequiredAmount = 3;
    ResourceRequirements.Add(StoneReq);

    FQuest_ResourceRequirement WoodReq;
    WoodReq.ResourceType = EQuest_ResourceType::Wood;
    WoodReq.RequiredAmount = 2;
    ResourceRequirements.Add(WoodReq);

    FQuest_ResourceRequirement PlantReq;
    PlantReq.ResourceType = EQuest_ResourceType::Plant;
    PlantReq.RequiredAmount = 1;
    ResourceRequirements.Add(PlantReq);
}

void UQuest_SurvivalTutorialSystem::UpdateObjectiveMarkers()
{
    // This would update visual markers in the world
    // Implementation depends on marker actor system
    UE_LOG(LogTemp, Warning, TEXT("Updating objective markers for phase: %s"), 
        *UEnum::GetValueAsString(CurrentPhase));
}

void UQuest_SurvivalTutorialSystem::PlayTutorialAudio(const FString& AudioCue)
{
    // Audio playback implementation
    UE_LOG(LogTemp, Warning, TEXT("Playing tutorial audio: %s"), *AudioCue);
}

void UQuest_SurvivalTutorialSystem::LogTutorialProgress()
{
    float Progress = GetTutorialProgress() * 100.0f;
    UE_LOG(LogTemp, Warning, TEXT("Tutorial Progress: %.1f%% - Phase: %s"), 
        Progress, *UEnum::GetValueAsString(CurrentPhase));
}

// AQuest_TutorialMarker Implementation
AQuest_TutorialMarker::AQuest_TutorialMarker()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create mesh component
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    RootComponent = MarkerMesh;

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(200.0f);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_TutorialMarker::OnTriggerBeginOverlap);

    AssociatedPhase = EQuest_TutorialPhase::NotStarted;
    bIsActive = true;
    MarkerDescription = TEXT("Tutorial Marker");
}

void AQuest_TutorialMarker::SetMarkerActive(bool bActive)
{
    bIsActive = bActive;
    SetActorHiddenInGame(!bActive);
    SetActorEnableCollision(bActive);
}

void AQuest_TutorialMarker::SetMarkerType(EQuest_TutorialPhase Phase)
{
    AssociatedPhase = Phase;
    
    // Update marker appearance based on phase
    switch (Phase)
    {
        case EQuest_TutorialPhase::ResourceGathering:
            MarkerDescription = TEXT("Resource Collection Point");
            break;
        case EQuest_TutorialPhase::CraftingIntroduction:
            MarkerDescription = TEXT("Crafting Area");
            break;
        case EQuest_TutorialPhase::ShelterBuilding:
            MarkerDescription = TEXT("Shelter Location");
            break;
        case EQuest_TutorialPhase::WaterLocation:
            MarkerDescription = TEXT("Water Source");
            break;
        case EQuest_TutorialPhase::DinosaurAwareness:
            MarkerDescription = TEXT("Observation Point");
            break;
        case EQuest_TutorialPhase::SafetyProtocols:
            MarkerDescription = TEXT("Safe Zone");
            break;
        default:
            MarkerDescription = TEXT("Tutorial Marker");
            break;
    }
}

void AQuest_TutorialMarker::OnPlayerEnterMarker()
{
    if (!bIsActive)
    {
        return;
    }

    // Get tutorial system and notify of marker interaction
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UQuest_SurvivalTutorialSystem* TutorialSystem = 
                GameInstance->GetSubsystem<UQuest_SurvivalTutorialSystem>())
            {
                TutorialSystem->CheckObjectiveCompletion(GetActorLocation());
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Player entered tutorial marker: %s"), *MarkerDescription);
}

void AQuest_TutorialMarker::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if it's the player character
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        OnPlayerEnterMarker();
    }
}