#include "EngineArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UEngineArchitecture::UEngineArchitecture()
{
    bArchitectureValid = false;
    LastValidationTime = 0.0f;
    AverageFrameTime = 0.0f;
    PeakActorCount = 0;

    // Initialize core system modules
    CoreSystemModules.Add(TEXT("TranspersonalGame"));
    CoreSystemModules.Add(TEXT("Core"));
    CoreSystemModules.Add(TEXT("Characters"));
    CoreSystemModules.Add(TEXT("WorldGeneration"));

    // Initialize gameplay system modules
    GameplaySystemModules.Add(TEXT("AI"));
    GameplaySystemModules.Add(TEXT("Combat"));
    GameplaySystemModules.Add(TEXT("Quest"));
    GameplaySystemModules.Add(TEXT("Environment"));

    // Initialize rendering system modules
    RenderingSystemModules.Add(TEXT("VFX"));
    RenderingSystemModules.Add(TEXT("Lighting"));
    RenderingSystemModules.Add(TEXT("Audio"));
}

void UEngineArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Subsystem initialized"));

    // Register core systems with priorities
    RegisterSystemModule(TEXT("Core"), 100);
    RegisterSystemModule(TEXT("Characters"), 90);
    RegisterSystemModule(TEXT("WorldGeneration"), 80);
    RegisterSystemModule(TEXT("AI"), 70);
    RegisterSystemModule(TEXT("Combat"), 60);
    RegisterSystemModule(TEXT("Environment"), 50);
    RegisterSystemModule(TEXT("VFX"), 40);
    RegisterSystemModule(TEXT("Audio"), 30);

    // Perform initial validation
    ValidateSystemDependencies();
}

void UEngineArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Subsystem deinitialized"));
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

bool UEngineArchitecture::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Validating system dependencies"));

    bool bAllValid = true;
    LastValidationTime = FPlatformTime::Seconds();

    // Validate core systems
    bAllValid &= ValidateWorldGeneration();
    bAllValid &= ValidateCharacterSystems();
    bAllValid &= ValidateDinosaurSystems();
    bAllValid &= ValidateRenderingPipeline();

    bArchitectureValid = bAllValid;

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Validation complete - %s"), 
           bAllValid ? TEXT("PASSED") : TEXT("FAILED"));

    return bAllValid;
}

bool UEngineArchitecture::ValidateModuleIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Validating module integrity"));

    // Check if core modules are registered
    bool bCoreValid = true;
    for (const FString& CoreModule : CoreSystemModules)
    {
        if (!RegisteredSystems.Contains(CoreModule))
        {
            UE_LOG(LogTemp, Error, TEXT("EngineArchitecture: Core module missing: %s"), *CoreModule);
            bCoreValid = false;
        }
    }

    // Check world state
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecture: No valid world found"));
        bCoreValid = false;
    }

    return bCoreValid;
}

void UEngineArchitecture::RegisterSystemModule(const FString& ModuleName, int32 Priority)
{
    RegisteredSystems.Add(ModuleName, Priority);
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Registered module %s with priority %d"), 
           *ModuleName, Priority);
}

bool UEngineArchitecture::CheckCompilationStatus()
{
    // Check if we can access basic UE5 classes
    bool bCompilationValid = true;

    // Test core class access
    UClass* ActorClass = AActor::StaticClass();
    UClass* PawnClass = APawn::StaticClass();
    UClass* CharacterClass = ACharacter::StaticClass();

    if (!ActorClass || !PawnClass || !CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecture: Core UE5 classes not accessible"));
        bCompilationValid = false;
    }

    // Test TranspersonalGame module classes
    UClass* TranspersonalCharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    UClass* TranspersonalGameModeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode"));

    if (!TranspersonalCharacterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: TranspersonalCharacter class not found"));
    }

    if (!TranspersonalGameModeClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: TranspersonalGameMode class not found"));
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Compilation status check complete"));
    return bCompilationValid;
}

float UEngineArchitecture::GetCurrentFrameTime() const
{
    return FApp::GetDeltaTime();
}

int32 UEngineArchitecture::GetActiveActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    return ActorCount;
}

void UEngineArchitecture::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Valid: %s"), bArchitectureValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Current Frame Time: %.3f ms"), GetCurrentFrameTime() * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Active Actor Count: %d"), GetActiveActorCount());
    UE_LOG(LogTemp, Warning, TEXT("Last Validation: %.2f seconds ago"), 
           FPlatformTime::Seconds() - LastValidationTime);

    // Log registered systems
    for (const auto& System : RegisteredSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("System: %s (Priority: %d)"), *System.Key, System.Value);
    }

    UpdatePerformanceMetrics();
}

bool UEngineArchitecture::ValidateWorldGeneration()
{
    // Check if world generation components exist
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Look for terrain/landscape actors
    bool bHasLandscape = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
        {
            bHasLandscape = true;
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: World generation validation - Landscape: %s"),
           bHasLandscape ? TEXT("FOUND") : TEXT("MISSING"));

    return true; // Non-blocking for now
}

bool UEngineArchitecture::ValidateCharacterSystems()
{
    // Check if character systems are functional
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Look for player start
    bool bHasPlayerStart = false;
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        bHasPlayerStart = true;
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Character systems validation - PlayerStart: %s"),
           bHasPlayerStart ? TEXT("FOUND") : TEXT("MISSING"));

    return bHasPlayerStart;
}

bool UEngineArchitecture::ValidateDinosaurSystems()
{
    // Check if dinosaur actors exist in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    int32 DinosaurCount = 0;
    for (TActorIterator<APawn> PawnItr(World); PawnItr; ++PawnItr)
    {
        APawn* Pawn = *PawnItr;
        if (Pawn && Pawn->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Dinosaur systems validation - Found %d dinosaur actors"),
           DinosaurCount);

    return true; // Non-blocking for now
}

bool UEngineArchitecture::ValidateRenderingPipeline()
{
    // Check if essential rendering actors exist
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    bool bHasDirectionalLight = false;
    bool bHasSkyAtmosphere = false;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("DirectionalLight")))
        {
            bHasDirectionalLight = true;
        }
        else if (ClassName.Contains(TEXT("SkyAtmosphere")))
        {
            bHasSkyAtmosphere = true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecture: Rendering pipeline validation - DirectionalLight: %s, SkyAtmosphere: %s"),
           bHasDirectionalLight ? TEXT("OK") : TEXT("MISSING"),
           bHasSkyAtmosphere ? TEXT("OK") : TEXT("MISSING"));

    return bHasDirectionalLight && bHasSkyAtmosphere;
}

void UEngineArchitecture::UpdatePerformanceMetrics()
{
    float CurrentFrameTime = GetCurrentFrameTime();
    int32 CurrentActorCount = GetActiveActorCount();

    // Update running averages
    if (AverageFrameTime == 0.0f)
    {
        AverageFrameTime = CurrentFrameTime;
    }
    else
    {
        AverageFrameTime = (AverageFrameTime * 0.9f) + (CurrentFrameTime * 0.1f);
    }

    if (CurrentActorCount > PeakActorCount)
    {
        PeakActorCount = CurrentActorCount;
    }
}