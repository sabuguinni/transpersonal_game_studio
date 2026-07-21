#include "Build_CriticalSystemIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"

ABuild_CriticalSystemIntegrator::ABuild_CriticalSystemIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create mesh component for visualization
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    // Set default mesh (sphere for system integration)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
        MeshComponent->SetWorldScale3D(FVector(1.5f, 1.5f, 1.5f));
    }
    
    // Initialize integration state
    IntegrationState = EBuild_SystemIntegrationState::Idle;
    TotalSystemsToIntegrate = 0;
    SystemsIntegrated = 0;
    SystemsWithErrors = 0;
    CriticalSystemFailures = 0;
    
    // Integration timing
    IntegrationStartTime = 0.0f;
    CurrentSystemIntegrationTime = 0.0f;
    TotalIntegrationTime = 0.0f;
    MaxSystemIntegrationTime = 30.0f; // 30 seconds per system max
    
    // Quality metrics
    SystemIntegrityScore = 0.0f;
    CrossSystemCompatibility = 0.0f;
    PerformanceImpactScore = 0.0f;
    
    bIntegrationInProgress = false;
    bAllSystemsIntegrated = false;
    bCriticalFailureDetected = false;
    
    // Initialize system lists
    CoreSystems.Empty();
    GameplaySystems.Empty();
    RenderingSystems.Empty();
    AudioSystems.Empty();
    NetworkingSystems.Empty();
}

void ABuild_CriticalSystemIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemIntegrator: Starting critical system integration"));
    
    // Initialize system lists
    InitializeSystemLists();
    
    // Start integration process
    StartSystemIntegration();
}

void ABuild_CriticalSystemIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIntegrationInProgress)
    {
        UpdateIntegrationProgress(DeltaTime);
        
        // Check for system timeout
        if (CurrentSystemIntegrationTime > MaxSystemIntegrationTime)
        {
            UE_LOG(LogTemp, Error, TEXT("System integration timeout - skipping current system"));
            HandleSystemTimeout();
        }
    }
}

void ABuild_CriticalSystemIntegrator::InitializeSystemLists()
{
    // Core Systems (highest priority)
    CoreSystems.Add(TEXT("TranspersonalGameState"));
    CoreSystems.Add(TEXT("TranspersonalCharacter"));
    CoreSystems.Add(TEXT("PCGWorldGenerator"));
    CoreSystems.Add(TEXT("FoliageManager"));
    CoreSystems.Add(TEXT("BuildIntegrationManager"));
    
    // Gameplay Systems
    GameplaySystems.Add(TEXT("CrowdSimulationManager"));
    GameplaySystems.Add(TEXT("ProceduralWorldManager"));
    GameplaySystems.Add(TEXT("QuestManager"));
    GameplaySystems.Add(TEXT("NPCBehaviorManager"));
    GameplaySystems.Add(TEXT("CombatSystem"));
    
    // Rendering Systems
    RenderingSystems.Add(TEXT("LightingManager"));
    RenderingSystems.Add(TEXT("VFXManager"));
    RenderingSystems.Add(TEXT("MaterialManager"));
    RenderingSystems.Add(TEXT("EnvironmentRenderer"));
    
    // Audio Systems
    AudioSystems.Add(TEXT("AudioManager"));
    AudioSystems.Add(TEXT("MusicSystem"));
    AudioSystems.Add(TEXT("SoundEffectManager"));
    
    // Networking Systems (if applicable)
    NetworkingSystems.Add(TEXT("NetworkManager"));
    NetworkingSystems.Add(TEXT("ReplicationSystem"));
    
    TotalSystemsToIntegrate = CoreSystems.Num() + GameplaySystems.Num() + 
                             RenderingSystems.Num() + AudioSystems.Num() + 
                             NetworkingSystems.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d systems for integration"), TotalSystemsToIntegrate);
}

void ABuild_CriticalSystemIntegrator::StartSystemIntegration()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration already in progress"));
        return;
    }
    
    bIntegrationInProgress = true;
    IntegrationStartTime = GetWorld()->GetTimeSeconds();
    IntegrationState = EBuild_SystemIntegrationState::IntegratingCore;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting critical system integration"));
    
    // Start with core systems
    IntegrateSystemCategory(CoreSystems, TEXT("Core Systems"));
}

