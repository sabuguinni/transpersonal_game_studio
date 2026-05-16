#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance

    // Initialize destruction settings
    DestructionThreshold = 100.0f;
    bPhysicsDestructionEnabled = true;
    DebrisLifetime = 30.0f;
    MaxDebrisCount = 50;
    MaxEventHistory = 100;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDestructibleParts();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Initialized with %d destructible parts"), DestructibleParts.Num());
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateDestructionStates(DeltaTime);
        CleanupOldEvents();
        LastUpdateTime = 0.0f;
    }
}

void UCore_DestructionSystem::ApplyDamage(float Damage, const FVector& ImpactLocation, AActor* DamageCauser)
{
    if (Damage <= 0.0f)
    {
        return;
    }

    // Find closest destructible part to impact location
    FCore_DestructiblePart* ClosestPart = nullptr;
    float ClosestDistance = FLT_MAX;

    for (FCore_DestructiblePart& Part : DestructibleParts)
    {
        if (Part.bIsDestroyed || !Part.MeshComponent)
        {
            continue;
        }

        float Distance = FVector::Dist(Part.MeshComponent->GetComponentLocation(), ImpactLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestPart = &Part;
        }
    }

    if (ClosestPart)
    {
        ClosestPart->Health -= Damage;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Part %s took %.2f damage, health: %.2f/%.2f"), 
               *ClosestPart->PartName, Damage, ClosestPart->Health, ClosestPart->MaxHealth);

        if (ClosestPart->Health <= 0.0f)
        {
            DestroyPart(ClosestPart->PartName);
        }

        // Register destruction event
        FCore_DestructionEvent Event;
        Event.DestroyedActor = GetOwner();
        Event.ImpactLocation = ImpactLocation;
        Event.Damage = Damage;
        Event.DestructionType = EPhysicsType::Dynamic;
        RegisterDestructionEvent(Event);
    }
}

void UCore_DestructionSystem::DestroyPart(const FString& PartName)
{
    for (FCore_DestructiblePart& Part : DestructibleParts)
    {
        if (Part.PartName == PartName && !Part.bIsDestroyed)
        {
            Part.bIsDestroyed = true;
            
            if (Part.MeshComponent)
            {
                FVector PartLocation = Part.MeshComponent->GetComponentLocation();
                
                // Create debris
                CreateDebris(PartLocation, FMath::RandRange(3, 8));
                
                // Hide or destroy the mesh component
                Part.MeshComponent->SetVisibility(false);
                Part.MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                
                UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Destroyed part %s"), *PartName);
            }
            
            break;
        }
    }
}

void UCore_DestructionSystem::CreateDebris(const FVector& Location, int32 DebrisCount)
{
    if (DebrisCount <= 0 || DebrisCount > MaxDebrisCount)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Random debris spawn location around the destruction point
        FVector DebrisLocation = Location + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 50.0f)
        );

        // Random velocity for debris
        FVector DebrisVelocity = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(200.0f, 800.0f)
        );

        SpawnDebrisActor(DebrisLocation, DebrisVelocity);
    }

    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Created %d debris pieces at location %s"), 
           DebrisCount, *Location.ToString());
}

void UCore_DestructionSystem::SimulateTreeFall(const FVector& ImpactDirection)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }

    // Enable physics simulation
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Apply impulse to simulate tree falling
    FVector FallImpulse = ImpactDirection.GetSafeNormal() * 50000.0f; // Strong impulse for tree fall
    FallImpulse.Z = FMath::Abs(FallImpulse.Z) * 0.3f; // Reduce vertical component

    MeshComp->AddImpulse(FallImpulse);

    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Tree fall simulation started with impulse %s"), 
           *FallImpulse.ToString());
}

void UCore_DestructionSystem::SimulateRockBreak(float ImpactForce)
{
    if (ImpactForce < DestructionThreshold)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector RockLocation = Owner->GetActorLocation();
    
    // Create multiple rock fragments
    int32 FragmentCount = FMath::RandRange(4, 12);
    CreateDebris(RockLocation, FragmentCount);

    // Apply damage to destroy the original rock
    ApplyDamage(ImpactForce * 0.1f, RockLocation, nullptr);

    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Rock break simulation with force %.2f"), ImpactForce);
}

