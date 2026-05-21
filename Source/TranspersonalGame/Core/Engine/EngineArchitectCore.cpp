#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/GameModeBase.h"

// UEngineArchitectSubsystem Implementation

void UEngineArchitectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Subsystem Initializing..."));
    
    // Initialize default configuration
    Config = FEng_ArchitectureConfig();
    ValidationState = EEng_ValidationState::Unknown;
    
    // Initialize module dependencies
    InitializeModuleDependencies();
    
    // Perform initial validation
    ValidateArchitecture();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Subsystem Initialized"));
}

void UEngineArchitectSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Subsystem Deinitializing..."));
    
    ModuleDependencies.Empty();
    ValidationState = EEng_ValidationState::Unknown;
    
    Super::Deinitialize();
}

bool UEngineArchitectSubsystem::ValidateArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Starting architecture validation"));
    
    ValidationState = EEng_ValidationState::Validating;
    
    bool bAllValidationsPassed = true;
    
    // Validate modules
    if (!ValidateModules())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Module validation failed"));
        bAllValidationsPassed = false;
    }
    
    // Validate performance
    if (!ValidatePerformance())
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performance validation failed"));
        bAllValidationsPassed = false;
    }
    
    // Validate world structure
    if (!ValidateWorldStructure())
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: World structure validation failed"));
        bAllValidationsPassed = false;
    }
    
    ValidationState = bAllValidationsPassed ? EEng_ValidationState::Passed : EEng_ValidationState::Failed;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validation complete - State: %s"), 
           ValidationState == EEng_ValidationState::Passed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bAllValidationsPassed;
}

void UEngineArchitectSubsystem::SetArchitectureConfig(const FEng_ArchitectureConfig& NewConfig)
{
    Config = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Configuration updated"));
    
    // Re-validate with new config
    ValidateArchitecture();
}

bool UEngineArchitectSubsystem::RegisterModule(const FString& ModuleName, const FString& Version)
{
    // Check if module already exists
    for (FEng_ModuleDependency& Dependency : ModuleDependencies)
    {
        if (Dependency.ModuleName == ModuleName)
        {
            Dependency.bIsLoaded = true;
            Dependency.Version = Version;
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module %s updated to version %s"), *ModuleName, *Version);
            return true;
        }
    }
    
    // Add new module
    FEng_ModuleDependency NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Version = Version;
    NewModule.bIsLoaded = true;
    NewModule.bIsRequired = false; // Dynamically registered modules are optional
    
    ModuleDependencies.Add(NewModule);
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module %s registered with version %s"), *ModuleName, *Version);
    
    return true;
}

bool UEngineArchitectSubsystem::IsModuleLoaded(const FString& ModuleName) const
{
    for (const FEng_ModuleDependency& Dependency : ModuleDependencies)
    {
        if (Dependency.ModuleName == ModuleName)
        {
            return Dependency.bIsLoaded;
        }
    }
    return false;
}

TArray<FEng_ModuleDependency> UEngineArchitectSubsystem::GetModuleDependencies() const
{
    return ModuleDependencies;
}

float UEngineArchitectSubsystem::GetCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetClient()->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

int32 UEngineArchitectSubsystem::GetActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return 0;
}

bool UEngineArchitectSubsystem::IsPerformanceWithinBudget() const
{
    float CurrentFPS = GetCurrentFrameRate();
    int32 ActorCount = GetActorCount();
    
    bool bFrameRateOK = CurrentFPS >= (Config.TargetFrameRate * 0.8f); // 80% tolerance
    bool bActorCountOK = ActorCount <= (Config.MaxActorsPerBiome * Config.BiomeCount);
    
    return bFrameRateOK && bActorCountOK;
}

void UEngineArchitectSubsystem::InitializeModuleDependencies()
{
    // Core required modules
    ModuleDependencies.Empty();
    
    FEng_ModuleDependency CoreModule;
    CoreModule.ModuleName = TEXT("TranspersonalGame");
    CoreModule.bIsRequired = true;
    CoreModule.bIsLoaded = true;
    CoreModule.Version = TEXT("1.0.0");
    ModuleDependencies.Add(CoreModule);
    
    FEng_ModuleDependency EngineModule;
    EngineModule.ModuleName = TEXT("Engine");
    EngineModule.bIsRequired = true;
    EngineModule.bIsLoaded = true;
    EngineModule.Version = TEXT("5.5.0");
    ModuleDependencies.Add(EngineModule);
    
    FEng_ModuleDependency CoreUObjectModule;
    CoreUObjectModule.ModuleName = TEXT("CoreUObject");
    CoreUObjectModule.bIsRequired = true;
    CoreUObjectModule.bIsLoaded = true;
    CoreUObjectModule.Version = TEXT("5.5.0");
    ModuleDependencies.Add(CoreUObjectModule);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initialized %d module dependencies"), ModuleDependencies.Num());
}