void ABuild_CriticalSystemIntegrator::IntegrateSystemCategory(const TArray<FString>& Systems, const FString& CategoryName)
{
    UE_LOG(LogTemp, Warning, TEXT("Integrating %s (%d systems)"), *CategoryName, Systems.Num());
    
    for (const FString& SystemName : Systems)
    {
        IntegrateIndividualSystem(SystemName);
    }
    
    // Move to next category
    AdvanceToNextCategory();
}

void ABuild_CriticalSystemIntegrator::IntegrateIndividualSystem(const FString& SystemName)
{
    CurrentSystemIntegrationTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Integrating system: %s"), *SystemName);
    
    // Simulate system integration (in real implementation, this would:
    // 1. Load the system class
    // 2. Check dependencies
    // 3. Initialize the system
    // 4. Test basic functionality
    // 5. Validate integration with other systems)
    
    bool bSystemIntegrationSuccess = ValidateSystemIntegration(SystemName);
    
    if (bSystemIntegrationSuccess)
    {
        SystemsIntegrated++;
        UE_LOG(LogTemp, Log, TEXT("✓ System %s integrated successfully"), *SystemName);
    }
    else
    {
        SystemsWithErrors++;
        UE_LOG(LogTemp, Error, TEXT("✗ System %s integration failed"), *SystemName);
        
        // Check if this is a critical system
        if (CoreSystems.Contains(SystemName))
        {
            CriticalSystemFailures++;
            bCriticalFailureDetected = true;
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Core system %s failed to integrate"), *SystemName);
        }
    }
}

bool ABuild_CriticalSystemIntegrator::ValidateSystemIntegration(const FString& SystemName)
{
    // Simulate system validation
    // In real implementation, this would perform actual system checks
    
    // Core systems have higher success rate
    float SuccessRate = CoreSystems.Contains(SystemName) ? 0.95f : 0.85f;
    
    // Random success based on system reliability
    return FMath::RandRange(0.0f, 1.0f) < SuccessRate;
}

void ABuild_CriticalSystemIntegrator::AdvanceToNextCategory()
{
    switch (IntegrationState)
    {
        case EBuild_SystemIntegrationState::IntegratingCore:
            IntegrationState = EBuild_SystemIntegrationState::IntegratingGameplay;
            IntegrateSystemCategory(GameplaySystems, TEXT("Gameplay Systems"));
            break;
            
        case EBuild_SystemIntegrationState::IntegratingGameplay:
            IntegrationState = EBuild_SystemIntegrationState::IntegratingRendering;
            IntegrateSystemCategory(RenderingSystems, TEXT("Rendering Systems"));
            break;
            
        case EBuild_SystemIntegrationState::IntegratingRendering:
            IntegrationState = EBuild_SystemIntegrationState::IntegratingAudio;
            IntegrateSystemCategory(AudioSystems, TEXT("Audio Systems"));
            break;
            
        case EBuild_SystemIntegrationState::IntegratingAudio:
            IntegrationState = EBuild_SystemIntegrationState::IntegratingNetworking;
            IntegrateSystemCategory(NetworkingSystems, TEXT("Networking Systems"));
            break;
            
        case EBuild_SystemIntegrationState::IntegratingNetworking:
            IntegrationState = EBuild_SystemIntegrationState::ValidatingIntegration;
            ValidateOverallIntegration();
            break;
            
        case EBuild_SystemIntegrationState::ValidatingIntegration:
            CompleteIntegration();
            break;
            
        default:
            break;
    }
}

void ABuild_CriticalSystemIntegrator::ValidateOverallIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating overall system integration"));
    
    // Calculate integration metrics
    float IntegrationSuccessRate = static_cast<float>(SystemsIntegrated) / static_cast<float>(TotalSystemsToIntegrate);
    SystemIntegrityScore = IntegrationSuccessRate;
    
    // Calculate cross-system compatibility
    CrossSystemCompatibility = bCriticalFailureDetected ? 0.3f : 0.9f;
    
    // Calculate performance impact
    PerformanceImpactScore = FMath::Clamp(1.0f - (static_cast<float>(SystemsWithErrors) * 0.1f), 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("  System Integrity: %.2f"), SystemIntegrityScore);
    UE_LOG(LogTemp, Warning, TEXT("  Cross-System Compatibility: %.2f"), CrossSystemCompatibility);
    UE_LOG(LogTemp, Warning, TEXT("  Performance Impact: %.2f"), PerformanceImpactScore);
}

