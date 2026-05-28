#include "ProductionDirector.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADir_ProductionDirector::ADir_ProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Update every 5 seconds

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create status indicator mesh
    StatusIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatusIndicator"));
    StatusIndicator->SetupAttachment(RootComponent);
    StatusIndicator->SetRelativeLocation(FVector(0, 0, 0));
    StatusIndicator->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));

    // Create text display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0, 0, 100));
    StatusDisplay->SetWorldSize(80.0f);
    StatusDisplay->SetTextRenderColor(FColor::Yellow);
    StatusDisplay->SetHorizontalAlignment(EHTA_Center);
    StatusDisplay->SetVerticalAlignment(EVTA_TextCenter);

    // Initialize agent deliverables tracking
    AgentDeliverables.Add(TEXT("Agent02_EngineArchitect"), false);
    AgentDeliverables.Add(TEXT("Agent03_CoreSystems"), false);
    AgentDeliverables.Add(TEXT("Agent05_WorldGenerator"), false);
    AgentDeliverables.Add(TEXT("Agent06_EnvironmentArt"), false);
    AgentDeliverables.Add(TEXT("Agent09_CharacterArtist"), false);
    AgentDeliverables.Add(TEXT("Agent10_Animation"), false);
    AgentDeliverables.Add(TEXT("Agent12_CombatAI"), false);
    AgentDeliverables.Add(TEXT("Agent15_Narrative"), false);
}

void ADir_ProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStatusDisplay();
    UpdateMilestoneProgress();
    
    // Log startup
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Milestone 1 coordination started"));
    LogProductionStatus();
}

void ADir_ProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMilestoneProgress();
    UpdateStatusText();
}

void ADir_ProductionDirector::InitializeStatusDisplay()
{
    if (StatusDisplay)
    {
        FString InitialText = TEXT("PRODUCTION DIRECTOR\nMilestone 1: WALK AROUND\n\nInitializing...");
        StatusDisplay->SetText(FText::FromString(InitialText));
    }

    // Set indicator color based on progress
    if (StatusIndicator && StatusIndicator->GetStaticMesh())
    {
        // Start with red (not complete)
        UMaterialInstanceDynamic* DynamicMat = StatusIndicator->CreateDynamicMaterialInstance(0);
        if (DynamicMat)
        {
            DynamicMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Red);
        }
    }
}

void ADir_ProductionDirector::UpdateMilestoneProgress()
{
    // Check world state to determine progress
    UWorld* World = GetWorld();
    if (!World) return;

    // Count relevant actors
    int32 CharacterCount = 0;
    int32 DinosaurCount = 0;
    int32 TerrainCount = 0;
    int32 LightingCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("TranspersonalCharacter")))
        {
            CharacterCount++;
        }
        else if (ClassName.Contains(TEXT("Dinosaur")) || ClassName.Contains(TEXT("TRex")) || 
                 ClassName.Contains(TEXT("Raptor")) || ClassName.Contains(TEXT("Brachiosaurus")))
        {
            DinosaurCount++;
        }
        else if (ClassName.Contains(TEXT("Landscape")))
        {
            TerrainCount++;
        }
        else if (ClassName.Contains(TEXT("DirectionalLight")) || ClassName.Contains(TEXT("SkyLight")))
        {
            LightingCount++;
        }
    }

    // Update milestone flags
    bCharacterMovementComplete = (CharacterCount > 0);
    bDinosaursPlaced = (DinosaurCount >= 3);
    bTerrainComplete = (TerrainCount > 0);
    bLightingComplete = (LightingCount >= 2);
    
    // HUD completion would need to be set externally by Agent #12
}

void ADir_ProductionDirector::MarkAgentComplete(const FString& AgentName, const FString& Deliverable)
{
    if (AgentDeliverables.Contains(AgentName))
    {
        AgentDeliverables[AgentName] = true;
        UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: %s completed deliverable: %s"), *AgentName, *Deliverable);
    }
}

float ADir_ProductionDirector::GetMilestone1Progress() const
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = 5;

    if (bCharacterMovementComplete) CompletedTasks++;
    if (bTerrainComplete) CompletedTasks++;
    if (bDinosaursPlaced) CompletedTasks++;
    if (bLightingComplete) CompletedTasks++;
    if (bSurvivalHUDComplete) CompletedTasks++;

    return (float)CompletedTasks / (float)TotalTasks;
}

void ADir_ProductionDirector::LogProductionStatus()
{
    float Progress = GetMilestone1Progress();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%%"), Progress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Character Movement: %s"), bCharacterMovementComplete ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %s"), bTerrainComplete ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs Placed: %s"), bDinosaursPlaced ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("Lighting: %s"), bLightingComplete ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("Survival HUD: %s"), bSurvivalHUDComplete ? TEXT("COMPLETE") : TEXT("PENDING"));
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void ADir_ProductionDirector::RefreshStatusDisplay()
{
    UpdateMilestoneProgress();
    UpdateStatusText();
    LogProductionStatus();
}

void ADir_ProductionDirector::UpdateStatusText()
{
    if (!StatusDisplay) return;

    float Progress = GetMilestone1Progress();
    FString ProgressBar = GetProgressBar(Progress);
    
    FString StatusText = FString::Printf(TEXT(
        "PRODUCTION DIRECTOR\n"
        "Milestone 1: WALK AROUND\n"
        "%s %.1f%%\n\n"
        "Character Movement: %s\n"
        "Terrain: %s\n"
        "Dinosaurs (3+): %s\n"
        "Lighting: %s\n"
        "Survival HUD: %s\n\n"
        "Next Priority:\n"
        "%s"
    ),
    *ProgressBar,
    Progress * 100.0f,
    bCharacterMovementComplete ? TEXT("✓") : TEXT("✗"),
    bTerrainComplete ? TEXT("✓") : TEXT("✗"),
    bDinosaursPlaced ? TEXT("✓") : TEXT("✗"),
    bLightingComplete ? TEXT("✓") : TEXT("✗"),
    bSurvivalHUDComplete ? TEXT("✓") : TEXT("✗"),
    Progress < 1.0f ? TEXT("Continue Milestone 1") : TEXT("Ready for Milestone 2")
    );

    StatusDisplay->SetText(FText::FromString(StatusText));

    // Update indicator color
    if (StatusIndicator)
    {
        UMaterialInstanceDynamic* DynamicMat = StatusIndicator->CreateDynamicMaterialInstance(0);
        if (DynamicMat)
        {
            FLinearColor IndicatorColor = Progress < 0.5f ? FLinearColor::Red : 
                                         Progress < 1.0f ? FLinearColor::Yellow : 
                                         FLinearColor::Green;
            DynamicMat->SetVectorParameterValue(TEXT("BaseColor"), IndicatorColor);
        }
    }
}

FString ADir_ProductionDirector::GetProgressBar(float Progress) const
{
    int32 BarLength = 20;
    int32 FilledLength = FMath::RoundToInt(Progress * BarLength);
    
    FString Bar = TEXT("[");
    for (int32 i = 0; i < BarLength; i++)
    {
        Bar += (i < FilledLength) ? TEXT("█") : TEXT("░");
    }
    Bar += TEXT("]");
    
    return Bar;
}