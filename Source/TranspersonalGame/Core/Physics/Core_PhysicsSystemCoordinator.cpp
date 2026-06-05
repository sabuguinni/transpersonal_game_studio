#include "Core_PhysicsSystemCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/GameInstance.h"

UCore_PhysicsSystemCoordinator::UCore_PhysicsSystemCoordinator()
{
    // Initialize default configuration
    SystemConfig = FCore_PhysicsSystemConfig();
    CurrentMetrics = FCore_PhysicsSystemMetrics();
    CurrentState = ECore_PhysicsSystemState::Uninitialized;
    
    // Initialize performance tracking
    LastFrameTime = 16.67f;
    AverageFrameTime = 16.67f;
    FrameSampleCount = 0;
    bPerformanceWarningActive = false;
    
    // Clear system references
    PhysicsIntegrationMaster = nullptr;
    PhysicsOptimizer = nullptr;
    CharacterPhysics = nullptr;
    TerrainPhysics = nullptr;
    CollisionManager = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
}

void UCore_PhysicsSystemCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Initializing physics coordination system"));
    
    SetSystemState(ECore_PhysicsSystemState::Initializing);
    
    // Initialize physics systems
    if (InitializePhysicsSystems())
    {
        SetSystemState(ECore_PhysicsSystemState::Active);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Successfully initialized"));
        
        // Set up update timers
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                MetricsUpdateTimer,
                FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemCoordinator::CheckPerformanceThresholds),
                1.0f, // Update every second
                true  // Loop
            );
            
            World->GetTimerManager().SetTimer(
                ValidationTimer,
                FTimerDelegate::CreateUObject(this, &UCore_PhysicsSystemCoordinator::LogPhysicsState),
                10.0f, // Validate every 10 seconds
                true   // Loop
            );
        }
    }
    else
    {
        SetSystemState(ECore_PhysicsSystemState::Error);
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemCoordinator: Failed to initialize physics systems"));
    }
}

void UCore_PhysicsSystemCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Shutting down physics coordination system"));
    
    ShutdownPhysicsSystems();
    SetSystemState(ECore_PhysicsSystemState::Shutdown);
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsUpdateTimer);
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Clear registered characters
    RegisteredCharacters.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsSystemCoordinator::ShouldCreateSubsystem(UObject* Outer) const
{
    return true; // Always create this subsystem
}

bool UCore_PhysicsSystemCoordinator::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Initializing core physics systems"));
    
    bool bAllSystemsInitialized = true;
    
    // Create physics subsystems
    if (!CreatePhysicsSubsystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemCoordinator: Failed to create physics subsystems"));
        bAllSystemsInitialized = false;
    }
    
    // Apply default physics configuration
    ApplyPhysicsConfig(SystemConfig);
    
    // Initialize performance metrics
    CurrentMetrics = FCore_PhysicsSystemMetrics();
    CurrentMetrics.CurrentQuality = ECore_PhysicsQuality::Medium;
    CurrentMetrics.bIsOptimizationActive = SystemConfig.bEnableAdaptiveQuality;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Physics systems initialization %s"), 
           bAllSystemsInitialized ? TEXT("SUCCESSFUL") : TEXT("FAILED"));
    
    return bAllSystemsInitialized;
}

void UCore_PhysicsSystemCoordinator::ShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Shutting down physics systems"));
    
    // Unregister all characters
    for (auto& CharacterPtr : RegisteredCharacters)
    {
        if (CharacterPtr.IsValid())
        {
            UnregisterCharacterPhysics(CharacterPtr.Get());
        }
    }
    RegisteredCharacters.Empty();
    
    // Cleanup physics subsystems
    CleanupPhysicsSubsystems();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Physics systems shutdown complete"));
}

void UCore_PhysicsSystemCoordinator::UpdatePhysicsSystems(float DeltaTime)
{
    if (CurrentState != ECore_PhysicsSystemState::Active)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Update physics subsystems if they exist
    // Note: Using raw pointers for cross-module compatibility
    // Actual subsystem updates would go here when modules are properly linked
    
    // Update registered character physics
    for (auto& CharacterPtr : RegisteredCharacters)
    {
        if (CharacterPtr.IsValid())
        {
            ACharacter* Character = CharacterPtr.Get();
            if (Character && Character->GetCharacterMovement())
            {
                // Update character-specific physics metrics
                CurrentMetrics.ActiveRigidBodies++;
            }
        }
    }
    
    // Check for performance optimization needs
    if (SystemConfig.bEnableAdaptiveQuality)
    {
        OptimizePhysicsQuality();
    }
}

