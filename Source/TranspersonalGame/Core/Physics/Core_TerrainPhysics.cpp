#include "Core_TerrainPhysics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default terrain data
    TerrainData.TerrainType = ECore_TerrainType::Grassy;
    TerrainData.FrictionMultiplier = 1.0f;
    TerrainData.BounceMultiplier = 0.3f;
    TerrainData.MovementSpeedMultiplier = 1.0f;
    TerrainData.bCausesSlipping = false;
    TerrainData.bCausesStumbling = false;
    TerrainData.StaminaDrainMultiplier = 1.0f;
    
    DetectionRadius = 500.0f;
    bEnableTerrainEffects = true;
    bDebugVisualization = false;
    
    LastUpdateTime = 0.0f;
    UpdateInterval = 0.1f;
    CachedWorld = nullptr;
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    InitializeTerrainPhysics();
    
    if (ValidateTerrainData())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Component initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: Invalid terrain data detected"));
    }
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableTerrainEffects || !CachedWorld)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateTerrainEffects();
        LastUpdateTime = 0.0f;
    }
    
    // Debug visualization
    if (bDebugVisualization)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        DrawDebugSphere(CachedWorld, OwnerLocation, DetectionRadius, 16, FColor::Green, false, UpdateInterval + 0.01f);
    }
}

void UCore_TerrainPhysics::ApplyTerrainEffects(AActor* TargetActor)
{
    if (!TargetActor || !bEnableTerrainEffects)
    {
        return;
    }
    
    // Check if actor is a character for movement effects
    if (ACharacter* Character = Cast<ACharacter>(TargetActor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // Apply movement speed multiplier
            float BaseMaxWalkSpeed = 600.0f; // Default UE5 character speed
            MovementComp->MaxWalkSpeed = BaseMaxWalkSpeed * TerrainData.MovementSpeedMultiplier;
            
            // Apply friction effects
            MovementComp->GroundFriction = FMath::Clamp(MovementComp->GroundFriction * TerrainData.FrictionMultiplier, 0.1f, 8.0f);
            
            // Handle slipping terrain
            if (TerrainData.bCausesSlipping)
            {
                FVector SlipForce = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), 0.0f);
                Character->LaunchCharacter(SlipForce, false, false);
            }
            
            // Handle stumbling terrain
            if (TerrainData.bCausesStumbling && FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance per update
            {
                FVector StumbleForce = FVector(FMath::RandRange(-50.0f, 50.0f), FMath::RandRange(-50.0f, 50.0f), 0.0f);
                Character->LaunchCharacter(StumbleForce, false, false);
            }
        }
    }
    
    // Apply physics effects to other actors
    if (UPrimitiveComponent* PrimComp = TargetActor->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            // Adjust physics material properties
            PrimComp->SetPhysMaterialOverride(nullptr); // Reset to default, then apply custom effects
            
            // Apply terrain-specific forces
            FVector TerrainForce = FVector::ZeroVector;
            switch (TerrainData.TerrainType)
            {
                case ECore_TerrainType::Muddy:
                    TerrainForce = FVector(0.0f, 0.0f, -200.0f); // Downward force for mud
                    break;
                case ECore_TerrainType::Icy:
                    TerrainForce = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), 0.0f);
                    break;
                case ECore_TerrainType::Volcanic:
                    TerrainForce = FVector(0.0f, 0.0f, 150.0f); // Upward force for volcanic activity
                    break;
                default:
                    break;
            }
            
            if (!TerrainForce.IsZero())
            {
                PrimComp->AddForce(TerrainForce);
            }
        }
    }
    
    // Broadcast terrain effect event
    OnTerrainEffectApplied.Broadcast(TargetActor, TerrainData.TerrainType);
}

FCore_TerrainPhysicsData UCore_TerrainPhysics::GetTerrainDataAtLocation(FVector Location)
{
    // For now, return the current terrain data
    // In a full implementation, this would sample terrain data from a heightmap or world composition
    return TerrainData;
}