bool UEngineArchitectSubsystem::ValidateModules()
{
    bool bAllModulesValid = true;
    
    for (FEng_ModuleDependency& Dependency : ModuleDependencies)
    {
        if (Dependency.bIsRequired && !Dependency.bIsLoaded)
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Required module %s is not loaded"), *Dependency.ModuleName);
            bAllModulesValid = false;
        }
    }
    
    return bAllModulesValid;
}

bool UEngineArchitectSubsystem::ValidatePerformance()
{
    LastFrameRate = GetCurrentFrameRate();
    int32 ActorCount = GetActorCount();
    
    bool bPerformanceOK = IsPerformanceWithinBudget();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performance - FPS: %.1f, Actors: %d, Budget OK: %s"), 
           LastFrameRate, ActorCount, bPerformanceOK ? TEXT("YES") : TEXT("NO"));
    
    return bPerformanceOK;
}

bool UEngineArchitectSubsystem::ValidateWorldStructure()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: No valid world found"));
        return false;
    }
    
    // Check for basic world components
    bool bHasGameMode = World->GetAuthGameMode() != nullptr;
    bool bHasWorldSettings = World->GetWorldSettings() != nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: World validation - GameMode: %s, WorldSettings: %s"),
           bHasGameMode ? TEXT("YES") : TEXT("NO"),
           bHasWorldSettings ? TEXT("YES") : TEXT("NO"));
    
    return bHasGameMode && bHasWorldSettings;
}

// AEngineValidationActor Implementation

AEngineValidationActor::AEngineValidationActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    bPerformContinuousValidation = true;
    ValidationInterval = 5.0f;
    CurrentValidationState = EEng_ValidationState::Unknown;
}

void AEngineValidationActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to Engine Architect Subsystem
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        ArchitectSubsystem = GameInstance->GetSubsystem<UEngineArchitectSubsystem>();
    }
    
    if (!ArchitectSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Validation Actor: Could not find Engine Architect Subsystem"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Validation Actor: Started validation monitoring"));
    
    // Perform initial validation
    PerformValidation();
}

void AEngineValidationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bPerformContinuousValidation && ArchitectSubsystem)
    {
        LastValidationTime += DeltaTime;
        
        if (LastValidationTime >= ValidationInterval)
        {
            PerformValidation();
            LastValidationTime = 0.0f;
        }
    }
}

void AEngineValidationActor::PerformValidation()
{
    if (!ArchitectSubsystem)
    {
        CurrentValidationState = EEng_ValidationState::Critical;
        return;
    }
    
    EEng_ValidationState PreviousState = CurrentValidationState;
    
    bool bValidationPassed = ArchitectSubsystem->ValidateArchitecture();
    CurrentValidationState = ArchitectSubsystem->GetValidationState();
    
    if (PreviousState != CurrentValidationState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Validation Actor: State changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentValidationState);
        
        OnValidationStateChanged(CurrentValidationState);
    }
}

// UEnginePerformanceComponent Implementation

UEnginePerformanceComponent::UEnginePerformanceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTrackPerformance = true;
    
    AverageTickTime = 0.0f;
    MaxTickTime = 0.0f;
    TickCount = 0;
    TotalTickTime = 0.0f;
    LastTickStartTime = 0.0;
}

void UEnginePerformanceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Performance Component: Started performance tracking for %s"), 
           *GetOwner()->GetName());
    
    ResetPerformanceStats();
}

void UEnginePerformanceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (bTrackPerformance)
    {
        double TickStartTime = FPlatformTime::Seconds();
        
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        
        double TickEndTime = FPlatformTime::Seconds();
        float TickDuration = static_cast<float>(TickEndTime - TickStartTime);
        
        // Update statistics
        TickCount++;
        TotalTickTime += TickDuration;
        AverageTickTime = TotalTickTime / TickCount;
        
        if (TickDuration > MaxTickTime)
        {
            MaxTickTime = TickDuration;
        }
        
        LastTickStartTime = TickStartTime;
    }
    else
    {
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    }
}

void UEnginePerformanceComponent::ResetPerformanceStats()
{
    TickCount = 0;
    TotalTickTime = 0.0f;
    AverageTickTime = 0.0f;
    MaxTickTime = 0.0f;
    LastTickStartTime = 0.0;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Performance Component: Performance stats reset for %s"), 
           *GetOwner()->GetName());
}

float UEnginePerformanceComponent::GetPerformanceScore() const
{
    if (TickCount == 0 || AverageTickTime <= 0.0f)
    {
        return 100.0f; // Perfect score if no data or no time consumed
    }
    
    // Performance score based on tick efficiency
    // Lower average tick time = higher score
    float BaseScore = 100.0f;
    float TimePenalty = AverageTickTime * 10000.0f; // Convert to milliseconds and scale
    
    float Score = FMath::Max(0.0f, BaseScore - TimePenalty);
    
    return Score;
}