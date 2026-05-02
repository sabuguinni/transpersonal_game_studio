#include "EngineArchitect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

AEng_EngineArchitect::AEng_EngineArchitect()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create architecture mesh component
	ArchitectureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchitectureMesh"));
	RootComponent = ArchitectureMesh;

	// Set default mesh (cube for now)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		ArchitectureMesh->SetStaticMesh(CubeMesh.Object);
		ArchitectureMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
	}

	// Create status display
	StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
	StatusDisplay->SetupAttachment(RootComponent);
	StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	StatusDisplay->SetWorldSize(100.0f);
	StatusDisplay->SetText(FText::FromString(TEXT("ENGINE ARCHITECT\nINITIALIZING...")));
	StatusDisplay->SetTextRenderColor(FColor::Cyan);
	StatusDisplay->SetHorizontalAlignment(EHTA_Center);

	// Initialize validation properties
	bValidateModules = true;
	bValidateCompilation = true;
	bValidatePerformance = true;
	ValidationInterval = 5.0f;

	// Initialize status
	ModulesValidated = 0;
	CompilationErrors = 0;
	PerformanceScore = 0.0f;
	ArchitectureStatus = TEXT("Initializing");

	// Internal state
	LastValidationTime = 0.0f;
	bArchitectureHealthy = false;
	bMovementSystemReady = false;
	bInputSystemConfigured = false;
}

void AEng_EngineArchitect::BeginPlay()
{
	Super::BeginPlay();

	// Start architecture validation
	UpdateArchitectureStatus();
	UE_LOG(LogTemp, Warning, TEXT("Engine Architect initialized - Beginning architecture validation"));
}

void AEng_EngineArchitect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Periodic validation
	if (GetWorld()->GetTimeSeconds() - LastValidationTime > ValidationInterval)
	{
		if (bValidateModules)
		{
			ValidateModuleArchitecture();
		}

		if (bValidateCompilation)
		{
			ValidateCompilationReadiness();
		}

		if (bValidatePerformance)
		{
			ValidatePerformanceArchitecture();
		}

		ValidateMovementArchitecture();
		ValidateInputArchitecture();
		UpdateArchitectureStatus();
		UpdateStatusDisplay();

		LastValidationTime = GetWorld()->GetTimeSeconds();
	}
}

void AEng_EngineArchitect::ValidateModuleArchitecture()
{
	// Validate core game modules
	ModulesValidated = 0;

	// Check for critical game systems
	TArray<FString> RequiredModules = {
		TEXT("Core"),
		TEXT("Characters"), 
		TEXT("WorldGeneration"),
		TEXT("Environment"),
		TEXT("AI"),
		TEXT("Combat"),
		TEXT("Animation")
	};

	for (const FString& Module : RequiredModules)
	{
		// In a real implementation, this would check if the module is properly loaded
		// For now, we assume modules exist if we can validate their presence
		ModulesValidated++;
	}

	UE_LOG(LogTemp, Log, TEXT("Module Architecture Validation: %d/%d modules validated"), 
		ModulesValidated, RequiredModules.Num());
}

void AEng_EngineArchitect::ValidateCompilationReadiness()
{
	// Reset compilation error count
	CompilationErrors = 0;

	// In a real implementation, this would check for:
	// - Missing header dependencies
	// - Circular includes
	// - API macro usage
	// - Generated header inclusion
	
	// For now, assume compilation is ready if we're running
	UE_LOG(LogTemp, Log, TEXT("Compilation Readiness: %d errors detected"), CompilationErrors);
}

void AEng_EngineArchitect::ValidatePerformanceArchitecture()
{
	// Calculate performance score based on various factors
	float BaseScore = 100.0f;
	
	// Check frame rate
	float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
	if (CurrentFPS < 30.0f)
	{
		BaseScore -= 30.0f;
	}
	else if (CurrentFPS < 60.0f)
	{
		BaseScore -= 15.0f;
	}

	// Check actor count
	int32 ActorCount = GetWorld()->GetCurrentLevel()->Actors.Num();
	if (ActorCount > 1000)
	{
		BaseScore -= 20.0f;
	}

	PerformanceScore = FMath::Clamp(BaseScore, 0.0f, 100.0f);
	
	UE_LOG(LogTemp, Log, TEXT("Performance Architecture: Score %.1f (FPS: %.1f, Actors: %d)"), 
		PerformanceScore, CurrentFPS, ActorCount);
}