bool UCore_TerrainPhysics::IsActorOnTerrain(AActor* TargetActor)
{
    if (!TargetActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(TargetActor->GetActorLocation(), GetOwner()->GetActorLocation());
    return Distance <= DetectionRadius;
}

void UCore_TerrainPhysics::SetTerrainType(ECore_TerrainType NewTerrainType)
{
    TerrainData.TerrainType = NewTerrainType;
    
    // Update terrain properties based on type
    switch (NewTerrainType)
    {
        case ECore_TerrainType::Rocky:
            TerrainData.FrictionMultiplier = 1.2f;
            TerrainData.BounceMultiplier = 0.8f;
            TerrainData.MovementSpeedMultiplier = 0.8f;
            TerrainData.bCausesStumbling = true;
            TerrainData.StaminaDrainMultiplier = 1.3f;
            break;
            
        case ECore_TerrainType::Muddy:
            TerrainData.FrictionMultiplier = 0.4f;
            TerrainData.BounceMultiplier = 0.1f;
            TerrainData.MovementSpeedMultiplier = 0.5f;
            TerrainData.bCausesSlipping = true;
            TerrainData.StaminaDrainMultiplier = 1.8f;
            break;
            
        case ECore_TerrainType::Sandy:
            TerrainData.FrictionMultiplier = 0.7f;
            TerrainData.BounceMultiplier = 0.2f;
            TerrainData.MovementSpeedMultiplier = 0.7f;
            TerrainData.StaminaDrainMultiplier = 1.4f;
            break;
            
        case ECore_TerrainType::Icy:
            TerrainData.FrictionMultiplier = 0.2f;
            TerrainData.BounceMultiplier = 0.9f;
            TerrainData.MovementSpeedMultiplier = 0.3f;
            TerrainData.bCausesSlipping = true;
            TerrainData.StaminaDrainMultiplier = 1.1f;
            break;
            
        case ECore_TerrainType::Volcanic:
            TerrainData.FrictionMultiplier = 1.1f;
            TerrainData.BounceMultiplier = 0.4f;
            TerrainData.MovementSpeedMultiplier = 0.9f;
            TerrainData.bCausesStumbling = true;
            TerrainData.StaminaDrainMultiplier = 2.0f;
            break;
            
        case ECore_TerrainType::Grassy:
        default:
            TerrainData.FrictionMultiplier = 1.0f;
            TerrainData.BounceMultiplier = 0.3f;
            TerrainData.MovementSpeedMultiplier = 1.0f;
            TerrainData.bCausesSlipping = false;
            TerrainData.bCausesStumbling = false;
            TerrainData.StaminaDrainMultiplier = 1.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Terrain type changed to %d"), (int32)NewTerrainType);
}

float UCore_TerrainPhysics::CalculateMovementPenalty(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 1.0f;
    }
    
    float Penalty = 1.0f;
    
    // Base penalty from terrain type
    Penalty *= (2.0f - TerrainData.MovementSpeedMultiplier);
    
    // Additional penalty from stamina drain
    Penalty *= TerrainData.StaminaDrainMultiplier;
    
    // Clamp penalty to reasonable range
    return FMath::Clamp(Penalty, 0.1f, 5.0f);
}

void UCore_TerrainPhysics::ProcessTerrainCollision(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || !bEnableTerrainEffects)
    {
        return;
    }
    
    // Apply terrain effects on collision
    ApplyTerrainEffects(OtherActor);
    
    // Log collision for debugging
    if (bDebugVisualization)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Collision with %s"), *OtherActor->GetName());
    }
}

void UCore_TerrainPhysics::UpdateTerrainEffects()
{
    if (!CachedWorld || !GetOwner())
    {
        return;
    }
    
    // Clear previous nearby actors
    NearbyActors.Empty();
    
    // Find all actors within detection radius
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), OwnerLocation);
            if (Distance <= DetectionRadius)
            {
                NearbyActors.Add(Actor);
                CalculateTerrainInteraction(Actor);
            }
        }
    }
}

void UCore_TerrainPhysics::CalculateTerrainInteraction(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }
    
    // Apply terrain effects to the target actor
    ApplyTerrainEffects(TargetActor);
}

bool UCore_TerrainPhysics::ValidateTerrainData() const
{
    // Validate terrain data ranges
    if (TerrainData.FrictionMultiplier < 0.0f || TerrainData.FrictionMultiplier > 2.0f)
    {
        return false;
    }
    
    if (TerrainData.BounceMultiplier < 0.0f || TerrainData.BounceMultiplier > 2.0f)
    {
        return false;
    }
    
    if (TerrainData.MovementSpeedMultiplier < 0.0f || TerrainData.MovementSpeedMultiplier > 5.0f)
    {
        return false;
    }
    
    if (TerrainData.StaminaDrainMultiplier < 0.0f || TerrainData.StaminaDrainMultiplier > 100.0f)
    {
        return false;
    }
    
    return true;
}

void UCore_TerrainPhysics::InitializeTerrainPhysics()
{
    // Set up initial terrain physics state
    if (GetOwner())
    {
        // Bind to collision events if owner has primitive components
        TArray<UPrimitiveComponent*> PrimComponents;
        GetOwner()->GetComponents<UPrimitiveComponent>(PrimComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimComponents)
        {
            if (PrimComp)
            {
                PrimComp->OnComponentHit.AddDynamic(this, &UCore_TerrainPhysics::ProcessTerrainCollision);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Physics system initialized"));
}