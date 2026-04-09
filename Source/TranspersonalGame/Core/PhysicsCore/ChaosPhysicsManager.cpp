// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ChaosPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogChaosPhysicsManager);

void UChaosPhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos Physics Manager initialized"));
    
    // Initialize default configuration
    CurrentConfig = FChaosPhysicsConfig();
    
    // Initialize default physics profiles
    InitializeDefaultProfiles();
}

void UChaosPhysicsManager::Deinitialize()
{
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsLODTimerHandle);
    }
    
    Super::Deinitialize();
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos Physics Manager deinitialized"));
}

void UChaosPhysicsManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    // Cache physics scene reference
    CachedPhysicsScene = InWorld.GetPhysicsScene();
    
    // Initialize Chaos Physics with optimized settings
    InitializeChaosPhysics();
    
    // Set up physics LOD system
    SetupPhysicsLOD();
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos Physics Manager world begin play"));
}

UChaosPhysicsManager* UChaosPhysicsManager::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UChaosPhysicsManager>();
    }
    return nullptr;
}

void UChaosPhysicsManager::InitializeChaosPhysics()
{
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Initializing Chaos Physics system"));
    
    // Configure for large-scale simulation
    ConfigureLargeScaleSimulation();
    
    // Optimize for dinosaur simulation
    OptimizeForDinosaurSimulation();
    
    // Configure environmental destruction
    ConfigureEnvironmentalDestruction();
    
    // Apply current configuration
    ApplyChaosConfiguration();
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos Physics system initialized successfully"));
}

void UChaosPhysicsManager::ConfigureLargeScaleSimulation()
{
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Configuring Chaos Physics for large-scale simulation"));
    
    // Optimize for many objects
    CurrentConfig.MaxIterations = 6; // Reduced for performance
    CurrentConfig.CollisionPairIterations = 1; // Reduced for performance
    CurrentConfig.PushOutIterations = 2; // Reduced for performance
    CurrentConfig.CollisionMarginFraction = 0.02f; // Smaller margin for precision
    CurrentConfig.CollisionMarginMax = 0.5f; // Smaller max margin
    CurrentConfig.CollisionCullDistance = 10.0f; // Increased cull distance
    CurrentConfig.bEnableCCD = false; // Disable CCD for performance
    CurrentConfig.bEnhancedDeterminism = false; // Disable for performance
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Large-scale simulation configuration applied"));
}

void UChaosPhysicsManager::ApplyPhysicsProfile(AActor* Actor, const FString& ProfileName)
{
    if (!Actor)
    {
        UE_LOG(LogChaosPhysicsManager, Warning, TEXT("Cannot apply physics profile to null actor"));
        return;
    }
    
    const FPhysicsSimulationProfile* Profile = PhysicsProfiles.Find(ProfileName);
    if (!Profile)
    {
        UE_LOG(LogChaosPhysicsManager, Warning, TEXT("Physics profile '%s' not found"), *ProfileName);
        return;
    }
    
    // Apply profile to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            FBodyInstance* BodyInstance = PrimComp->GetBodyInstance();
            
            // Apply profile settings
            BodyInstance->SetMassScale(Profile->MassScale);
            BodyInstance->LinearDamping = Profile->LinearDamping;
            BodyInstance->AngularDamping = Profile->AngularDamping;
            BodyInstance->MaxLinearVelocity = Profile->MaxLinearVelocity;
            BodyInstance->MaxAngularVelocity = Profile->MaxAngularVelocity;
            BodyInstance->SleepThreshold = Profile->SleepThreshold;
            BodyInstance->StabilizationThresholdMultiplier = Profile->StabilizationThreshold;
            
            // Update the body instance
            BodyInstance->UpdateMassProperties();
        }
    }
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Applied physics profile '%s' to actor '%s'"), 
        *ProfileName, *Actor->GetName());
}

void UChaosPhysicsManager::CreatePhysicsProfile(const FString& ProfileName, const FPhysicsSimulationProfile& Profile)
{
    PhysicsProfiles.Add(ProfileName, Profile);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Created physics profile: %s"), *ProfileName);
}

FPhysicsSimulationProfile UChaosPhysicsManager::GetPhysicsProfile(const FString& ProfileName) const
{
    const FPhysicsSimulationProfile* Profile = PhysicsProfiles.Find(ProfileName);
    return Profile ? *Profile : FPhysicsSimulationProfile();
}

void UChaosPhysicsManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bPhysicsSimulationEnabled = bEnabled;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Enable/disable physics simulation for all actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->SetSimulatePhysics(bEnabled);
            }
        }
    }
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Physics simulation %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UChaosPhysicsManager::SetChaosPhysicsConfig(const FChaosPhysicsConfig& Config)
{
    CurrentConfig = Config;
    ApplyChaosConfiguration();
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos physics configuration updated"));
}

FChaosPhysicsConfig UChaosPhysicsManager::GetChaosPhysicsConfig() const
{
    return CurrentConfig;
}

void UChaosPhysicsManager::OptimizeForDinosaurSimulation()
{
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Optimizing physics for dinosaur simulation"));
    
    // Create dinosaur-specific physics profile
    FPhysicsSimulationProfile DinosaurProfile;
    DinosaurProfile.ProfileName = TEXT("Dinosaur");
    DinosaurProfile.MassScale = 1.0f;
    DinosaurProfile.LinearDamping = 0.05f; // Slight damping for realism
    DinosaurProfile.AngularDamping = 0.1f; // More angular damping for stability
    DinosaurProfile.MaxLinearVelocity = 2000.0f; // Fast but not unrealistic
    DinosaurProfile.MaxAngularVelocity = 1800.0f; // Reasonable rotation speed
    DinosaurProfile.SleepThreshold = 0.2f; // Higher threshold for large creatures
    DinosaurProfile.StabilizationThreshold = 0.1f;
    
    CreatePhysicsProfile(TEXT("Dinosaur"), DinosaurProfile);
    
    // Create smaller creature profile
    FPhysicsSimulationProfile SmallCreatureProfile;
    SmallCreatureProfile.ProfileName = TEXT("SmallCreature");
    SmallCreatureProfile.MassScale = 0.5f;
    SmallCreatureProfile.LinearDamping = 0.02f;
    SmallCreatureProfile.AngularDamping = 0.05f;
    SmallCreatureProfile.MaxLinearVelocity = 1500.0f;
    SmallCreatureProfile.MaxAngularVelocity = 2400.0f;
    SmallCreatureProfile.SleepThreshold = 0.1f;
    SmallCreatureProfile.StabilizationThreshold = 0.05f;
    
    CreatePhysicsProfile(TEXT("SmallCreature"), SmallCreatureProfile);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Dinosaur physics optimization complete"));
}

void UChaosPhysicsManager::ConfigureEnvironmentalDestruction()
{
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Configuring environmental destruction physics"));
    
    // Create debris physics profile
    FPhysicsSimulationProfile DebrisProfile;
    DebrisProfile.ProfileName = TEXT("Debris");
    DebrisProfile.MassScale = 0.8f;
    DebrisProfile.LinearDamping = 0.1f; // Higher damping to settle quickly
    DebrisProfile.AngularDamping = 0.2f; // High angular damping
    DebrisProfile.MaxLinearVelocity = 1000.0f;
    DebrisProfile.MaxAngularVelocity = 1200.0f;
    DebrisProfile.SleepThreshold = 0.3f; // Sleep quickly
    DebrisProfile.StabilizationThreshold = 0.15f;
    
    CreatePhysicsProfile(TEXT("Debris"), DebrisProfile);
    
    // Create destructible environment profile
    FPhysicsSimulationProfile DestructibleProfile;
    DestructibleProfile.ProfileName = TEXT("Destructible");
    DestructibleProfile.MassScale = 1.2f; // Heavier for more impact
    DestructibleProfile.LinearDamping = 0.03f;
    DestructibleProfile.AngularDamping = 0.08f;
    DestructibleProfile.MaxLinearVelocity = 1500.0f;
    DestructibleProfile.MaxAngularVelocity = 1800.0f;
    DestructibleProfile.SleepThreshold = 0.15f;
    DestructibleProfile.StabilizationThreshold = 0.08f;
    
    CreatePhysicsProfile(TEXT("Destructible"), DestructibleProfile);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Environmental destruction configuration complete"));
}

void UChaosPhysicsManager::SetupPhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Set up timer for physics LOD updates
    World->GetTimerManager().SetTimer(PhysicsLODTimerHandle, this, 
        &UChaosPhysicsManager::UpdatePhysicsLOD, 1.0f, true);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Physics LOD system initialized"));
}

void UChaosPhysicsManager::ForcePhysicsUpdate()
{
    UWorld* World = GetWorld();
    if (!World || !CachedPhysicsScene)
    {
        return;
    }
    
    // Force physics scene update
    CachedPhysicsScene->FlushAsyncScene();
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Physics update forced"));
}

