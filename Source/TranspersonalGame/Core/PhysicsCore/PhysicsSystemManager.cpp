#include "PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/StaticMeshActor.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize physics settings
    bEnableCollisionDetection = true;
    bEnableRagdollPhysics = true;
    bEnableDestructibleObjects = true;
    
    // Physics material settings
    DefaultFriction = 0.7f;
    DefaultRestitution = 0.3f;
    DefaultDensity = 1.0f;
    
    // Collision settings
    MaxCollisionDistance = 10000.0f;
    CollisionCheckInterval = 0.033f; // 30 FPS for collision checks
    
    // Ragdoll settings
    RagdollActivationThreshold = 50.0f;
    RagdollDeactivationTime = 5.0f;
    
    // Performance settings
    MaxPhysicsObjects = 1000;
    PhysicsLODDistance = 5000.0f;
    
    LastCollisionCheckTime = 0.0f;
    ActiveRagdolls.Empty();
    PhysicsObjects.Empty();
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing physics systems"));
    
    // Initialize physics world settings
    InitializePhysicsSettings();
    
    // Register existing physics objects
    RegisterExistingPhysicsObjects();
    
    // Set up collision channels
    SetupCollisionChannels();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics systems initialized successfully"));
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update collision detection
    if (bEnableCollisionDetection)
    {
        UpdateCollisionDetection(DeltaTime);
    }
    
    // Update ragdoll physics
    if (bEnableRagdollPhysics)
    {
        UpdateRagdollPhysics(DeltaTime);
    }
    
    // Update physics LOD
    UpdatePhysicsLOD(DeltaTime);
    
    // Clean up inactive physics objects
    CleanupInactiveObjects(DeltaTime);
}

void UPhysicsSystemManager::InitializePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: No valid world found"));
        return;
    }
    
    // Set global physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Enable chaos physics for better performance
        PhysicsSettings->DefaultGravityZ = -980.0f; // Realistic gravity
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016f;
        PhysicsSettings->MaxSubsteps = 6;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics settings initialized"));
}

void UPhysicsSystemManager::RegisterExistingPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all actors with physics components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValidPhysicsActor(Actor))
        {
            RegisterPhysicsObject(Actor);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Registered %d physics objects"), PhysicsObjects.Num());
}

void UPhysicsSystemManager::SetupCollisionChannels()
{
    // Define custom collision channels for prehistoric survival game
    // This would typically be done in project settings, but we log the setup here
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Setting up collision channels"));
    UE_LOG(LogTemp, Warning, TEXT("- Player collision channel"));
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaur collision channel"));
    UE_LOG(LogTemp, Warning, TEXT("- Environment collision channel"));
    UE_LOG(LogTemp, Warning, TEXT("- Projectile collision channel"));
    UE_LOG(LogTemp, Warning, TEXT("- Destructible collision channel"));
}

void UPhysicsSystemManager::UpdateCollisionDetection(float DeltaTime)
{
    LastCollisionCheckTime += DeltaTime;
    
    if (LastCollisionCheckTime >= CollisionCheckInterval)
    {
        LastCollisionCheckTime = 0.0f;
        
        // Perform collision checks between registered physics objects
        for (int32 i = 0; i < PhysicsObjects.Num(); ++i)
        {
            if (!IsValid(PhysicsObjects[i]))
            {
                continue;
            }
            
            CheckObjectCollisions(PhysicsObjects[i]);
        }
    }
}

void UPhysicsSystemManager::UpdateRagdollPhysics(float DeltaTime)
{
    // Update active ragdolls
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; --i)
    {
        FRagdollData& RagdollData = ActiveRagdolls[i];
        
        if (!IsValid(RagdollData.Character))
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        RagdollData.TimeActive += DeltaTime;
        
        // Deactivate ragdoll after timeout
        if (RagdollData.TimeActive >= RagdollDeactivationTime)
        {
            DeactivateRagdoll(RagdollData.Character);
            ActiveRagdolls.RemoveAt(i);
        }
    }
}

