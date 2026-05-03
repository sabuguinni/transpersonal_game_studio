#include "Core_PhysicsSystemManager.h"
#include "Core_TerrainInteractionSystem.h"
#include "Core_SurvivalSystem.h"
#include "../../TranspersonalCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    bPhysicsSystemsInitialized = false;
    bPhysicsSystemsEnabled = true;
    PhysicsUpdateFrequency = 60.0f; // 60 Hz physics updates
    PhysicsTimeAccumulator = 0.0f;
    
    // Initialize subsystem pointers
    TerrainInteractionSystem = nullptr;
    SurvivalSystem = nullptr;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Create and initialize all physics subsystems
    CreateSubsystems();
    InitializePhysicsSystems();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: BeginPlay completed"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsSystemsEnabled || !bPhysicsSystemsInitialized)
    {
        return;
    }
    
    // Accumulate time for fixed physics updates
    PhysicsTimeAccumulator += DeltaTime;
    
    const float PhysicsTimestep = 1.0f / PhysicsUpdateFrequency;
    
    // Update physics systems at fixed intervals
    while (PhysicsTimeAccumulator >= PhysicsTimestep)
    {
        UpdatePhysicsSystems(PhysicsTimestep);
        PhysicsTimeAccumulator -= PhysicsTimestep;
    }
}

void UCore_PhysicsSystemManager::InitializePhysicsSystems()
{
    if (bPhysicsSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics systems already initialized"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Initializing physics systems..."));
    
    // Validate that all required systems are present
    if (!ValidatePhysicsSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Failed to validate physics systems"));
        return;
    }
    
    // Initialize terrain interaction system
    if (TerrainInteractionSystem)
    {
        // TerrainInteractionSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Terrain interaction system initialized"));
    }
    
    // Initialize survival system
    if (SurvivalSystem)
    {
        // SurvivalSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Survival system initialized"));
    }
    
    bPhysicsSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: All physics systems initialized successfully"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSystems(float DeltaTime)
{
    if (!bPhysicsSystemsInitialized)
    {
        return;
    }
    
    // Update terrain interaction system
    if (TerrainInteractionSystem)
    {
        // Process terrain interactions for all registered characters
        for (ATranspersonalCharacter* Character : RegisteredCharacters)
        {
            if (IsValid(Character))
            {
                // TerrainInteractionSystem->UpdateCharacterTerrainInteraction(Character, DeltaTime);
            }
        }
    }
    
    // Update survival system
    if (SurvivalSystem)
    {
        // Process survival mechanics for all registered characters
        for (ATranspersonalCharacter* Character : RegisteredCharacters)
        {
            if (IsValid(Character))
            {
                // SurvivalSystem->UpdateCharacterSurvival(Character, DeltaTime);
            }
        }
    }
}

void UCore_PhysicsSystemManager::RegisterCharacter(ATranspersonalCharacter* Character)
{
    if (!IsValid(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Attempted to register invalid character"));
        return;
    }
    
    if (RegisteredCharacters.Contains(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Character already registered"));
        return;
    }
    
    RegisteredCharacters.Add(Character);
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Character registered: %s"), *Character->GetName());
    
    // Notify subsystems about new character
    if (TerrainInteractionSystem)
    {
        // TerrainInteractionSystem->OnCharacterRegistered(Character);
    }
    
    if (SurvivalSystem)
    {
        // SurvivalSystem->OnCharacterRegistered(Character);
    }
}

void UCore_PhysicsSystemManager::UnregisterCharacter(ATranspersonalCharacter* Character)
{
    if (!IsValid(Character))
    {
        return;
    }
    
    if (RegisteredCharacters.Remove(Character) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Character unregistered: %s"), *Character->GetName());
        
        // Notify subsystems about character removal
        if (TerrainInteractionSystem)
        {
            // TerrainInteractionSystem->OnCharacterUnregistered(Character);
        }
        
        if (SurvivalSystem)
        {
            // SurvivalSystem->OnCharacterUnregistered(Character);
        }
    }
}

void UCore_PhysicsSystemManager::CreateSubsystems()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Creating physics subsystems..."));
    
    // Create terrain interaction system
    if (!TerrainInteractionSystem)
    {
        TerrainInteractionSystem = CreateDefaultSubobject<UCore_TerrainInteractionSystem>(TEXT("TerrainInteractionSystem"));
        if (TerrainInteractionSystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Terrain interaction system created"));
        }
    }
    
    // Create survival system
    if (!SurvivalSystem)
    {
        SurvivalSystem = CreateDefaultSubobject<UCore_SurvivalSystem>(TEXT("SurvivalSystem"));
        if (SurvivalSystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Survival system created"));
        }
    }
}

bool UCore_PhysicsSystemManager::ValidatePhysicsSystems() const
{
    bool bAllSystemsValid = true;
    
    if (!TerrainInteractionSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Terrain interaction system is null"));
        bAllSystemsValid = false;
    }
    
    if (!SurvivalSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Survival system is null"));
        bAllSystemsValid = false;
    }
    
    return bAllSystemsValid;
}