void UCore_PhysicsSystemCoordinator::SetPhysicsQuality(ECore_PhysicsQuality NewQuality)
{
    if (CurrentMetrics.CurrentQuality != NewQuality)
    {
        ECore_PhysicsQuality OldQuality = CurrentMetrics.CurrentQuality;
        CurrentMetrics.CurrentQuality = NewQuality;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Physics quality changed from %d to %d"), 
               (int32)OldQuality, (int32)NewQuality);
        
        // Apply quality settings to physics subsystems
        switch (NewQuality)
        {
            case ECore_PhysicsQuality::Low:
                SystemConfig.MaxSubsteps = 2;
                SystemConfig.MaxActiveRigidBodies = 500;
                break;
            case ECore_PhysicsQuality::Medium:
                SystemConfig.MaxSubsteps = 4;
                SystemConfig.MaxActiveRigidBodies = 1000;
                break;
            case ECore_PhysicsQuality::High:
                SystemConfig.MaxSubsteps = 6;
                SystemConfig.MaxActiveRigidBodies = 1500;
                break;
            case ECore_PhysicsQuality::Ultra:
                SystemConfig.MaxSubsteps = 8;
                SystemConfig.MaxActiveRigidBodies = 2000;
                break;
            case ECore_PhysicsQuality::Adaptive:
                // Adaptive quality adjusts automatically
                break;
        }
        
        ApplyPhysicsConfig(SystemConfig);
        OnQualityChanged.Broadcast(NewQuality);
    }
}

void UCore_PhysicsSystemCoordinator::EnableAdaptiveQuality(bool bEnable)
{
    SystemConfig.bEnableAdaptiveQuality = bEnable;
    CurrentMetrics.bIsOptimizationActive = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Adaptive quality %s"), 
           bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    if (bEnable)
    {
        SetPhysicsQuality(ECore_PhysicsQuality::Adaptive);
    }
}

void UCore_PhysicsSystemCoordinator::ApplyPhysicsConfig(const FCore_PhysicsSystemConfig& NewConfig)
{
    SystemConfig = NewConfig;
    
    // Apply configuration to UE5 physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->FixedDeltaTime = SystemConfig.FixedDeltaTime;
        PhysicsSettings->MaxSubsteps = SystemConfig.MaxSubsteps;
        PhysicsSettings->MaxSubstepDeltaTime = SystemConfig.MaxSubstepDeltaTime;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Applied physics config - DeltaTime: %f, MaxSubsteps: %d"), 
               SystemConfig.FixedDeltaTime, SystemConfig.MaxSubsteps);
    }
    
    // Update metrics
    CurrentMetrics.PhysicsSubsteps = SystemConfig.MaxSubsteps;
}

bool UCore_PhysicsSystemCoordinator::RegisterCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemCoordinator: Cannot register null character"));
        return false;
    }
    
    // Check if already registered
    if (IsCharacterPhysicsRegistered(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Character %s already registered"), 
               *Character->GetName());
        return true;
    }
    
    // Add to registered characters
    RegisteredCharacters.Add(Character);
    
    // Configure character physics
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        // Apply survival-appropriate movement settings
        MovementComp->MaxWalkSpeed = 600.0f; // Realistic human walking speed
        MovementComp->MaxAcceleration = 2048.0f;
        MovementComp->BrakingDecelerationWalking = 2000.0f;
        MovementComp->JumpZVelocity = 420.0f;
        MovementComp->AirControl = 0.35f;
        
        // Enable physics interaction
        MovementComp->bEnablePhysicsInteraction = true;
        MovementComp->bTouchForceScaledToMass = true;
        MovementComp->TouchForceFactor = 1.0f;
    }
    
    // Configure collision
    if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleComp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
        CapsuleComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Successfully registered character %s"), 
           *Character->GetName());
    
    return true;
}

void UCore_PhysicsSystemCoordinator::UnregisterCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    // Remove from registered characters
    RegisteredCharacters.RemoveAll([Character](const TWeakObjectPtr<ACharacter>& CharacterPtr)
    {
        return CharacterPtr.Get() == Character;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Unregistered character %s"), 
           *Character->GetName());
}

bool UCore_PhysicsSystemCoordinator::IsCharacterPhysicsRegistered(ACharacter* Character) const
{
    if (!Character)
    {
        return false;
    }
    
    return RegisteredCharacters.ContainsByPredicate([Character](const TWeakObjectPtr<ACharacter>& CharacterPtr)
    {
        return CharacterPtr.Get() == Character;
    });
}

bool UCore_PhysicsSystemCoordinator::ValidatePhysicsSystems()
{
    bool bAllSystemsValid = true;
    
    // Validate system state
    if (CurrentState != ECore_PhysicsSystemState::Active)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemCoordinator: System not in active state"));
        bAllSystemsValid = false;
    }
    
    // Validate registered characters
    int32 ValidCharacters = 0;
    for (auto& CharacterPtr : RegisteredCharacters)
    {
        if (CharacterPtr.IsValid())
        {
            ValidCharacters++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Validation - %d valid characters registered"), 
           ValidCharacters);
    
    // Validate physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        if (PhysicsSettings->FixedDeltaTime <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemCoordinator: Invalid physics delta time"));
            bAllSystemsValid = false;
        }
    }
    
    return bAllSystemsValid;
}

