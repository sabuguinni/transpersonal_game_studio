#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"
#include "TranspersonalGame/AI/DinosaurTRex.h"
#include "TranspersonalGame/AI/DinosaurCombatAIController.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    bIntegrationValidated = false;
    LastValidationTime = 0.0f;
    ValidationInterval = 10.0f;
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager: Starting integration validation"));
    
    // Schedule initial validation
    GetWorld()->GetTimerManager().SetTimer(
        ValidationTimerHandle,
        this,
        &UBuildIntegrationManager::ValidateSystemIntegration,
        ValidationInterval,
        true
    );
}

void UBuildIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update validation metrics
    LastValidationTime += DeltaTime;
}

void UBuildIntegrationManager::ValidateSystemIntegration()
{
    if (!GetWorld())
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("BuildIntegrationManager: No valid world for integration validation"));
        return;
    }
    
    FBuild_IntegrationReport Report;
    Report.ValidationTime = GetWorld()->GetTimeSeconds();
    Report.bAllSystemsOperational = true;
    
    // Validate core systems
    ValidateCharacterSystem(Report);
    ValidateGameStateSystem(Report);
    ValidateAISystem(Report);
    ValidateWorldSystems(Report);
    
    // Store report
    IntegrationReports.Add(Report);
    
    // Keep only last 10 reports
    if (IntegrationReports.Num() > 10)
    {
        IntegrationReports.RemoveAt(0);
    }
    
    bIntegrationValidated = Report.bAllSystemsOperational;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration validation complete: %s"), 
           Report.bAllSystemsOperational ? TEXT("PASS") : TEXT("FAIL"));
}

void UBuildIntegrationManager::ValidateCharacterSystem(FBuild_IntegrationReport& Report)
{
    // Check if TranspersonalCharacter class is available
    UClass* CharacterClass = ATranspersonalCharacter::StaticClass();
    if (!CharacterClass)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("TranspersonalCharacter class not found"));
        return;
    }
    
    // Find character instances in world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, FoundActors);
    
    Report.CharacterSystemStatus.bIsLoaded = true;
    Report.CharacterSystemStatus.ActiveInstances = FoundActors.Num();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Character system validation: %d instances found"), FoundActors.Num());
}

void UBuildIntegrationManager::ValidateGameStateSystem(FBuild_IntegrationReport& Report)
{
    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("GameState not found"));
        return;
    }
    
    ATranspersonalGameState* TransGameState = Cast<ATranspersonalGameState>(GameState);
    if (!TransGameState)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("TranspersonalGameState not active"));
        return;
    }
    
    Report.GameStateSystemStatus.bIsLoaded = true;
    Report.GameStateSystemStatus.ActiveInstances = 1;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("GameState system validation: PASS"));
}

void UBuildIntegrationManager::ValidateAISystem(FBuild_IntegrationReport& Report)
{
    // Check for dinosaur AI controllers
    UClass* AIControllerClass = ADinosaurCombatAIController::StaticClass();
    if (!AIControllerClass)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("DinosaurCombatAIController class not found"));
        return;
    }
    
    // Check for dinosaur pawns
    UClass* DinosaurClass = ADinosaurTRex::StaticClass();
    if (!DinosaurClass)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("DinosaurTRex class not found"));
        return;
    }
    
    TArray<AActor*> DinosaurActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), DinosaurClass, DinosaurActors);
    
    Report.AISystemStatus.bIsLoaded = true;
    Report.AISystemStatus.ActiveInstances = DinosaurActors.Num();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("AI system validation: %d dinosaurs found"), DinosaurActors.Num());
}

void UBuildIntegrationManager::ValidateWorldSystems(FBuild_IntegrationReport& Report)
{
    // Count total actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    Report.WorldSystemStatus.bIsLoaded = true;
    Report.WorldSystemStatus.ActiveInstances = AllActors.Num();
    
    // Check for critical world objects
    int32 LightCount = 0;
    int32 MeshCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->IsA<ALight>())
        {
            LightCount++;
        }
        else if (Actor->IsA<AStaticMeshActor>())
        {
            MeshCount++;
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("World validation: %d total actors, %d lights, %d meshes"), 
           AllActors.Num(), LightCount, MeshCount);
    
    // Minimum world requirements
    if (LightCount == 0)
    {
        Report.bAllSystemsOperational = false;
        Report.FailedSystems.Add(TEXT("No lighting found in world"));
    }
}

FBuild_IntegrationReport UBuildIntegrationManager::GetLatestIntegrationReport() const
{
    if (IntegrationReports.Num() > 0)
    {
        return IntegrationReports.Last();
    }
    
    return FBuild_IntegrationReport();
}

bool UBuildIntegrationManager::IsSystemIntegrationValid() const
{
    return bIntegrationValidated;
}

void UBuildIntegrationManager::ForceValidation()
{
    ValidateSystemIntegration();
}