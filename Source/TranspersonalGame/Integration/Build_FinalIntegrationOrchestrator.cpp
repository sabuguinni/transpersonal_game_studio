#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Character.h"

ABuild_FinalIntegrationOrchestrator::ABuild_FinalIntegrationOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create integration visualizer mesh
    IntegrationVisualizerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntegrationVisualizerMesh"));
    IntegrationVisualizerMesh->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentIntegrationStatus = EBuild_IntegrationStatus::Pending;
    MaxWorldActorCount = 1000;
    MinPerformanceScore = 75.0f;
    MinIntegrationCompleteness = 90.0f;
    ValidationInterval = 5.0f;
    bAutoValidateOnBeginPlay = true;
    bContinuousValidation = true;
    LastValidationTime = 0.0f;
    bValidationInProgress = false;

    // Initialize integration metrics
    IntegrationMetrics = FBuild_IntegrationMetrics();
    IntegrationMetrics.TotalSystemsCount = 7; // Character, GameMode, GameState, WorldGen, Foliage, VFX, QA

    // Setup critical systems list
    InitializeCriticalSystemsList();
}

void ABuild_FinalIntegrationOrchestrator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting integration orchestration"));

    // Setup visualizer
    SetupIntegrationVisualizer();

    // Auto-validate if enabled
    if (bAutoValidateOnBeginPlay)
    {
        StartIntegrationOrchestration();
    }
}

void ABuild_FinalIntegrationOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Continuous validation if enabled
    if (bContinuousValidation && !bValidationInProgress)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastValidationTime >= ValidationInterval)
        {
            ValidateAllSystems();
            LastValidationTime = CurrentTime;
        }
    }

    // Update visualizer appearance
    UpdateVisualizerAppearance();
}

void ABuild_FinalIntegrationOrchestrator::InitializeCriticalSystemsList()
{
    CriticalSystemClasses.Empty();
    CriticalSystemClasses.Add(TEXT("TranspersonalCharacter"));
    CriticalSystemClasses.Add(TEXT("TranspersonalGameMode"));
    CriticalSystemClasses.Add(TEXT("TranspersonalGameState"));
    CriticalSystemClasses.Add(TEXT("PCGWorldGenerator"));
    CriticalSystemClasses.Add(TEXT("FoliageManager"));
    CriticalSystemClasses.Add(TEXT("VFX_ImpactEffectsManager"));
    CriticalSystemClasses.Add(TEXT("QA_VFXSystemValidator"));
}

void ABuild_FinalIntegrationOrchestrator::SetupIntegrationVisualizer()
{
    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        IntegrationVisualizerMesh->SetStaticMesh(CubeMeshAsset.Object);
        IntegrationVisualizerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }
}

void ABuild_FinalIntegrationOrchestrator::UpdateVisualizerAppearance()
{
    if (!IntegrationVisualizerMesh)
        return;

    // Change color based on integration status
    FLinearColor StatusColor;
    switch (CurrentIntegrationStatus)
    {
        case EBuild_IntegrationStatus::Pending:
            StatusColor = FLinearColor::Yellow;
            break;
        case EBuild_IntegrationStatus::InProgress:
            StatusColor = FLinearColor::Blue;
            break;
        case EBuild_IntegrationStatus::Complete:
            StatusColor = FLinearColor::Green;
            break;
        case EBuild_IntegrationStatus::Failed:
            StatusColor = FLinearColor::Red;
            break;
        case EBuild_IntegrationStatus::Validated:
            StatusColor = FLinearColor::White;
            break;
        default:
            StatusColor = FLinearColor::Gray;
            break;
    }

    // Apply color to mesh (simplified approach)
    // In a full implementation, this would use dynamic materials
}

void ABuild_FinalIntegrationOrchestrator::StartIntegrationOrchestration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting integration orchestration"));

    CurrentIntegrationStatus = EBuild_IntegrationStatus::InProgress;
    OnIntegrationStatusChanged.Broadcast(CurrentIntegrationStatus);

    // Validate all systems
    ValidateAllSystems();

    // Update metrics
    UpdateIntegrationMetrics();

    // Generate report
    GenerateIntegrationReport();

    // Check if integration is complete
    if (IsBuildReady())
    {
        FinalizeIntegration();
    }
}

void ABuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (bValidationInProgress)
        return;

    bValidationInProgress = true;
    SystemValidationResults.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating all systems"));

    // Validate each critical system
    ValidateCharacterSystem();
    ValidateGameModeSystem();
    ValidateWorldGenerationSystem();
    ValidateFoliageSystem();
    ValidateVFXSystem();
    ValidateQASystem();

    bValidationInProgress = false;
    LastValidationTime = GetWorld()->GetTimeSeconds();
}