void ABuild_CriticalSystemIntegrator::CompleteIntegration()
{
    bIntegrationInProgress = false;
    bAllSystemsIntegrated = true;
    IntegrationState = EBuild_SystemIntegrationState::Complete;
    TotalIntegrationTime = GetWorld()->GetTimeSeconds() - IntegrationStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Critical system integration complete!"));
    GenerateIntegrationReport();
}

void ABuild_CriticalSystemIntegrator::HandleSystemTimeout()
{
    SystemsWithErrors++;
    CurrentSystemIntegrationTime = 0.0f;
    
    UE_LOG(LogTemp, Error, TEXT("System integration timeout - marking as failed"));
}

void ABuild_CriticalSystemIntegrator::UpdateIntegrationProgress(float DeltaTime)
{
    CurrentSystemIntegrationTime += DeltaTime;
    TotalIntegrationTime += DeltaTime;
    
    // Update visual representation
    if (MeshComponent)
    {
        float Progress = GetIntegrationProgress();
        FLinearColor ProgressColor = bCriticalFailureDetected ? 
            FLinearColor::Red : 
            FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Green, Progress);
        
        // In real implementation, you would set material parameters here
    }
}

void ABuild_CriticalSystemIntegrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL SYSTEM INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), TotalSystemsToIntegrate);
    UE_LOG(LogTemp, Warning, TEXT("Successfully Integrated: %d"), SystemsIntegrated);
    UE_LOG(LogTemp, Warning, TEXT("Systems with Errors: %d"), SystemsWithErrors);
    UE_LOG(LogTemp, Warning, TEXT("Critical Failures: %d"), CriticalSystemFailures);
    UE_LOG(LogTemp, Warning, TEXT("Integration Time: %.2f seconds"), TotalIntegrationTime);
    UE_LOG(LogTemp, Warning, TEXT("System Integrity Score: %.2f"), SystemIntegrityScore);
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Compatibility: %.2f"), CrossSystemCompatibility);
    UE_LOG(LogTemp, Warning, TEXT("Performance Impact Score: %.2f"), PerformanceImpactScore);
    UE_LOG(LogTemp, Warning, TEXT("Critical Failure Detected: %s"), bCriticalFailureDetected ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Integration Status: %s"), IsIntegrationSuccessful() ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("=========================================="));
}

float ABuild_CriticalSystemIntegrator::GetIntegrationProgress() const
{
    if (TotalSystemsToIntegrate == 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(SystemsIntegrated) / static_cast<float>(TotalSystemsToIntegrate);
}

bool ABuild_CriticalSystemIntegrator::IsIntegrationSuccessful() const
{
    return bAllSystemsIntegrated && 
           !bCriticalFailureDetected && 
           SystemIntegrityScore >= 0.8f &&
           CrossSystemCompatibility >= 0.7f;
}

FString ABuild_CriticalSystemIntegrator::GetCurrentIntegrationPhase() const
{
    switch (IntegrationState)
    {
        case EBuild_SystemIntegrationState::Idle:
            return TEXT("Idle");
        case EBuild_SystemIntegrationState::IntegratingCore:
            return TEXT("Integrating Core Systems");
        case EBuild_SystemIntegrationState::IntegratingGameplay:
            return TEXT("Integrating Gameplay Systems");
        case EBuild_SystemIntegrationState::IntegratingRendering:
            return TEXT("Integrating Rendering Systems");
        case EBuild_SystemIntegrationState::IntegratingAudio:
            return TEXT("Integrating Audio Systems");
        case EBuild_SystemIntegrationState::IntegratingNetworking:
            return TEXT("Integrating Networking Systems");
        case EBuild_SystemIntegrationState::ValidatingIntegration:
            return TEXT("Validating Overall Integration");
        case EBuild_SystemIntegrationState::Complete:
            return TEXT("Integration Complete");
        default:
            return TEXT("Unknown");
    }
}