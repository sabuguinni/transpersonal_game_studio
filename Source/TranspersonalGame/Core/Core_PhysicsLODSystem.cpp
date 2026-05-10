#include "Core_PhysicsLODSystem.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsLOD, Log, All);

UCore_PhysicsLODSystem::UCore_PhysicsLODSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
	
	// Initialize LOD distances
	LODDistances.Add(500.0f);   // LOD 0 - Full physics
	LODDistances.Add(1500.0f);  // LOD 1 - Reduced physics
	LODDistances.Add(3000.0f);  // LOD 2 - Minimal physics
	LODDistances.Add(5000.0f);  // LOD 3 - No physics
	
	MaxPhysicsObjects = 100;
	CurrentPhysicsObjects = 0;
	bEnableLODSystem = true;
	bDebugLOD = false;
}

void UCore_PhysicsLODSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// Find all physics objects in the world
	RefreshPhysicsObjects();
	
	UE_LOG(LogPhysicsLOD, Log, TEXT("Physics LOD System initialized with %d objects"), PhysicsObjects.Num());
}

void UCore_PhysicsLODSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bEnableLODSystem)
		return;
	
	UpdatePhysicsLOD();
}

void UCore_PhysicsLODSystem::RefreshPhysicsObjects()
{
	PhysicsObjects.Empty();
	
	UWorld* World = GetWorld();
	if (!World)
		return;
	
	// Find all actors with physics components
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (!Actor)
			continue;
		
		// Look for primitive components with physics
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		
		for (UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (Component && Component->IsSimulatingPhysics())
			{
				FPhysicsLODObject LODObject;
				LODObject.Actor = Actor;
				LODObject.Component = Component;
				LODObject.CurrentLOD = 0;
				LODObject.bOriginallySimulatingPhysics = true;
				
				PhysicsObjects.Add(LODObject);
				break; // One entry per actor
			}
		}
	}
	
	CurrentPhysicsObjects = PhysicsObjects.Num();
}

void UCore_PhysicsLODSystem::UpdatePhysicsLOD()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
		return;
	
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	int32 ActivePhysicsCount = 0;
	
	for (FPhysicsLODObject& LODObject : PhysicsObjects)
	{
		if (!LODObject.Actor.IsValid() || !LODObject.Component.IsValid())
			continue;
		
		float Distance = FVector::Dist(PlayerLocation, LODObject.Actor->GetActorLocation());
		int32 NewLOD = CalculateLODLevel(Distance);
		
		if (NewLOD != LODObject.CurrentLOD)
		{
			ApplyLODLevel(LODObject, NewLOD);
			LODObject.CurrentLOD = NewLOD;
		}
		
		if (NewLOD < 3) // Count active physics objects (LOD 0-2)
			ActivePhysicsCount++;
	}
	
	// Performance management - disable furthest objects if over limit
	if (ActivePhysicsCount > MaxPhysicsObjects)
	{
		EnforcePhysicsLimit(PlayerLocation);
	}
	
	if (bDebugLOD)
	{
		UE_LOG(LogPhysicsLOD, Log, TEXT("Active physics objects: %d/%d"), ActivePhysicsCount, MaxPhysicsObjects);
	}
}

int32 UCore_PhysicsLODSystem::CalculateLODLevel(float Distance) const
{
	for (int32 i = 0; i < LODDistances.Num(); i++)
	{
		if (Distance < LODDistances[i])
			return i;
	}
	return LODDistances.Num(); // Maximum LOD (no physics)
}

void UCore_PhysicsLODSystem::ApplyLODLevel(FPhysicsLODObject& LODObject, int32 LODLevel)
{
	if (!LODObject.Component.IsValid())
		return;
	
	UPrimitiveComponent* Component = LODObject.Component.Get();
	
	switch (LODLevel)
	{
		case 0: // Full physics
			Component->SetSimulatePhysics(LODObject.bOriginallySimulatingPhysics);
			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Component->SetNotifyRigidBodyCollision(true);
			break;
			
		case 1: // Reduced physics
			Component->SetSimulatePhysics(LODObject.bOriginallySimulatingPhysics);
			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Component->SetNotifyRigidBodyCollision(false); // Disable collision events
			break;
			
		case 2: // Minimal physics
			Component->SetSimulatePhysics(false);
			Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Component->SetNotifyRigidBodyCollision(false);
			break;
			
		case 3: // No physics
		default:
			Component->SetSimulatePhysics(false);
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Component->SetNotifyRigidBodyCollision(false);
			break;
	}
	
	if (bDebugLOD)
	{
		UE_LOG(LogPhysicsLOD, VeryVerbose, TEXT("Applied LOD %d to %s"), LODLevel, *LODObject.Actor->GetName());
	}
}

void UCore_PhysicsLODSystem::EnforcePhysicsLimit(const FVector& PlayerLocation)
{
	// Sort objects by distance and disable the furthest ones
	PhysicsObjects.Sort([&PlayerLocation](const FPhysicsLODObject& A, const FPhysicsLODObject& B)
	{
		if (!A.Actor.IsValid() || !B.Actor.IsValid())
			return false;
		
		float DistA = FVector::DistSquared(PlayerLocation, A.Actor->GetActorLocation());
		float DistB = FVector::DistSquared(PlayerLocation, B.Actor->GetActorLocation());
		return DistA < DistB;
	});
	
	int32 ActiveCount = 0;
	for (FPhysicsLODObject& LODObject : PhysicsObjects)
	{
		if (!LODObject.Actor.IsValid())
			continue;
		
		if (ActiveCount < MaxPhysicsObjects && LODObject.CurrentLOD < 3)
		{
			ActiveCount++;
		}
		else if (LODObject.CurrentLOD < 3)
		{
			// Force to no-physics LOD
			ApplyLODLevel(LODObject, 3);
			LODObject.CurrentLOD = 3;
		}
	}
}

void UCore_PhysicsLODSystem::SetMaxPhysicsObjects(int32 NewMax)
{
	MaxPhysicsObjects = FMath::Clamp(NewMax, 10, 500);
	UE_LOG(LogPhysicsLOD, Log, TEXT("Max physics objects set to %d"), MaxPhysicsObjects);
}

void UCore_PhysicsLODSystem::SetLODDistance(int32 LODLevel, float Distance)
{
	if (LODLevel >= 0 && LODLevel < LODDistances.Num())
	{
		LODDistances[LODLevel] = Distance;
		UE_LOG(LogPhysicsLOD, Log, TEXT("LOD %d distance set to %.1f"), LODLevel, Distance);
	}
}

void UCore_PhysicsLODSystem::EnableDebugLOD(bool bEnable)
{
	bDebugLOD = bEnable;
	UE_LOG(LogPhysicsLOD, Log, TEXT("Debug LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsLODSystem::GetPhysicsLODStats(int32& OutTotalObjects, int32& OutActiveObjects, TArray<int32>& OutLODCounts) const
{
	OutTotalObjects = PhysicsObjects.Num();
	OutActiveObjects = 0;
	OutLODCounts.Init(0, 4);
	
	for (const FPhysicsLODObject& LODObject : PhysicsObjects)
	{
		if (LODObject.Actor.IsValid())
		{
			if (LODObject.CurrentLOD < 3)
				OutActiveObjects++;
			
			if (LODObject.CurrentLOD >= 0 && LODObject.CurrentLOD < OutLODCounts.Num())
				OutLODCounts[LODObject.CurrentLOD]++;
		}
	}
}