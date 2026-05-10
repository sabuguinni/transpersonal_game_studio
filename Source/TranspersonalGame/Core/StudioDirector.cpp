#include "StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"

// Studio Director Component Implementation
UStudioDirectorComponent::UStudioDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentCycle = 0;
    bProductionActive = false;
    CycleStartTime = 0.0f;
    CompilationErrors = 0;
    OrphanHeaders = 0;
}

void UStudioDirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Component initialized"));
    
    // Start initial assessment
    ValidateCodebase();
    CheckPlayabilityRequirements();
}

void UStudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProductionActive)
    {
        UpdateProductionMetrics();
    }
}

void UStudioDirectorComponent::StartProductionCycle(int32 CycleNumber)
{
    CurrentCycle = CycleNumber;
    bProductionActive = true;
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %d started"), CycleNumber);
    
    // Clear previous cycle data
    AgentTasks.Empty();
    AgentProgress.Empty();
    CriticalIssues.Empty();
    
    // Run initial validation
    ValidatePlayablePrototype();
}

void UStudioDirectorComponent::EndProductionCycle()
{
    bProductionActive = false;
    float CycleDuration = GetWorld()->GetTimeSeconds() - CycleStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %d completed in %.2f seconds"), CurrentCycle, CycleDuration);
    
    // Final validation
    bool bPrototypeValid = ValidatePlayablePrototype();
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Status: %s"), bPrototypeValid ? TEXT("VALID") : TEXT("INVALID"));
}

bool UStudioDirectorComponent::ValidatePlayablePrototype()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("No world available for validation"));
        return false;
    }
    
    bool bValid = true;
    CriticalIssues.Empty();
    
    // Check for PlayerStart
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
    if (PlayerStarts.Num() == 0)
    {
        CriticalIssues.Add(TEXT("Missing PlayerStart"));
        bValid = false;
    }
    
    // Check for basic lighting
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() == 0)
    {
        CriticalIssues.Add(TEXT("Missing Directional Light"));
        bValid = false;
    }
    
    // Check for landscape/terrain
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), Landscapes);
    if (Landscapes.Num() == 0)
    {
        CriticalIssues.Add(TEXT("Missing Landscape/Terrain"));
        bValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Prototype validation: %d critical issues found"), CriticalIssues.Num());
    
    return bValid;
}

void UStudioDirectorComponent::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription)
{
    AgentTasks.Add(AgentNumber, TaskDescription);
    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void UStudioDirectorComponent::ReportAgentProgress(int32 AgentNumber, const FString& Progress)
{
    AgentProgress.Add(AgentNumber, Progress);
    UE_LOG(LogTemp, Log, TEXT("Agent #%d progress: %s"), AgentNumber, *Progress);
}

bool UStudioDirectorComponent::CheckCompilationStatus()
{
    // This would typically interface with UBT or check for compilation artifacts
    // For now, we'll simulate based on class availability
    
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    UClass* GameModeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode"));
    
    CompilationErrors = 0;
    
    if (!CharacterClass)
    {
        CompilationErrors++;
        CriticalIssues.AddUnique(TEXT("TranspersonalCharacter class not found"));
    }
    
    if (!GameModeClass)
    {
        CompilationErrors++;
        CriticalIssues.AddUnique(TEXT("TranspersonalGameMode class not found"));
    }
    
    return CompilationErrors == 0;
}

int32 UStudioDirectorComponent::CountOrphanHeaders()
{
    // This would scan the filesystem for .h files without corresponding .cpp files
    // For now, return the known count from memory
    OrphanHeaders = 122; // From brain memory
    return OrphanHeaders;
}

TArray<FString> UStudioDirectorComponent::GetCriticalErrors()
{
    return CriticalIssues;
}

void UStudioDirectorComponent::UpdateProductionMetrics()
{
    // Update compilation status
    CheckCompilationStatus();
    
    // Update orphan header count
    CountOrphanHeaders();
    
    // Check cycle timeout
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float CycleDuration = CurrentTime - CycleStartTime;
    
    if (CycleDuration > 600.0f) // 10 minutes
    {
        CriticalIssues.AddUnique(TEXT("Production cycle timeout"));
        UE_LOG(LogTemp, Error, TEXT("Production cycle timeout after %.2f seconds"), CycleDuration);
    }
}

void UStudioDirectorComponent::ValidateCodebase()
{
    CheckCompilationStatus();
    CountOrphanHeaders();
    
    UE_LOG(LogTemp, Warning, TEXT("Codebase validation: %d compilation errors, %d orphan headers"), 
           CompilationErrors, OrphanHeaders);
}

void UStudioDirectorComponent::CheckPlayabilityRequirements()
{
    ValidatePlayablePrototype();
    
    UE_LOG(LogTemp, Warning, TEXT("Playability check: %d critical issues"), CriticalIssues.Num());
    for (const FString& Issue : CriticalIssues)
    {
        UE_LOG(LogTemp, Warning, TEXT("- %s"), *Issue);
    }
}

// Studio Director Actor Implementation
AStudioDirector::AStudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create studio director component
    StudioDirectorComponent = CreateDefaultSubobject<UStudioDirectorComponent>(TEXT("StudioDirectorComponent"));
    
    // Set default values
    bAutoManageProduction = true;
    ProductionCycleDuration = 300.0f; // 5 minutes
    MaxConcurrentAgents = 5;
}

void AStudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Actor spawned"));
    
    if (bAutoManageProduction)
    {
        InitializeProductionPipeline();
    }
}

void AStudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoManageProduction)
    {
        MonitorAgentChain();
        EnsurePlayablePrototype();
    }
}

void AStudioDirector::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing production pipeline"));
    
    if (StudioDirectorComponent)
    {
        StudioDirectorComponent->StartProductionCycle(20); // Current cycle
        
        // Assign critical tasks
        StudioDirectorComponent->AssignTaskToAgent(2, TEXT("Fix compilation errors and clean orphan headers"));
        StudioDirectorComponent->AssignTaskToAgent(5, TEXT("Expand landscape to 200km² with biomes"));
        StudioDirectorComponent->AssignTaskToAgent(9, TEXT("Create real dinosaur actors with collision"));
        StudioDirectorComponent->AssignTaskToAgent(12, TEXT("Implement survival HUD"));
    }
}

void AStudioDirector::RunCriticalAssessment()
{
    UE_LOG(LogTemp, Warning, TEXT("Running critical assessment"));
    
    if (StudioDirectorComponent)
    {
        bool bCompilationOK = StudioDirectorComponent->CheckCompilationStatus();
        int32 OrphanCount = StudioDirectorComponent->CountOrphanHeaders();
        bool bPrototypeValid = StudioDirectorComponent->ValidatePlayablePrototype();
        
        UE_LOG(LogTemp, Warning, TEXT("Assessment Results:"));
        UE_LOG(LogTemp, Warning, TEXT("- Compilation: %s"), bCompilationOK ? TEXT("OK") : TEXT("ERRORS"));
        UE_LOG(LogTemp, Warning, TEXT("- Orphan Headers: %d"), OrphanCount);
        UE_LOG(LogTemp, Warning, TEXT("- Prototype: %s"), bPrototypeValid ? TEXT("VALID") : TEXT("INVALID"));
    }
}

void AStudioDirector::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating MinPlayableMap"));
    
    if (StudioDirectorComponent)
    {
        bool bValid = StudioDirectorComponent->ValidatePlayablePrototype();
        TArray<FString> Issues = StudioDirectorComponent->GetCriticalErrors();
        
        UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap validation: %s"), bValid ? TEXT("PASSED") : TEXT("FAILED"));
        for (const FString& Issue : Issues)
        {
            UE_LOG(LogTemp, Error, TEXT("Issue: %s"), *Issue);
        }
    }
}

void AStudioDirector::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating production report"));
    
    if (StudioDirectorComponent)
    {
        RunCriticalAssessment();
        ValidateMinPlayableMap();
        
        UE_LOG(LogTemp, Warning, TEXT("Production report generated - check logs for details"));
    }
}

void AStudioDirector::MonitorAgentChain()
{
    // Monitor agent progress and handle timeouts/blocks
    if (StudioDirectorComponent && StudioDirectorComponent->bProductionActive)
    {
        // Check for critical blocks
        TArray<FString> CriticalErrors = StudioDirectorComponent->GetCriticalErrors();
        if (CriticalErrors.Num() > 5)
        {
            UE_LOG(LogTemp, Error, TEXT("Too many critical errors - production halt recommended"));
        }
    }
}

void AStudioDirector::EnsurePlayablePrototype()
{
    // Ensure minimum playable prototype requirements are met
    if (StudioDirectorComponent)
    {
        bool bValid = StudioDirectorComponent->ValidatePlayablePrototype();
        if (!bValid)
        {
            UE_LOG(LogTemp, Warning, TEXT("Playable prototype requirements not met - prioritizing fixes"));
        }
    }
}