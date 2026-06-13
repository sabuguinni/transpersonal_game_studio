#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create marker mesh component
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    RootComponent = MarkerMesh;

    // Set default values
    CurrentCycle = 0;
    bPlayablePrototypeReady = false;

    // Try to load a basic mesh for the marker
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MarkerMesh->SetStaticMesh(CubeMeshAsset.Object);
        MarkerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMilestones();
    RefreshProductionState();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production state every 5 seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f)
    {
        UpdateTimer = 0.0f;
        CheckMilestoneProgress();
        ValidatePlayablePrototype();
    }
}

void AProductionCoordinator::UpdateAgentStatus(const FString& AgentName, int32 Cycle, bool HasDeliverables, const FString& LastDeliverable)
{
    // Find existing agent status or create new one
    FDir_AgentStatus* ExistingStatus = AgentStatuses.FindByPredicate([&AgentName](const FDir_AgentStatus& Status)
    {
        return Status.AgentName == AgentName;
    });

    if (ExistingStatus)
    {
        ExistingStatus->CycleNumber = Cycle;
        ExistingStatus->bHasDeliverables = HasDeliverables;
        ExistingStatus->LastDeliverable = LastDeliverable;
        ExistingStatus->CompletionPercentage = FMath::Min(100.0f, (float)Cycle * 5.0f);
    }
    else
    {
        FDir_AgentStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.CycleNumber = Cycle;
        NewStatus.bHasDeliverables = HasDeliverables;
        NewStatus.LastDeliverable = LastDeliverable;
        NewStatus.CompletionPercentage = FMath::Min(100.0f, (float)Cycle * 5.0f);
        AgentStatuses.Add(NewStatus);
    }

    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s - Cycle %d, Deliverables: %s"), 
           *AgentName, Cycle, HasDeliverables ? TEXT("Yes") : TEXT("No"));
}

void AProductionCoordinator::CheckMilestoneProgress()
{
    for (FDir_MilestoneStatus& Milestone : Milestones)
    {
        int32 CompletedCount = 0;
        Milestone.CompletedRequirements.Empty();

        for (const FString& Requirement : Milestone.Requirements)
        {
            if (CheckRequirement(Requirement))
            {
                CompletedCount++;
                Milestone.CompletedRequirements.Add(Requirement);
            }
        }

        Milestone.ProgressPercentage = (float)CompletedCount / (float)FMath::Max(1, Milestone.Requirements.Num()) * 100.0f;
        Milestone.bIsComplete = (CompletedCount == Milestone.Requirements.Num());
    }

    UpdateMilestoneProgress();
}

void AProductionCoordinator::ValidatePlayablePrototype()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Check for essential gameplay elements
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;

    // Count relevant actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();

        if (ActorName.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("character")))
        {
            bHasCharacter = true;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            bHasTerrain = true;
        }
        else if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("brachi")))
        {
            bHasDinosaurs = true;
        }
        else if (ActorName.Contains(TEXT("light")) || ActorName.Contains(TEXT("sun")))
        {
            bHasLighting = true;
        }
    }

    bPlayablePrototypeReady = bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;

    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Status: Character=%s, Terrain=%s, Dinosaurs=%s, Lighting=%s, Ready=%s"),
           bHasCharacter ? TEXT("✓") : TEXT("✗"),
           bHasTerrain ? TEXT("✓") : TEXT("✗"),
           bHasDinosaurs ? TEXT("✓") : TEXT("✗"),
           bHasLighting ? TEXT("✓") : TEXT("✗"),
           bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
}

FString AProductionCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Current Cycle: %d\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Playable Prototype Ready: %s\n"), bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n--- Agent Status ---\n");
    for (const FDir_AgentStatus& Status : AgentStatuses)
    {
        Report += FString::Printf(TEXT("%s: Cycle %d (%.1f%%) - %s\n"), 
                                  *Status.AgentName, 
                                  Status.CycleNumber, 
                                  Status.CompletionPercentage,
                                  Status.bHasDeliverables ? TEXT("Has Deliverables") : TEXT("No Deliverables"));
    }

    Report += TEXT("\n--- Milestone Progress ---\n");
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("%s: %.1f%% Complete (%d/%d requirements)\n"), 
                                  *Milestone.MilestoneName, 
                                  Milestone.ProgressPercentage,
                                  Milestone.CompletedRequirements.Num(),
                                  Milestone.Requirements.Num());
    }

    return Report;
}