TArray<FString> UCore_PhysicsSystemCoordinator::GetSystemDiagnostics()
{
    TArray<FString> Diagnostics;
    
    Diagnostics.Add(FString::Printf(TEXT("System State: %d"), (int32)CurrentState));
    Diagnostics.Add(FString::Printf(TEXT("Physics Quality: %d"), (int32)CurrentMetrics.CurrentQuality));
    Diagnostics.Add(FString::Printf(TEXT("Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime));
    Diagnostics.Add(FString::Printf(TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies));
    Diagnostics.Add(FString::Printf(TEXT("Registered Characters: %d"), RegisteredCharacters.Num()));
    Diagnostics.Add(FString::Printf(TEXT("Adaptive Quality: %s"), CurrentMetrics.bIsOptimizationActive ? TEXT("ON") : TEXT("OFF")));
    Diagnostics.Add(FString::Printf(TEXT("Physics Substeps: %d"), CurrentMetrics.PhysicsSubsteps));
    Diagnostics.Add(FString::Printf(TEXT("Average Frame Time: %.2f ms"), AverageFrameTime));
    
    return Diagnostics;
}

void UCore_PhysicsSystemCoordinator::LogPhysicsState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEM STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("State: %d"), (int32)CurrentState);
    UE_LOG(LogTemp, Warning, TEXT("Quality: %d"), (int32)CurrentMetrics.CurrentQuality);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Registered Characters: %d"), RegisteredCharacters.Num());
    UE_LOG(LogTemp, Warning, TEXT("Adaptive Quality: %s"), CurrentMetrics.bIsOptimizationActive ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Warning, TEXT("=== END PHYSICS STATE ==="));
}

void UCore_PhysicsSystemCoordinator::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time tracking
    LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    CurrentMetrics.PhysicsFrameTime = LastFrameTime;
    
    // Update running average
    if (FrameSampleCount < 60) // First 60 frames
    {
        AverageFrameTime = ((AverageFrameTime * FrameSampleCount) + LastFrameTime) / (FrameSampleCount + 1);
        FrameSampleCount++;
    }
    else
    {
        // Rolling average for subsequent frames
        AverageFrameTime = (AverageFrameTime * 0.95f) + (LastFrameTime * 0.05f);
    }
    
    // Reset per-frame counters
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.ActiveCollisionShapes = 0;
}

void UCore_PhysicsSystemCoordinator::CheckPerformanceThresholds()
{
    // Check if frame time exceeds target
    bool bPerformanceIssue = AverageFrameTime > SystemConfig.TargetFrameTime;
    
    if (bPerformanceIssue && !bPerformanceWarningActive)
    {
        bPerformanceWarningActive = true;
        OnPerformanceWarning.Broadcast(AverageFrameTime);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Performance warning - Frame time: %.2f ms"), 
               AverageFrameTime);
    }
    else if (!bPerformanceIssue && bPerformanceWarningActive)
    {
        bPerformanceWarningActive = false;
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Performance recovered - Frame time: %.2f ms"), 
               AverageFrameTime);
    }
}

void UCore_PhysicsSystemCoordinator::OptimizePhysicsQuality()
{
    if (CurrentMetrics.CurrentQuality == ECore_PhysicsQuality::Adaptive)
    {
        // Determine optimal quality based on performance
        if (AverageFrameTime > SystemConfig.TargetFrameTime * 1.5f)
        {
            SetPhysicsQuality(ECore_PhysicsQuality::Low);
        }
        else if (AverageFrameTime > SystemConfig.TargetFrameTime * 1.2f)
        {
            SetPhysicsQuality(ECore_PhysicsQuality::Medium);
        }
        else if (AverageFrameTime < SystemConfig.TargetFrameTime * 0.8f)
        {
            SetPhysicsQuality(ECore_PhysicsQuality::High);
        }
    }
}

bool UCore_PhysicsSystemCoordinator::CreatePhysicsSubsystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Creating physics subsystems"));
    
    // Note: In a real implementation, these would create actual subsystem instances
    // For now, we're setting up the coordination framework
    
    bool bSuccess = true;
    
    // Initialize subsystem pointers to nullptr (cross-module compatibility)
    PhysicsIntegrationMaster = nullptr;
    PhysicsOptimizer = nullptr;
    CharacterPhysics = nullptr;
    TerrainPhysics = nullptr;
    CollisionManager = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Physics subsystems framework ready"));
    
    return bSuccess;
}

void UCore_PhysicsSystemCoordinator::CleanupPhysicsSubsystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Cleaning up physics subsystems"));
    
    // Clear subsystem references
    PhysicsIntegrationMaster = nullptr;
    PhysicsOptimizer = nullptr;
    CharacterPhysics = nullptr;
    TerrainPhysics = nullptr;
    CollisionManager = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: Physics subsystems cleanup complete"));
}

void UCore_PhysicsSystemCoordinator::SetSystemState(ECore_PhysicsSystemState NewState)
{
    if (CurrentState != NewState)
    {
        ECore_PhysicsSystemState OldState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemCoordinator: State changed from %d to %d"), 
               (int32)OldState, (int32)NewState);
        
        OnSystemStateChanged.Broadcast(NewState);
    }
}