void UCore_DestructionSystem::EnablePhysicsDestruction(bool bEnable)
{
    bPhysicsDestructionEnabled = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Physics destruction %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_DestructionSystem::SetDestructionThreshold(float Threshold)
{
    DestructionThreshold = FMath::Max(0.0f, Threshold);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: Destruction threshold set to %.2f"), DestructionThreshold);
}

bool UCore_DestructionSystem::IsFullyDestroyed() const
{
    for (const FCore_DestructiblePart& Part : DestructibleParts)
    {
        if (!Part.bIsDestroyed)
        {
            return false;
        }
    }
    return DestructibleParts.Num() > 0; // Only fully destroyed if we have parts and all are destroyed
}

void UCore_DestructionSystem::RegisterDestructionEvent(const FCore_DestructionEvent& Event)
{
    DestructionEvents.Add(Event);
    
    // Limit event history size
    if (DestructionEvents.Num() > MaxEventHistory)
    {
        DestructionEvents.RemoveAt(0, DestructionEvents.Num() - MaxEventHistory);
    }
}

TArray<FCore_DestructionEvent> UCore_DestructionSystem::GetRecentDestructionEvents() const
{
    // Return last 10 events
    int32 StartIndex = FMath::Max(0, DestructionEvents.Num() - 10);
    TArray<FCore_DestructionEvent> RecentEvents;
    
    for (int32 i = StartIndex; i < DestructionEvents.Num(); i++)
    {
        RecentEvents.Add(DestructionEvents[i]);
    }
    
    return RecentEvents;
}

void UCore_DestructionSystem::InitializeDestructibleParts()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Find all static mesh components on the owner
    TArray<UStaticMeshComponent*> MeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            FCore_DestructiblePart NewPart;
            NewPart.PartName = MeshComp->GetName();
            NewPart.Health = 100.0f;
            NewPart.MaxHealth = 100.0f;
            NewPart.bIsDestroyed = false;
            NewPart.MeshComponent = MeshComp;
            
            DestructibleParts.Add(NewPart);
        }
    }

    // If no mesh components found, create a default part
    if (DestructibleParts.Num() == 0)
    {
        FCore_DestructiblePart DefaultPart;
        DefaultPart.PartName = TEXT("Main");
        DefaultPart.Health = 100.0f;
        DefaultPart.MaxHealth = 100.0f;
        DefaultPart.bIsDestroyed = false;
        DefaultPart.MeshComponent = nullptr;
        
        DestructibleParts.Add(DefaultPart);
    }
}

void UCore_DestructionSystem::UpdateDestructionStates(float DeltaTime)
{
    // Update any ongoing destruction animations or physics
    for (FCore_DestructiblePart& Part : DestructibleParts)
    {
        if (Part.bIsDestroyed || !Part.MeshComponent)
        {
            continue;
        }

        // Check if physics simulation is active and part should be destroyed
        if (bPhysicsDestructionEnabled && Part.MeshComponent->IsSimulatingPhysics())
        {
            FVector Velocity = Part.MeshComponent->GetPhysicsLinearVelocity();
            float Speed = Velocity.Size();
            
            // High speed impacts can cause additional damage
            if (Speed > 1000.0f)
            {
                float ImpactDamage = CalculateDamageFromImpact(Speed);
                if (ImpactDamage > 0.0f)
                {
                    Part.Health -= ImpactDamage * DeltaTime;
                    
                    if (Part.Health <= 0.0f)
                    {
                        DestroyPart(Part.PartName);
                    }
                }
            }
        }
    }
}

void UCore_DestructionSystem::CleanupOldEvents()
{
    // Remove events older than 60 seconds
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    DestructionEvents.RemoveAll([CurrentTime](const FCore_DestructionEvent& Event)
    {
        // Simple cleanup - remove if we have too many events
        return false; // For now, keep all events until max history limit
    });
}

void UCore_DestructionSystem::SpawnDebrisActor(const FVector& Location, const FVector& Velocity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn a simple debris actor (cube for now)
    AStaticMeshActor* DebrisActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (DebrisActor)
    {
        UStaticMeshComponent* DebrisMesh = DebrisActor->GetStaticMeshComponent();
        if (DebrisMesh)
        {
            // Set random scale for debris
            float Scale = FMath::RandRange(0.1f, 0.5f);
            DebrisActor->SetActorScale3D(FVector(Scale));
            
            // Enable physics
            DebrisMesh->SetSimulatePhysics(true);
            DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Apply initial velocity
            DebrisMesh->SetPhysicsLinearVelocity(Velocity);
            
            // Set debris to destroy itself after lifetime
            DebrisActor->SetLifeSpan(DebrisLifetime);
        }
    }
}

float UCore_DestructionSystem::CalculateDamageFromImpact(float ImpactForce) const
{
    // Simple damage calculation based on impact force
    float BaseDamage = ImpactForce * 0.01f; // Scale factor
    return FMath::Clamp(BaseDamage, 0.0f, 50.0f); // Max 50 damage per impact
}