void UChaosPhysicsManager::GetPhysicsStatistics(int32& ActiveBodies, int32& SleepingBodies, float& SimulationTime) const
{
    ActiveBodies = 0;
    SleepingBodies = 0;
    SimulationTime = 0.0f;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count physics bodies
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (PrimComp->IsAnyRigidBodySleeping())
                {
                    SleepingBodies++;
                }
                else
                {
                    ActiveBodies++;
                }
            }
        }
    }
    
    // Simulation time would need to be tracked separately
    SimulationTime = 0.0f; // Placeholder
}

void UChaosPhysicsManager::SetPhysicsDebuggingEnabled(bool bEnabled)
{
    bPhysicsDebuggingEnabled = bEnabled;
    
    // Enable/disable physics debugging visualization
    if (bEnabled)
    {
        // Enable physics debug drawing
        if (GEngine && GEngine->GetGameUserSettings())
        {
            // This would enable physics debug visualization
            UE_LOG(LogChaosPhysicsManager, Log, TEXT("Physics debugging enabled"));
        }
    }
    else
    {
        UE_LOG(LogChaosPhysicsManager, Log, TEXT("Physics debugging disabled"));
    }
}

void UChaosPhysicsManager::ResetAllPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ResetCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Reset physics state
                PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
                PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                PrimComp->WakeAllRigidBodies();
                ResetCount++;
            }
        }
    }
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Reset %d physics actors"), ResetCount);
}

void UChaosPhysicsManager::InitializeDefaultProfiles()
{
    // Default profile
    FPhysicsSimulationProfile DefaultProfile;
    DefaultProfile.ProfileName = TEXT("Default");
    DefaultProfile.MassScale = 1.0f;
    DefaultProfile.LinearDamping = 0.01f;
    DefaultProfile.AngularDamping = 0.0f;
    DefaultProfile.MaxLinearVelocity = 3000.0f;
    DefaultProfile.MaxAngularVelocity = 3600.0f;
    DefaultProfile.SleepThreshold = 0.1f;
    DefaultProfile.StabilizationThreshold = 0.05f;
    
    CreatePhysicsProfile(TEXT("Default"), DefaultProfile);
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Default physics profiles initialized"));
}

void UChaosPhysicsManager::ApplyChaosConfiguration()
{
    // Apply configuration to UPhysicsSettings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        UE_LOG(LogChaosPhysicsManager, Warning, TEXT("Could not access physics settings"));
        return;
    }
    
    // Apply Chaos-specific settings
    PhysicsSettings->ChaosSettings.Iterations = CurrentConfig.MaxIterations;
    PhysicsSettings->ChaosSettings.CollisionPairIterations = CurrentConfig.CollisionPairIterations;
    PhysicsSettings->ChaosSettings.PushOutIterations = CurrentConfig.PushOutIterations;
    PhysicsSettings->ChaosSettings.CollisionMarginFraction = CurrentConfig.CollisionMarginFraction;
    PhysicsSettings->ChaosSettings.CollisionMarginMax = CurrentConfig.CollisionMarginMax;
    PhysicsSettings->ChaosSettings.CollisionCullDistance = CurrentConfig.CollisionCullDistance;
    
    UE_LOG(LogChaosPhysicsManager, Log, TEXT("Chaos physics configuration applied to engine settings"));
}

void UChaosPhysicsManager::UpdatePhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player locations for distance calculations
    TArray<FVector> PlayerLocations;
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            PlayerLocations.Add(PC->GetPawn()->GetActorLocation());
        }
    }
    
    if (PlayerLocations.Num() == 0)
    {
        return; // No players to calculate distance from
    }
    
    // Update physics LOD for all actors
    int32 ProcessedActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        // Calculate distance to nearest player
        float NearestDistance = GetDistanceToNearestPlayer(Actor->GetActorLocation());
        
        // Apply LOD based on distance
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Determine LOD level
                bool bShouldSimulate = true;
                if (PhysicsLODDistances.Num() > 0 && NearestDistance > PhysicsLODDistances[0])
                {
                    // Far distance - disable physics
                    bShouldSimulate = false;
                }
                
                if (PrimComp->IsSimulatingPhysics() != bShouldSimulate)
                {
                    PrimComp->SetSimulatePhysics(bShouldSimulate);
                    ProcessedActors++;
                }
            }
        }
    }
    
    if (ProcessedActors > 0)
    {
        UE_LOG(LogChaosPhysicsManager, VeryVerbose, TEXT("Updated physics LOD for %d actors"), ProcessedActors);
    }
}

float UChaosPhysicsManager::GetDistanceToNearestPlayer(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FLT_MAX;
    }
    
    float NearestDistance = FLT_MAX;
    
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
            NearestDistance = FMath::Min(NearestDistance, Distance);
        }
    }
    
    return NearestDistance;
}