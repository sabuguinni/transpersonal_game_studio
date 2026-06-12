#include "Crowd_MassEntityConfig.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UCrowd_MassEntityConfig::UCrowd_MassEntityConfig()
{
    // Set default values for crowd simulation
    MaxCrowdEntities = 1000;
    SpawnRadius = 2000.0f;
    SpawnCenter = FVector::ZeroVector;
    
    DefaultMovementSpeed = 150.0f;
    RunningSpeedMultiplier = 2.5f;
    StateChangeInterval = 3.0f;
    FleeDistance = 800.0f;
    
    HighLODDistance = 500.0f;
    MediumLODDistance = 1500.0f;
    LowLODDistance = 3000.0f;
    
    GroupFormationRadius = 300.0f;
    MaxGroupSize = 8;
    LeadershipProbability = 0.15f;
    
    MaxProcessingEntitiesPerFrame = 200;
    UpdateFrequency = 0.1f;
    bEnableMultithreading = true;
}