void UPhysicsSystemManager::UpdatePhysicsLOD(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update physics LOD based on distance from player
    for (AActor* PhysicsObject : PhysicsObjects)
    {
        if (!IsValid(PhysicsObject))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, PhysicsObject->GetActorLocation());
        
        // Disable physics for distant objects
        UPrimitiveComponent* PrimitiveComp = PhysicsObject->FindComponentByClass<UPrimitiveComponent>();
        if (PrimitiveComp)
        {
            bool bShouldSimulatePhysics = Distance <= PhysicsLODDistance;
            if (PrimitiveComp->IsSimulatingPhysics() != bShouldSimulatePhysics)
            {
                PrimitiveComp->SetSimulatePhysics(bShouldSimulatePhysics);
            }
        }
    }
}

void UPhysicsSystemManager::CleanupInactiveObjects(float DeltaTime)
{
    // Remove invalid objects from physics objects list
    for (int32 i = PhysicsObjects.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(PhysicsObjects[i]))
        {
            PhysicsObjects.RemoveAt(i);
        }
    }
}

void UPhysicsSystemManager::RegisterPhysicsObject(AActor* Actor)
{
    if (!Actor || PhysicsObjects.Contains(Actor))
    {
        return;
    }
    
    if (IsValidPhysicsActor(Actor))
    {
        PhysicsObjects.Add(Actor);
        
        // Set up physics properties
        UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimitiveComp)
        {
            // Apply default physics material properties
            PrimitiveComp->SetUseCCD(true); // Enable continuous collision detection
            
            // Set physics material if none exists
            if (!PrimitiveComp->GetMaterial(0))
            {
                ApplyDefaultPhysicsMaterial(PrimitiveComp);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered physics object: %s"), *Actor->GetName());
    }
}

void UPhysicsSystemManager::UnregisterPhysicsObject(AActor* Actor)
{
    if (Actor)
    {
        PhysicsObjects.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Unregistered physics object: %s"), *Actor->GetName());
    }
}

bool UPhysicsSystemManager::IsValidPhysicsActor(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Check if actor has physics-enabled components
    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimitiveComp && (PrimitiveComp->IsSimulatingPhysics() || PrimitiveComp->IsCollisionEnabled()))
    {
        return true;
    }
    
    // Check for skeletal mesh components (for ragdolls)
    USkeletalMeshComponent* SkeletalComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalComp)
    {
        return true;
    }
    
    return false;
}

void UPhysicsSystemManager::CheckObjectCollisions(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimitiveComp)
    {
        return;
    }
    
    // Get overlapping actors
    TArray<AActor*> OverlappingActors;
    Actor->GetOverlappingActors(OverlappingActors);
    
    for (AActor* OverlappingActor : OverlappingActors)
    {
        if (OverlappingActor && OverlappingActor != Actor)
        {
            HandleCollision(Actor, OverlappingActor);
        }
    }
}

void UPhysicsSystemManager::HandleCollision(AActor* ActorA, AActor* ActorB)
{
    // Basic collision handling - can be extended for specific game mechanics
    
    // Check if collision involves a character (for potential ragdoll activation)
    ACharacter* Character = Cast<ACharacter>(ActorA);
    if (!Character)
    {
        Character = Cast<ACharacter>(ActorB);
    }
    
    if (Character)
    {
        // Calculate impact force
        UPrimitiveComponent* CharacterComp = Character->FindComponentByClass<UPrimitiveComponent>();
        if (CharacterComp && CharacterComp->IsSimulatingPhysics())
        {
            FVector Velocity = CharacterComp->GetPhysicsLinearVelocity();
            float ImpactForce = Velocity.Size();
            
            // Activate ragdoll if impact is strong enough
            if (ImpactForce >= RagdollActivationThreshold)
            {
                ActivateRagdoll(Character);
            }
        }
    }
    
    // Log collision for debugging
    UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Collision between %s and %s"), 
           *ActorA->GetName(), *ActorB->GetName());
}