void AProductionCoordinator::RefreshProductionState()
{
    CurrentCycle++;
    CheckMilestoneProgress();
    ValidatePlayablePrototype();
    
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AProductionCoordinator::InitializeMilestones()
{
    // Milestone 1: Walk Around Prototype
    FDir_MilestoneStatus WalkAroundMilestone;
    WalkAroundMilestone.MilestoneName = TEXT("Walk Around Prototype");
    WalkAroundMilestone.Requirements.Add(TEXT("ThirdPersonCharacter"));
    WalkAroundMilestone.Requirements.Add(TEXT("Landscape"));
    WalkAroundMilestone.Requirements.Add(TEXT("DinosaurActors"));
    WalkAroundMilestone.Requirements.Add(TEXT("BasicLighting"));
    WalkAroundMilestone.Requirements.Add(TEXT("PlayerMovement"));
    Milestones.Add(WalkAroundMilestone);

    // Milestone 2: Survival Systems
    FDir_MilestoneStatus SurvivalMilestone;
    SurvivalMilestone.MilestoneName = TEXT("Survival Systems");
    SurvivalMilestone.Requirements.Add(TEXT("HealthSystem"));
    SurvivalMilestone.Requirements.Add(TEXT("HungerSystem"));
    SurvivalMilestone.Requirements.Add(TEXT("StaminaSystem"));
    SurvivalMilestone.Requirements.Add(TEXT("SurvivalHUD"));
    Milestones.Add(SurvivalMilestone);

    // Milestone 3: Dinosaur AI
    FDir_MilestoneStatus DinosaurAIMilestone;
    DinosaurAIMilestone.MilestoneName = TEXT("Dinosaur AI");
    DinosaurAIMilestone.Requirements.Add(TEXT("DinosaurBehaviorTree"));
    DinosaurAIMilestone.Requirements.Add(TEXT("TerritorialBehavior"));
    DinosaurAIMilestone.Requirements.Add(TEXT("HuntingBehavior"));
    DinosaurAIMilestone.Requirements.Add(TEXT("FlockingBehavior"));
    Milestones.Add(DinosaurAIMilestone);
}

void AProductionCoordinator::UpdateMilestoneProgress()
{
    // Update visual representation based on milestone progress
    if (MarkerMesh)
    {
        float OverallProgress = 0.0f;
        for (const FDir_MilestoneStatus& Milestone : Milestones)
        {
            OverallProgress += Milestone.ProgressPercentage;
        }
        OverallProgress /= FMath::Max(1, Milestones.Num());

        // Change color based on progress (green = complete, yellow = in progress, red = not started)
        FLinearColor ProgressColor = FLinearColor::Red;
        if (OverallProgress > 75.0f)
        {
            ProgressColor = FLinearColor::Green;
        }
        else if (OverallProgress > 25.0f)
        {
            ProgressColor = FLinearColor::Yellow;
        }

        // Apply color to material (if available)
        MarkerMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(ProgressColor.R, ProgressColor.G, ProgressColor.B));
    }
}

bool AProductionCoordinator::CheckRequirement(const FString& Requirement)
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // Check for specific requirements in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        FString ReqLower = Requirement.ToLower();

        if (ActorName.Contains(ReqLower) || ActorLabel.Contains(ReqLower))
        {
            return true;
        }

        // Specific requirement mappings
        if (ReqLower == TEXT("thirdpersoncharacter") && (ActorName.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("character"))))
        {
            return true;
        }
        else if (ReqLower == TEXT("landscape") && (ActorName.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain"))))
        {
            return true;
        }
        else if (ReqLower == TEXT("dinosauractors") && (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("brachi"))))
        {
            return true;
        }
        else if (ReqLower == TEXT("basiclighting") && (ActorName.Contains(TEXT("light")) || ActorName.Contains(TEXT("sun"))))
        {
            return true;
        }
    }

    return false;
}