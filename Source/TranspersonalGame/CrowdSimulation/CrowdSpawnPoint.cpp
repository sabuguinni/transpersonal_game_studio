#include "CrowdSpawnPoint.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACrowdSpawnPoint::ACrowdSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MaxCrowdMembers = 10;
    SpawnRadius = 500.0f;
    DefaultBehavior = ECrowd_SpawnBehavior::Idle;
    bAutoSpawnOnBeginPlay = true;
}

void ACrowdSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoSpawnOnBeginPlay)
    {
        SpawnCrowdMembers();
    }
}

void ACrowdSpawnPoint::SpawnCrowdMembers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    DespawnAllMembers();

    for (int32 i = 0; i < MaxCrowdMembers; ++i)
    {
        float Angle = (360.0f / MaxCrowdMembers) * i;
        float RadAngle = FMath::DegreesToRadians(Angle);
        
        FVector Offset(
            FMath::Cos(RadAngle) * SpawnRadius,
            FMath::Sin(RadAngle) * SpawnRadius,
            0.0f
        );
        
        FVector SpawnLocation = GetActorLocation() + Offset;
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        // Placeholder - in production this would spawn actual NPC actors
        // AActor* Member = World->SpawnActor<AActor>(NPCClass, SpawnLocation, SpawnRotation, SpawnParams);
        // if (Member)
        // {
        //     SpawnedMembers.Add(Member);
        // }
    }
}

void ACrowdSpawnPoint::DespawnAllMembers()
{
    for (AActor* Member : SpawnedMembers)
    {
        if (Member)
        {
            Member->Destroy();
        }
    }
    SpawnedMembers.Empty();
}