void UPhysicsSystemManager::ActivateRagdoll(ACharacter* Character)
{
    if (!Character || !bEnableRagdollPhysics)
    {
        return;
    }
    
    // Check if ragdoll is already active
    for (const FRagdollData& RagdollData : ActiveRagdolls)
    {
        if (RagdollData.Character == Character)
        {
            return; // Already active
        }
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (SkeletalMesh)
    {
        // Enable ragdoll physics
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Add to active ragdolls list
        FRagdollData NewRagdoll;
        NewRagdoll.Character = Character;
        NewRagdoll.TimeActive = 0.0f;
        ActiveRagdolls.Add(NewRagdoll);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Activated ragdoll for %s"), *Character->GetName());
    }
}

void UPhysicsSystemManager::DeactivateRagdoll(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (SkeletalMesh)
    {
        // Disable ragdoll physics
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Deactivated ragdoll for %s"), *Character->GetName());
    }
}

void UPhysicsSystemManager::ApplyDefaultPhysicsMaterial(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Set basic physics properties
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    if (BodyInstance)
    {
        // Apply default values for prehistoric survival game
        BodyInstance->SetUseCCD(true);
        BodyInstance->bLockXRotation = false;
        BodyInstance->bLockYRotation = false;
        BodyInstance->bLockZRotation = false;
        
        // Set mass based on component type
        if (Component->IsA<UStaticMeshComponent>())
        {
            BodyInstance->SetMassOverride(100.0f); // Default mass for static objects
        }
        else if (Component->IsA<USkeletalMeshComponent>())
        {
            BodyInstance->SetMassOverride(80.0f); // Default mass for characters
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied default physics material to %s"), 
           *Component->GetName());
}

void UPhysicsSystemManager::CreateDestructibleObject(AActor* Actor, float DestructionThreshold)
{
    if (!Actor || !bEnableDestructibleObjects)
    {
        return;
    }
    
    // Basic destructible object setup
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Enable physics and collision
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        // Set destruction threshold
        FBodyInstance* BodyInstance = MeshComp->GetBodyInstance();
        if (BodyInstance)
        {
            BodyInstance->SetUseCCD(true);
        }
        
        // Register as physics object
        RegisterPhysicsObject(Actor);
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Created destructible object: %s"), *Actor->GetName());
    }
}

FVector UPhysicsSystemManager::CalculateImpactForce(AActor* ActorA, AActor* ActorB) const
{
    if (!ActorA || !ActorB)
    {
        return FVector::ZeroVector;
    }
    
    UPrimitiveComponent* CompA = ActorA->FindComponentByClass<UPrimitiveComponent>();
    UPrimitiveComponent* CompB = ActorB->FindComponentByClass<UPrimitiveComponent>();
    
    if (!CompA || !CompB)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate relative velocity
    FVector VelocityA = CompA->GetPhysicsLinearVelocity();
    FVector VelocityB = CompB->GetPhysicsLinearVelocity();
    FVector RelativeVelocity = VelocityA - VelocityB;
    
    // Calculate impact direction
    FVector Direction = (ActorB->GetActorLocation() - ActorA->GetActorLocation()).GetSafeNormal();
    
    // Calculate impact force magnitude
    float Mass = CompA->GetMass();
    FVector ImpactForce = Direction * RelativeVelocity.Size() * Mass * 0.1f;
    
    return ImpactForce;
}

bool UPhysicsSystemManager::IsObjectInPhysicsRange(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return true; // If no player, assume in range
    }
    
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
    return Distance <= PhysicsLODDistance;
}

void UPhysicsSystemManager::SetPhysicsLODDistance(float NewDistance)
{
    PhysicsLODDistance = FMath::Max(NewDistance, 1000.0f); // Minimum 1000 units
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics LOD distance set to %f"), PhysicsLODDistance);
}

void UPhysicsSystemManager::SetMaxPhysicsObjects(int32 NewMax)
{
    MaxPhysicsObjects = FMath::Max(NewMax, 100); // Minimum 100 objects
    
    // Remove excess objects if necessary
    while (PhysicsObjects.Num() > MaxPhysicsObjects)
    {
        PhysicsObjects.RemoveAt(PhysicsObjects.Num() - 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Max physics objects set to %d"), MaxPhysicsObjects);
}

int32 UPhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    int32 ActiveCount = 0;
    
    for (AActor* Actor : PhysicsObjects)
    {
        if (IsValid(Actor) && IsObjectInPhysicsRange(Actor))
        {
            UPrimitiveComponent* PrimitiveComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
            {
                ActiveCount++;
            }
        }
    }
    
    return ActiveCount;
}

int32 UPhysicsSystemManager::GetActiveRagdollCount() const
{
    return ActiveRagdolls.Num();
}