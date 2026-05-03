#include "ProductionDirector.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AProductionDirector::AProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Update every 5 seconds

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create visual marker
    VisualMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMarker"));
    VisualMarker->SetupAttachment(RootComponent);
    
    // Load cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualMarker->SetStaticMesh(CubeMeshAsset.Object);
        VisualMarker->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetWorldSize(200.0f);
    StatusDisplay->SetTextRenderColor(FColor::Orange);
    StatusDisplay->SetRelativeLocation(FVector(0, 0, 150));

    // Initialize production tracking
    CurrentCycle = 4;
    CurrentPhase = TEXT("MILESTONE 1 - WALK AROUND");
    LastScanTime = 0.0f;
}

void AProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMilestones();
    ScanLevelForProgress();
    UpdateDisplayText();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Initialized and tracking Milestone 1"));
}

void AProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastScanTime += DeltaTime;
    if (LastScanTime >= 10.0f) // Scan every 10 seconds
    {
        ScanLevelForProgress();
        UpdateDisplayText();
        LastScanTime = 0.0f;
    }
}

void AProductionDirector::InitializeMilestones()
{
    Milestones.Empty();

    // Milestone 1: Walk Around
    FDir_MilestoneData Milestone1;
    Milestone1.Name = TEXT("Walk Around");
    Milestone1.Description = TEXT("Player can move in world with visible dinosaurs");
    Milestone1.RequiredElements = 5; // Character + 3 Dinosaurs + Terrain
    Milestone1.Status = EDir_MilestoneStatus::InProgress;
    Milestones.Add(Milestone1);

    // Milestone 2: Basic Interaction
    FDir_MilestoneData Milestone2;
    Milestone2.Name = TEXT("Basic Interaction");
    Milestone2.Description = TEXT("Player can interact with objects and environment");
    Milestone2.RequiredElements = 3;
    Milestone2.Status = EDir_MilestoneStatus::NotStarted;
    Milestones.Add(Milestone2);

    // Milestone 3: Survival Mechanics
    FDir_MilestoneData Milestone3;
    Milestone3.Name = TEXT("Survival Mechanics");
    Milestone3.Description = TEXT("Health, hunger, thirst, stamina systems working");
    Milestone3.RequiredElements = 4;
    Milestone3.Status = EDir_MilestoneStatus::NotStarted;
    Milestones.Add(Milestone3);
}

void AProductionDirector::ScanLevelForProgress()
{
    if (!GetWorld()) return;

    // Count playable elements for Milestone 1
    int32 CharacterCount = CountActorsOfType(TEXT("Character"));
    int32 DinosaurCount = CountActorsOfType(TEXT("Dinosaur"));
    int32 TerrainCount = CountActorsOfType(TEXT("Landscape"));

    // Update Milestone 1 progress
    if (Milestones.Num() > 0)
    {
        int32 CompletedElements = 0;
        if (CharacterCount > 0) CompletedElements++;
        if (DinosaurCount >= 3) CompletedElements += 3;
        if (TerrainCount > 0) CompletedElements++;

        Milestones[0].CompletedElements = CompletedElements;
        
        if (CompletedElements >= Milestones[0].RequiredElements)
        {
            Milestones[0].Status = EDir_MilestoneStatus::Completed;
        }
        else if (CompletedElements > 0)
        {
            Milestones[0].Status = EDir_MilestoneStatus::InProgress;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Scanned - Characters: %d, Dinosaurs: %d, Terrain: %d"), 
           CharacterCount, DinosaurCount, TerrainCount);
}

int32 AProductionDirector::CountActorsOfType(const FString& ActorType) const
{
    if (!GetWorld()) return 0;

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(ActorType))
        {
            Count++;
        }
    }
    return Count;
}

void AProductionDirector::UpdateDisplayText()
{
    if (!StatusDisplay) return;

    FString DisplayText = FString::Printf(TEXT("%s\nCycle: %d\n\n"), *CurrentPhase, CurrentCycle);
    
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        FString StatusText;
        switch (Milestone.Status)
        {
            case EDir_MilestoneStatus::NotStarted:
                StatusText = TEXT("NOT STARTED");
                break;
            case EDir_MilestoneStatus::InProgress:
                StatusText = TEXT("IN PROGRESS");
                break;
            case EDir_MilestoneStatus::Completed:
                StatusText = TEXT("COMPLETED");
                break;
            case EDir_MilestoneStatus::Blocked:
                StatusText = TEXT("BLOCKED");
                break;
        }

        DisplayText += FString::Printf(TEXT("%s: %s\n%d/%d Elements\n\n"), 
                                      *Milestone.Name, 
                                      *StatusText,
                                      Milestone.CompletedElements,
                                      Milestone.RequiredElements);
    }

    StatusDisplay->SetText(DisplayText);
}

void AProductionDirector::UpdateMilestoneStatus(const FString& MilestoneName, EDir_MilestoneStatus NewStatus)
{
    for (FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.Name == MilestoneName)
        {
            Milestone.Status = NewStatus;
            UpdateDisplayText();
            break;
        }
    }
}

void AProductionDirector::IncrementMilestoneProgress(const FString& MilestoneName)
{
    for (FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.Name == MilestoneName)
        {
            Milestone.CompletedElements = FMath::Min(Milestone.CompletedElements + 1, Milestone.RequiredElements);
            UpdateDisplayText();
            break;
        }
    }
}

FString AProductionDirector::GetMilestoneReport() const
{
    FString Report = FString::Printf(TEXT("PRODUCTION REPORT - CYCLE %d\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Phase: %s\n\n"), *CurrentPhase);

    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("Milestone: %s\n"), *Milestone.Name);
        Report += FString::Printf(TEXT("Status: %s\n"), 
                                 Milestone.Status == EDir_MilestoneStatus::Completed ? TEXT("COMPLETED") :
                                 Milestone.Status == EDir_MilestoneStatus::InProgress ? TEXT("IN PROGRESS") :
                                 Milestone.Status == EDir_MilestoneStatus::Blocked ? TEXT("BLOCKED") : TEXT("NOT STARTED"));
        Report += FString::Printf(TEXT("Progress: %d/%d\n"), Milestone.CompletedElements, Milestone.RequiredElements);
        Report += FString::Printf(TEXT("Description: %s\n\n"), *Milestone.Description);
    }

    return Report;
}

void AProductionDirector::RefreshDisplay()
{
    ScanLevelForProgress();
    UpdateDisplayText();
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Display refreshed manually"));
}