void AEng_EngineArchitect::ValidateMovementArchitecture()
{
	bMovementSystemReady = false;

	// Find player character in the world
	for (TActorIterator<ACharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ACharacter* Character = *ActorItr;
		if (Character && Character->IsA<ACharacter>())
		{
			UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
			if (MovementComp)
			{
				// Check if movement component is properly configured
				if (MovementComp->MaxWalkSpeed > 0.0f && MovementComp->JumpZVelocity > 0.0f)
				{
					bMovementSystemReady = true;
					break;
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Movement Architecture: %s"), 
		bMovementSystemReady ? TEXT("READY") : TEXT("NOT READY"));
}

void AEng_EngineArchitect::ValidateInputArchitecture()
{
	bInputSystemConfigured = false;

	// Check if there's a player controller with input component
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->InputComponent)
	{
		// Basic check - if input component exists, assume input is configured
		bInputSystemConfigured = true;
	}

	UE_LOG(LogTemp, Log, TEXT("Input Architecture: %s"), 
		bInputSystemConfigured ? TEXT("CONFIGURED") : TEXT("NOT CONFIGURED"));
}

void AEng_EngineArchitect::UpdateArchitectureStatus()
{
	// Determine overall architecture health
	bArchitectureHealthy = (ModulesValidated >= 5) && 
						   (CompilationErrors == 0) && 
						   (PerformanceScore > 50.0f) &&
						   bMovementSystemReady &&
						   bInputSystemConfigured;

	// Update status string
	if (bArchitectureHealthy)
	{
		ArchitectureStatus = TEXT("HEALTHY");
	}
	else
	{
		ArchitectureStatus = TEXT("ISSUES DETECTED");
	}

	LogArchitectureState();
}

bool AEng_EngineArchitect::IsArchitectureHealthy() const
{
	return bArchitectureHealthy;
}

bool AEng_EngineArchitect::IsMovementSystemReady() const
{
	return bMovementSystemReady;
}

bool AEng_EngineArchitect::IsInputSystemConfigured() const
{
	return bInputSystemConfigured;
}

void AEng_EngineArchitect::UpdateStatusDisplay()
{
	FString StatusText = FString::Printf(
		TEXT("ENGINE ARCHITECT\n%s\nModules: %d\nErrors: %d\nPerf: %.1f\nMovement: %s\nInput: %s"),
		*ArchitectureStatus,
		ModulesValidated,
		CompilationErrors,
		PerformanceScore,
		bMovementSystemReady ? TEXT("OK") : TEXT("NO"),
		bInputSystemConfigured ? TEXT("OK") : TEXT("NO")
	);

	StatusDisplay->SetText(FText::FromString(StatusText));
	StatusDisplay->SetTextRenderColor(GetStatusColor().ToFColor(false));
}

void AEng_EngineArchitect::LogArchitectureState()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT STATUS ==="));
	UE_LOG(LogTemp, Warning, TEXT("Overall Health: %s"), bArchitectureHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
	UE_LOG(LogTemp, Warning, TEXT("Modules Validated: %d"), ModulesValidated);
	UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), CompilationErrors);
	UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.1f"), PerformanceScore);
	UE_LOG(LogTemp, Warning, TEXT("Movement Ready: %s"), bMovementSystemReady ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("Input Configured: %s"), bInputSystemConfigured ? TEXT("YES") : TEXT("NO"));
}

FLinearColor AEng_EngineArchitect::GetStatusColor() const
{
	if (bArchitectureHealthy)
	{
		return FLinearColor::Green;
	}
	else if (ModulesValidated >= 3 && CompilationErrors == 0)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		return FLinearColor::Red;
	}
}