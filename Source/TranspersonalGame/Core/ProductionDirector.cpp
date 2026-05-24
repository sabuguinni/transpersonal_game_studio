#include "ProductionDirector.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AProductionDirector::AProductionDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create visual mesh component
	DirectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectorMesh"));
	DirectorMesh->SetupAttachment(RootComponent);

	// Set default mesh to cube
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		DirectorMesh->SetStaticMesh(CubeMesh.Object);
		DirectorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
	}

	// Initialize default values
	bTerrainComplete = false;
	bCharacterMovementComplete = false;
	bDinosaursPlaced = false;
	bLightingComplete = false;
	bSurvivalHUDComplete = false;
	Milestone1Progress = 0.0f;
	CurrentFocus = TEXT("Terrain Generation");
}

void AProductionDirector::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeMilestone1Tasks();
	LogProgressToConsole();
	
	UE_LOG(LogTemp, Warning, TEXT("Production Director: Milestone 1 coordination active"));
}

void AProductionDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update progress every 5 seconds
	static float ProgressTimer = 0.0f;
	ProgressTimer += DeltaTime;
	
	if (ProgressTimer >= 5.0f)
	{
		UpdateMilestoneProgress();
		ProgressTimer = 0.0f;
	}
}

void AProductionDirector::InitializeMilestone1Tasks()
{
	PendingTasks.Empty();
	CompletedTasks.Empty();

	// Define Milestone 1 tasks in priority order
	PendingTasks.Add(TEXT("Create varied terrain with hills and valleys"));
	PendingTasks.Add(TEXT("Implement ThirdPersonCharacter with WASD movement"));
	PendingTasks.Add(TEXT("Add camera boom and follow camera"));
	PendingTasks.Add(TEXT("Place 3-5 dinosaur meshes in world"));
	PendingTasks.Add(TEXT("Configure directional light and sky atmosphere"));
	PendingTasks.Add(TEXT("Create survival HUD with health/hunger/thirst bars"));
	PendingTasks.Add(TEXT("Test player can walk, run, jump"));
	PendingTasks.Add(TEXT("Verify dinosaurs are visible and have collision"));

	UE_LOG(LogTemp, Log, TEXT("Production Director: Initialized %d Milestone 1 tasks"), PendingTasks.Num());
}

void AProductionDirector::UpdateMilestoneProgress()
{
	int32 TotalTasks = CompletedTasks.Num() + PendingTasks.Num();
	if (TotalTasks > 0)
	{
		Milestone1Progress = (float)CompletedTasks.Num() / (float)TotalTasks * 100.0f;
	}

	// Update current focus based on next pending task
	if (PendingTasks.Num() > 0)
	{
		CurrentFocus = PendingTasks[0];
	}
	else
	{
		CurrentFocus = TEXT("Milestone 1 Complete!");
	}

	LogProgressToConsole();
}

void AProductionDirector::MarkTaskComplete(const FString& TaskName)
{
	if (PendingTasks.Contains(TaskName))
	{
		PendingTasks.Remove(TaskName);
		CompletedTasks.Add(TaskName);
		
		UE_LOG(LogTemp, Warning, TEXT("Production Director: Task completed - %s"), *TaskName);
		UpdateMilestoneProgress();
	}
}

void AProductionDirector::AddPendingTask(const FString& TaskName)
{
	if (!PendingTasks.Contains(TaskName) && !CompletedTasks.Contains(TaskName))
	{
		PendingTasks.Add(TaskName);
		UE_LOG(LogTemp, Log, TEXT("Production Director: Added task - %s"), *TaskName);
	}
}

FString AProductionDirector::GetNextPriorityTask()
{
	if (PendingTasks.Num() > 0)
	{
		return PendingTasks[0];
	}
	return TEXT("No pending tasks");
}

bool AProductionDirector::IsMilestone1Complete()
{
	return bTerrainComplete && bCharacterMovementComplete && bDinosaursPlaced && 
		   bLightingComplete && bSurvivalHUDComplete;
}

void AProductionDirector::NotifyAgentProgress(const FString& AgentName, const FString& TaskCompleted)
{
	FString LogMessage = FString::Printf(TEXT("Agent %s completed: %s"), *AgentName, *TaskCompleted);
	UE_LOG(LogTemp, Warning, TEXT("Production Director: %s"), *LogMessage);
	
	MarkTaskComplete(TaskCompleted);
}

FString AProductionDirector::GetTaskForAgent(const FString& AgentName)
{
	// Assign tasks based on agent specialization
	if (AgentName.Contains(TEXT("WorldGenerator")) || AgentName.Contains(TEXT("Environment")))
	{
		if (!bTerrainComplete)
		{
			return TEXT("Create varied terrain with hills and valleys");
		}
	}
	else if (AgentName.Contains(TEXT("Character")) || AgentName.Contains(TEXT("Animation")))
	{
		if (!bCharacterMovementComplete)
		{
			return TEXT("Implement ThirdPersonCharacter with WASD movement");
		}
	}
	else if (AgentName.Contains(TEXT("AI")) || AgentName.Contains(TEXT("NPC")))
	{
		if (!bDinosaursPlaced)
		{
			return TEXT("Place 3-5 dinosaur meshes in world");
		}
	}
	else if (AgentName.Contains(TEXT("Lighting")) || AgentName.Contains(TEXT("Atmosphere")))
	{
		if (!bLightingComplete)
		{
			return TEXT("Configure directional light and sky atmosphere");
		}
	}
	
	return GetNextPriorityTask();
}

void AProductionDirector::LogProgressToConsole()
{
	UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE 1 PROGRESS: %.1f%% ==="), Milestone1Progress);
	UE_LOG(LogTemp, Warning, TEXT("Current Focus: %s"), *CurrentFocus);
	UE_LOG(LogTemp, Warning, TEXT("Completed: %d | Pending: %d"), CompletedTasks.Num(), PendingTasks.Num());
	
	if (IsMilestone1Complete())
	{
		UE_LOG(LogTemp, Error, TEXT("*** MILESTONE 1 COMPLETE - PLAYABLE PROTOTYPE READY ***"));
	}
}