#include "CrowdSpawnPoint.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

ACrowdSpawnPoint::ACrowdSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create spawn radius sphere
    SpawnRadius = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnRadius"));
    SpawnRadius->SetupAttachment(RootComponent);
    SpawnRadius->SetSphereRadius(500.0f);
    SpawnRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnRadius->SetVisibility(true);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    
    // Try to set a default mesh (sphere)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(SphereMesh.Object);
        VisualizationMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Default configuration
    MaxCrowdSize = 10;
    SpawnInterval = 5.0f;
    NPCType = ECrowd_NPCType::Tribal;
    DefaultActivity = ECrowd_ActivityType::Wandering;
    WanderRadius = 1000.0f;
    bIsActive = true;

    // Runtime state
    CurrentCrowdCount = 0;
    LastSpawnTime = 0.0f;
    SpawnedNPCs.Empty();
}

void ACrowdSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVisualization();
    
    // Log spawn point activation
    UE_LOG(LogTemp, Log, TEXT("CrowdSpawnPoint %s activated at location %s"), 
           *GetName(), *GetActorLocation().ToString());
}

void ACrowdSpawnPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsActive)
    {
        UpdateCrowdSpawning(DeltaTime);
        
        // Periodically clean up invalid NPCs
        static float CleanupTimer = 0.0f;
        CleanupTimer += DeltaTime;
        if (CleanupTimer >= 10.0f)
        {
            CleanupInvalidNPCs();
            CleanupTimer = 0.0f;
        }
    }
}

void ACrowdSpawnPoint::SpawnCrowdMember()
{
    if (!CanSpawnNewNPC())
    {
        return;
    }

    FVector SpawnLocation = GetRandomSpawnLocation();
    
    // For now, spawn a basic actor as placeholder
    // In full implementation, this would spawn a proper NPC with Mass Entity
    AActor* NewNPC = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    
    if (NewNPC)
    {
        NewNPC->SetActorLabel(FString::Printf(TEXT("CrowdNPC_%d"), CurrentCrowdCount + 1));
        SpawnedNPCs.Add(NewNPC);
        CurrentCrowdCount++;
        LastSpawnTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Spawned crowd member %d at %s"), 
               CurrentCrowdCount, *SpawnLocation.ToString());
    }
}

void ACrowdSpawnPoint::DespawnCrowdMember(AActor* NPCActor)
{
    if (NPCActor && SpawnedNPCs.Contains(NPCActor))
    {
        SpawnedNPCs.Remove(NPCActor);
        CurrentCrowdCount = FMath::Max(0, CurrentCrowdCount - 1);
        NPCActor->Destroy();
        
        UE_LOG(LogTemp, Log, TEXT("Despawned crowd member, count now: %d"), CurrentCrowdCount);
    }
}

void ACrowdSpawnPoint::SetSpawnActive(bool bActive)
{
    bIsActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("CrowdSpawnPoint %s set to %s"), 
           *GetName(), bActive ? TEXT("Active") : TEXT("Inactive"));
}

FVector ACrowdSpawnPoint::GetRandomSpawnLocation() const
{
    FVector BaseLocation = GetActorLocation();
    float Radius = SpawnRadius->GetScaledSphereRadius();
    
    // Generate random point within spawn radius
    FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, Radius);
    RandomOffset.Z = 0.0f; // Keep on ground level
    
    FVector SpawnLocation = BaseLocation + RandomOffset;
    
    // Try to find valid navigation point
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return SpawnLocation;
}

void ACrowdSpawnPoint::CleanupInvalidNPCs()
{
    TArray<AActor*> ValidNPCs;
    
    for (AActor* NPC : SpawnedNPCs)
    {
        if (IsValid(NPC))
        {
            ValidNPCs.Add(NPC);
        }
    }
    
    if (ValidNPCs.Num() != SpawnedNPCs.Num())
    {
        int32 RemovedCount = SpawnedNPCs.Num() - ValidNPCs.Num();
        SpawnedNPCs = ValidNPCs;
        CurrentCrowdCount = ValidNPCs.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d invalid NPCs, current count: %d"), 
               RemovedCount, CurrentCrowdCount);
    }
}

void ACrowdSpawnPoint::UpdateCrowdSpawning(float DeltaTime)
{
    if (!CanSpawnNewNPC())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSpawnTime >= SpawnInterval)
    {
        SpawnCrowdMember();
    }
}

bool ACrowdSpawnPoint::CanSpawnNewNPC() const
{
    return bIsActive && 
           CurrentCrowdCount < MaxCrowdSize && 
           GetWorld() != nullptr;
}

void ACrowdSpawnPoint::InitializeVisualization()
{
    if (VisualizationMesh)
    {
        // Set material color based on NPC type
        // This would be expanded with proper materials in full implementation
        VisualizationMesh->SetVisibility(true);
    }
    
    if (SpawnRadius)
    {
        SpawnRadius->SetSphereRadius(WanderRadius * 0.5f);
    }
}