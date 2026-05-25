#include "Core_PhysicsIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

DEFINE_LOG_CATEGORY(LogCore_PhysicsIntegration);

UCore_PhysicsIntegration::UCore_PhysicsIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default values
    TerrainAdaptationStrength = 1.0f;
    CollisionResponseMultiplier = 1.0f;
    MaxTerrainSlope = 45.0f;
    bEnableAdvancedPhysics = true;
    bEnableTerrainIK = true;
    
    // Initialize state
    CurrentGroundNormal = FVector::UpVector;
    CurrentTerrainSlope = 0.0f;
    bIsOnValidTerrain = true;
    bIsOnUnevenTerrain = false;
    
    // Internal state
    LastValidationTime = 0.0f;
    LastKnownGoodPosition = FVector::ZeroVector;
    bPhysicsValidated = false;
}

void UCore_PhysicsIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Core_PhysicsIntegration component initialized"));
    
    // Validate initial setup
    ValidatePhysicsSetup();
    
    // Store initial position
    if (AActor* Owner = GetOwner())
    {
        LastKnownGoodPosition = Owner->GetActorLocation();
    }
}

void UCore_PhysicsIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAdvancedPhysics)
    {
        return;
    }
    
    // Update physics state
    UpdatePhysicsState();
    
    // Update terrain adaptation
    if (bEnableTerrainIK)
    {
        UpdateTerrainAdaptation();
    }
    
    // Validate physics periodically
    LastValidationTime += DeltaTime;
    if (LastValidationTime > 1.0f) // Validate every second
    {
        ValidateTerrainInteraction();
        LastValidationTime = 0.0f;
    }
}

void UCore_PhysicsIntegration::UpdateTerrainAdaptation()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector ActorLocation = Owner->GetActorLocation();
    FVector TraceStart = ActorLocation + FVector(0, 0, 50);
    FVector TraceEnd = ActorLocation - FVector(0, 0, 200);
    
    FHitResult HitResult;
    if (PerformTerrainTrace(HitResult, TraceStart, TraceEnd))
    {
        CurrentGroundNormal = HitResult.Normal;
        
        // Calculate terrain slope
        float DotProduct = FVector::DotProduct(CurrentGroundNormal, FVector::UpVector);
        CurrentTerrainSlope = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Check if terrain is valid
        bIsOnValidTerrain = CurrentTerrainSlope <= MaxTerrainSlope;
        bIsOnUnevenTerrain = CurrentTerrainSlope > 5.0f;
        
        // Apply terrain adaptation if character
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                // Adjust movement based on terrain slope
                float SlopeMultiplier = FMath::Clamp(1.0f - (CurrentTerrainSlope / MaxTerrainSlope), 0.1f, 1.0f);
                MovementComp->MaxWalkSpeed = MovementComp->MaxWalkSpeed * SlopeMultiplier * TerrainAdaptationStrength;
            }
        }
    }
}

void UCore_PhysicsIntegration::ApplyPhysicsCorrection(const FVector& CorrectionVector)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector NewLocation = CurrentLocation + (CorrectionVector * CollisionResponseMultiplier);
    
    // Validate new position
    FHitResult HitResult;
    if (!PerformTerrainTrace(HitResult, NewLocation + FVector(0, 0, 50), NewLocation - FVector(0, 0, 200)))
    {
        // If new position is invalid, use last known good position
        NewLocation = LastKnownGoodPosition;
    }
    else
    {
        LastKnownGoodPosition = NewLocation;
    }
    
    Owner->SetActorLocation(NewLocation);
    
    UE_LOG(LogCore_PhysicsIntegration, VeryVerbose, TEXT("Applied physics correction: %s"), *CorrectionVector.ToString());
}

bool UCore_PhysicsIntegration::PerformTerrainTrace(FHitResult& OutHit, const FVector& StartLocation, const FVector& EndLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    
    return World->LineTraceSingleByChannel(
        OutHit,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
}

FVector UCore_PhysicsIntegration::CalculateTerrainNormal(const FVector& Location)
{
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 100);
    FVector TraceEnd = Location - FVector(0, 0, 200);
    
    if (PerformTerrainTrace(HitResult, TraceStart, TraceEnd))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

void UCore_PhysicsIntegration::HandleCollisionResponse(const FHitResult& HitResult)
{
    if (!HitResult.bBlockingHit)
    {
        return;
    }
    
    // Calculate response vector
    FVector ResponseVector = HitResult.Normal * CollisionResponseMultiplier;
    
    // Apply response
    ApplyPhysicsCorrection(ResponseVector);
    
    UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Handled collision response with %s"), 
           HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("Unknown"));
}

void UCore_PhysicsIntegration::EnablePhysicsSimulation(bool bEnable)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimComp->SetSimulatePhysics(bEnable);
        UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Physics simulation %s for %s"), 
               bEnable ? TEXT("enabled") : TEXT("disabled"), *Owner->GetName());
    }
}