bool ABuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    FString SystemName = TEXT("Character System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check if TranspersonalCharacter class exists
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (CharacterClass)
    {
        bLoaded = true;
        Score += 50.0f;
        Message = TEXT("Character class loaded successfully");

        // Check if character is operational in world
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), CharacterClass, FoundActors);
        if (FoundActors.Num() > 0)
        {
            bOperational = true;
            Score += 50.0f;
            Message = TEXT("Character system fully operational");
        }
    }
    else
    {
        Message = TEXT("Character class not found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateGameModeSystem()
{
    FString SystemName = TEXT("GameMode System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check current game mode
    AGameModeBase* CurrentGameMode = GetWorld()->GetAuthGameMode();
    if (CurrentGameMode)
    {
        bLoaded = true;
        bOperational = true;
        Score = 100.0f;
        Message = FString::Printf(TEXT("GameMode active: %s"), *CurrentGameMode->GetClass()->GetName());
    }
    else
    {
        Message = TEXT("No active GameMode found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateWorldGenerationSystem()
{
    FString SystemName = TEXT("World Generation System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check if PCGWorldGenerator class exists
    UClass* WorldGenClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PCGWorldGenerator"));
    if (WorldGenClass)
    {
        bLoaded = true;
        Score += 50.0f;
        Message = TEXT("World generation class loaded");

        // Check for world generation actors
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), WorldGenClass, FoundActors);
        if (FoundActors.Num() > 0)
        {
            bOperational = true;
            Score += 50.0f;
            Message = TEXT("World generation system operational");
        }
    }
    else
    {
        Message = TEXT("World generation class not found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateFoliageSystem()
{
    FString SystemName = TEXT("Foliage System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check if FoliageManager class exists
    UClass* FoliageClass = FindObject<UClass>(ANY_PACKAGE, TEXT("FoliageManager"));
    if (FoliageClass)
    {
        bLoaded = true;
        Score += 50.0f;
        Message = TEXT("Foliage manager class loaded");

        // Check for foliage actors
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), FoliageClass, FoundActors);
        if (FoundActors.Num() > 0)
        {
            bOperational = true;
            Score += 50.0f;
            Message = TEXT("Foliage system operational");
        }
    }
    else
    {
        Message = TEXT("Foliage manager class not found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    FString SystemName = TEXT("VFX System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check if VFX_ImpactEffectsManager class exists
    UClass* VFXClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_ImpactEffectsManager"));
    if (VFXClass)
    {
        bLoaded = true;
        Score += 50.0f;
        Message = TEXT("VFX manager class loaded");

        // Check for VFX actors
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), VFXClass, FoundActors);
        if (FoundActors.Num() > 0)
        {
            bOperational = true;
            Score += 50.0f;
            Message = TEXT("VFX system operational");
        }
    }
    else
    {
        Message = TEXT("VFX manager class not found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateQASystem()
{
    FString SystemName = TEXT("QA System");
    bool bLoaded = false;
    bool bOperational = false;
    FString Message = TEXT("Validation in progress");
    float Score = 0.0f;

    // Check if QA_VFXSystemValidator class exists
    UClass* QAClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QA_VFXSystemValidator"));
    if (QAClass)
    {
        bLoaded = true;
        Score += 50.0f;
        Message = TEXT("QA validator class loaded");

        // Check for QA actors
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), QAClass, FoundActors);
        if (FoundActors.Num() > 0)
        {
            bOperational = true;
            Score += 50.0f;
            Message = TEXT("QA system operational");
        }
    }
    else
    {
        Message = TEXT("QA validator class not found");
    }

    FBuild_SystemValidationResult Result = CreateValidationResult(SystemName, bLoaded, bOperational, Message, Score);
    SystemValidationResults.Add(Result);
    OnSystemValidationComplete.Broadcast(Result);

    return bLoaded && bOperational;
}

void ABuild_FinalIntegrationOrchestrator::UpdateIntegrationMetrics()
{
    IntegrationMetrics.LoadedSystemsCount = 0;
    IntegrationMetrics.OperationalSystemsCount = 0;
    float TotalScore = 0.0f;

    // Count loaded and operational systems
    for (const FBuild_SystemValidationResult& Result : SystemValidationResults)
    {
        if (Result.bIsLoaded)
        {
            IntegrationMetrics.LoadedSystemsCount++;
        }
        if (Result.bIsOperational)
        {
            IntegrationMetrics.OperationalSystemsCount++;
        }
        TotalScore += Result.PerformanceScore;
    }

    // Calculate metrics
    IntegrationMetrics.OverallPerformanceScore = SystemValidationResults.Num() > 0 ? TotalScore / SystemValidationResults.Num() : 0.0f;
    IntegrationMetrics.IntegrationCompleteness = IntegrationMetrics.TotalSystemsCount > 0 ? 
        (float)IntegrationMetrics.OperationalSystemsCount / IntegrationMetrics.TotalSystemsCount * 100.0f : 0.0f;

    // Count world actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    IntegrationMetrics.WorldActorCount = AllActors.Num();

    IntegrationMetrics.LastValidationTime = FDateTime::Now();
}

void ABuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *GetIntegrationStatusString());
    UE_LOG(LogTemp, Warning, TEXT("Loaded Systems: %d/%d"), IntegrationMetrics.LoadedSystemsCount, IntegrationMetrics.TotalSystemsCount);
    UE_LOG(LogTemp, Warning, TEXT("Operational Systems: %d/%d"), IntegrationMetrics.OperationalSystemsCount, IntegrationMetrics.TotalSystemsCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.1f%%"), IntegrationMetrics.OverallPerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("Integration Completeness: %.1f%%"), IntegrationMetrics.IntegrationCompleteness);
    UE_LOG(LogTemp, Warning, TEXT("World Actor Count: %d"), IntegrationMetrics.WorldActorCount);

    for (const FBuild_SystemValidationResult& Result : SystemValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("System: %s - Loaded: %s, Operational: %s, Score: %.1f%% - %s"), 
            *Result.SystemName,
            Result.bIsLoaded ? TEXT("YES") : TEXT("NO"),
            Result.bIsOperational ? TEXT("YES") : TEXT("NO"),
            Result.PerformanceScore,
            *Result.ValidationMessage);
    }
}

bool ABuild_FinalIntegrationOrchestrator::IsBuildReady()
{
    return IntegrationMetrics.IntegrationCompleteness >= MinIntegrationCompleteness &&
           IntegrationMetrics.OverallPerformanceScore >= MinPerformanceScore &&
           IntegrationMetrics.WorldActorCount <= MaxWorldActorCount;
}

void ABuild_FinalIntegrationOrchestrator::FinalizeIntegration()
{
    CurrentIntegrationStatus = EBuild_IntegrationStatus::Complete;
    OnIntegrationStatusChanged.Broadcast(CurrentIntegrationStatus);
    OnIntegrationComplete.Broadcast();

    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration finalized successfully"));
}

FString ABuild_FinalIntegrationOrchestrator::GetIntegrationStatusString()
{
    switch (CurrentIntegrationStatus)
    {
        case EBuild_IntegrationStatus::Pending: return TEXT("Pending");
        case EBuild_IntegrationStatus::InProgress: return TEXT("In Progress");
        case EBuild_IntegrationStatus::Complete: return TEXT("Complete");
        case EBuild_IntegrationStatus::Failed: return TEXT("Failed");
        case EBuild_IntegrationStatus::Validated: return TEXT("Validated");
        default: return TEXT("Unknown");
    }
}

FBuild_SystemValidationResult ABuild_FinalIntegrationOrchestrator::CreateValidationResult(const FString& SystemName, bool bLoaded, bool bOperational, const FString& Message, float Score)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.bIsLoaded = bLoaded;
    Result.bIsOperational = bOperational;
    Result.ValidationMessage = Message;
    Result.PerformanceScore = Score;
    return Result;
}

float ABuild_FinalIntegrationOrchestrator::CalculateSystemPerformanceScore()
{
    return IntegrationMetrics.OverallPerformanceScore;
}

float ABuild_FinalIntegrationOrchestrator::CalculateIntegrationCompleteness()
{
    return IntegrationMetrics.IntegrationCompleteness;
}

bool ABuild_FinalIntegrationOrchestrator::IsPerformanceWithinThresholds()
{
    return IntegrationMetrics.OverallPerformanceScore >= MinPerformanceScore &&
           IntegrationMetrics.WorldActorCount <= MaxWorldActorCount;
}

void ABuild_FinalIntegrationOrchestrator::PrepareBuildPackage()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Preparing build package"));
    // Implementation for build packaging preparation
}

void ABuild_FinalIntegrationOrchestrator::ResetIntegrationState()
{
    CurrentIntegrationStatus = EBuild_IntegrationStatus::Pending;
    SystemValidationResults.Empty();
    IntegrationMetrics = FBuild_IntegrationMetrics();
    IntegrationMetrics.TotalSystemsCount = 7;
}

void ABuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    GenerateIntegrationReport();
}

void ABuild_FinalIntegrationOrchestrator::ValidateSystemByClass(const FString& SystemClassName)
{
    // Implementation for individual system validation
    UE_LOG(LogTemp, Warning, TEXT("Validating system: %s"), *SystemClassName);
}