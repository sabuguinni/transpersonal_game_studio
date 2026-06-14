#include "Crowd_MassEntitySubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem Initialized"));
    
    // Reserve space for entities
    EntityData.Reserve(1000);
    EntityActors.Reserve(1000);
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    // Clean up all spawned actors
    for (AActor* Actor : EntityActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    EntityData.Empty();
    EntityActors.Empty();
    
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::Tick(float DeltaTime)
{
    if (EntityData.Num() > 0)
    {
        UpdateEntityMovement(DeltaTime);
        UpdateEntityLOD();
    }
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(int32 Count, FVector SpawnCenter, float SpawnRadius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Generate random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        
        FVector SpawnLocation = SpawnCenter + FVector(
            Distance * FMath::Cos(Angle),
            Distance * FMath::Sin(Angle),
            0.0f
        );

        // Spawn static mesh actor
        AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            FRotator::ZeroRotator
        );

        if (NewActor)
        {
            // Set basic cube mesh
            UStaticMeshComponent* MeshComp = NewActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
                if (CubeMeshAsset.Succeeded())
                {
                    MeshComp->SetStaticMesh(CubeMeshAsset.Object);
                    MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 1.8f)); // Human-like proportions
                }
            }

            NewActor->SetActorLabel(FString::Printf(TEXT("CrowdEntity_%d"), i));
            EntityActors.Add(NewActor);

            // Create entity data
            FCrowd_EntityData NewEntity;
            NewEntity.Position = SpawnLocation;
            NewEntity.Velocity = FVector(
                FMath::RandRange(-50.0f, 50.0f),
                FMath::RandRange(-50.0f, 50.0f),
                0.0f
            );
            NewEntity.Speed = FMath::RandRange(80.0f, 120.0f);
            NewEntity.LODLevel = 0;
            NewEntity.bIsActive = true;

            EntityData.Add(NewEntity);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities"), Count);
}

void UCrowd_MassEntitySubsystem::UpdateEntityMovement(float DeltaTime)
{
    for (int32 i = 0; i < EntityData.Num(); i++)
    {
        if (!EntityData[i].bIsActive || i >= EntityActors.Num())
        {
            continue;
        }

        AActor* Actor = EntityActors[i];
        if (!IsValid(Actor))
        {
            continue;
        }

        // Simple movement update
        FVector NewPosition = EntityData[i].Position + (EntityData[i].Velocity * DeltaTime);
        EntityData[i].Position = NewPosition;
        
        Actor->SetActorLocation(NewPosition);

        // Random direction change occasionally
        if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per frame
        {
            EntityData[i].Velocity = FVector(
                FMath::RandRange(-EntityData[i].Speed, EntityData[i].Speed),
                FMath::RandRange(-EntityData[i].Speed, EntityData[i].Speed),
                0.0f
            );
        }
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD(const FVector& ViewerPosition)
{
    for (int32 i = 0; i < EntityData.Num(); i++)
    {
        if (i >= EntityActors.Num())
        {
            continue;
        }

        AActor* Actor = EntityActors[i];
        if (!IsValid(Actor))
        {
            continue;
        }

        float Distance = FVector::Dist(ViewerPosition, EntityData[i].Position);
        
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            if (Distance < LODDistance_High)
            {
                EntityData[i].LODLevel = 0; // High LOD
                MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 1.8f));
                MeshComp->SetVisibility(true);
            }
            else if (Distance < LODDistance_Medium)
            {
                EntityData[i].LODLevel = 1; // Medium LOD
                MeshComp->SetWorldScale3D(FVector(0.4f, 0.4f, 1.5f));
                MeshComp->SetVisibility(true);
            }
            else if (Distance < LODDistance_Low)
            {
                EntityData[i].LODLevel = 2; // Low LOD
                MeshComp->SetWorldScale3D(FVector(0.3f, 0.3f, 1.0f));
                MeshComp->SetVisibility(true);
            }
            else
            {
                EntityData[i].LODLevel = 3; // Culled
                MeshComp->SetVisibility(false);
            }
        }
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD()
{
    // Get player position for LOD calculation
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector ViewerPosition = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    UpdateEntityLOD(ViewerPosition);
}

void UCrowd_MassEntitySubsystem::CullDistantEntities(const FVector& ViewerPosition)
{
    for (int32 i = 0; i < EntityData.Num(); i++)
    {
        float Distance = FVector::Dist(ViewerPosition, EntityData[i].Position);
        EntityData[i].bIsActive = (Distance < CullDistance);
    }
}

void UCrowd_MassEntitySubsystem::SetEntityMovementTarget(int32 EntityIndex, const FVector& TargetPosition)
{
    if (EntityIndex >= 0 && EntityIndex < EntityData.Num())
    {
        FVector Direction = (TargetPosition - EntityData[EntityIndex].Position).GetSafeNormal();
        EntityData[EntityIndex].Velocity = Direction * EntityData[EntityIndex].Speed;
    }
}

int32 UCrowd_MassEntitySubsystem::GetActiveEntityCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_EntityData& Entity : EntityData)
    {
        if (Entity.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}