void UCore_PhysicsIntegration::SetPhysicsProperties(float Mass, float LinearDamping, float AngularDamping)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
    {
        FBodyInstance* BodyInstance = PrimComp->GetBodyInstance();
        if (BodyInstance)
        {
            BodyInstance->SetMassOverride(Mass);
            BodyInstance->LinearDamping = LinearDamping;
            BodyInstance->AngularDamping = AngularDamping;
            
            UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Set physics properties - Mass: %f, LinearDamping: %f, AngularDamping: %f"), 
                   Mass, LinearDamping, AngularDamping);
        }
    }
}

void UCore_PhysicsIntegration::ApplyImpulseAtLocation(const FVector& Impulse, const FVector& Location)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimComp->AddImpulseAtLocation(Impulse, Location);
        UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Applied impulse %s at location %s"), 
               *Impulse.ToString(), *Location.ToString());
    }
}

void UCore_PhysicsIntegration::SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse Response)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
    {
        PrimComp->SetCollisionResponseToChannel(Channel, Response);
        UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Set collision response for channel %d to %d"), 
               (int32)Channel, (int32)Response);
    }
}

bool UCore_PhysicsIntegration::CheckPhysicsStability()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }
    
    // Check if actor is in a stable position
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector Velocity = Owner->GetVelocity();
    
    bool bIsStable = Velocity.Size() < 10.0f && bIsOnValidTerrain;
    
    if (bIsStable)
    {
        LastKnownGoodPosition = CurrentLocation;
    }
    
    return bIsStable;
}

void UCore_PhysicsIntegration::OptimizePhysicsPerformance()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Reduce physics update frequency for distant objects
    if (UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>())
    {
        // Get distance to player
        if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
            
            if (Distance > 5000.0f) // 50 meters
            {
                // Reduce physics complexity for distant objects
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            else
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
}

void UCore_PhysicsIntegration::ValidatePhysicsSetup()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogCore_PhysicsIntegration, Error, TEXT("No owner actor found for physics integration"));
        return;
    }
    
    // Check for required components
    UPrimitiveComponent* PrimComp = Owner->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp)
    {
        UE_LOG(LogCore_PhysicsIntegration, Warning, TEXT("No primitive component found on %s"), *Owner->GetName());
    }
    
    // Validate physics settings
    if (TerrainAdaptationStrength <= 0.0f)
    {
        UE_LOG(LogCore_PhysicsIntegration, Warning, TEXT("Invalid terrain adaptation strength: %f"), TerrainAdaptationStrength);
        TerrainAdaptationStrength = 1.0f;
    }
    
    if (MaxTerrainSlope <= 0.0f || MaxTerrainSlope > 90.0f)
    {
        UE_LOG(LogCore_PhysicsIntegration, Warning, TEXT("Invalid max terrain slope: %f"), MaxTerrainSlope);
        MaxTerrainSlope = 45.0f;
    }
    
    bPhysicsValidated = true;
    UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Physics setup validated for %s"), *Owner->GetName());
}

void UCore_PhysicsIntegration::TestPhysicsIntegration()
{
    UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Testing physics integration..."));
    
    // Test terrain trace
    FHitResult HitResult;
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector ActorLocation = Owner->GetActorLocation();
        bool bHit = PerformTerrainTrace(HitResult, ActorLocation + FVector(0, 0, 50), ActorLocation - FVector(0, 0, 200));
        
        UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Terrain trace test: %s"), bHit ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        if (bHit)
        {
            UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Ground normal: %s, Slope: %f degrees"), 
                   *HitResult.Normal.ToString(), CurrentTerrainSlope);
        }
    }
    
    // Test physics stability
    bool bStable = CheckPhysicsStability();
    UE_LOG(LogCore_PhysicsIntegration, Log, TEXT("Physics stability test: %s"), bStable ? TEXT("STABLE") : TEXT("UNSTABLE"));
}

void UCore_PhysicsIntegration::UpdatePhysicsState()
{
    // Update internal physics state
    if (IsPhysicsComponentValid())
    {
        LogPhysicsState();
    }
}

void UCore_PhysicsIntegration::ValidateTerrainInteraction()
{
    if (!bIsOnValidTerrain)
    {
        UE_LOG(LogCore_PhysicsIntegration, Warning, TEXT("Actor on invalid terrain (slope: %f degrees)"), CurrentTerrainSlope);
        
        // Apply correction if needed
        if (CurrentTerrainSlope > MaxTerrainSlope)
        {
            FVector CorrectionVector = CurrentGroundNormal * 100.0f;
            ApplyPhysicsCorrection(CorrectionVector);
        }
    }
}

bool UCore_PhysicsIntegration::IsPhysicsComponentValid() const
{
    return bPhysicsValidated && GetOwner() != nullptr;
}

void UCore_PhysicsIntegration::LogPhysicsState() const
{
    UE_LOG(LogCore_PhysicsIntegration, VeryVerbose, TEXT("Physics State - Terrain Valid: %s, Slope: %f, Uneven: %s"), 
           bIsOnValidTerrain ? TEXT("true") : TEXT("false"),
           CurrentTerrainSlope,
           bIsOnUnevenTerrain ? TEXT("true") : TEXT("false"));
}