#include "Core_FluidDynamics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_FluidDynamics::UCore_FluidDynamics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    FluidType = ECore_FluidType::Water;
    FluidProperties = FCore_FluidProperties();
    WaterLevel = 0.0f;
    LastUpdateTime = 0.0f;
}

void UCore_FluidDynamics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFluidSystem();
}

void UCore_FluidDynamics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    // Update fluid flow
    UpdateFluidFlow(DeltaTime);
    
    // Find actors in fluid and apply effects
    ActorsInFluid.Empty();
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner() && IsActorInFluid(Actor))
            {
                ActorsInFluid.Add(Actor);
                ApplyFluidEffects(Actor, DeltaTime);
            }
        }
    }
}

void UCore_FluidDynamics::InitializeFluidSystem()
{
    UpdateFluidProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: Initialized fluid system with type: %d"), (int32)FluidType);
}

bool UCore_FluidDynamics::IsActorInFluid(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    // Check if actor is within fluid bounds
    bool bInBounds = (ActorLocation.X >= FluidBoundsMin.X && ActorLocation.X <= FluidBoundsMax.X) &&
                     (ActorLocation.Y >= FluidBoundsMin.Y && ActorLocation.Y <= FluidBoundsMax.Y) &&
                     (ActorLocation.Z >= FluidBoundsMin.Z && ActorLocation.Z <= FluidBoundsMax.Z);
    
    if (!bInBounds)
    {
        return false;
    }
    
    // Check if actor is below water level
    float FluidDepth = GetFluidDepthAtLocation(ActorLocation);
    return FluidDepth > 0.0f;
}

float UCore_FluidDynamics::GetFluidDepthAtLocation(const FVector& Location)
{
    // Calculate depth based on water level and location
    float FluidSurfaceZ = WaterLevel;
    float ActorZ = Location.Z;
    
    // Get actor bounds to determine immersion depth
    if (AActor* Owner = GetOwner())
    {
        FVector Origin, BoxExtent;
        Owner->GetActorBounds(false, Origin, BoxExtent);
        float ActorBottom = ActorZ - BoxExtent.Z;
        
        if (ActorBottom < FluidSurfaceZ)
        {
            return FMath::Min(FluidSurfaceZ - ActorBottom, BoxExtent.Z * 2.0f);
        }
    }
    
    return FMath::Max(0.0f, FluidSurfaceZ - ActorZ);
}

void UCore_FluidDynamics::ApplyBuoyancyForce(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    float SubmergedVolume = CalculateSubmergedVolume(Actor);
    if (SubmergedVolume > 0.0f)
    {
        FVector BuoyancyForce = CalculateBuoyancyForce(Actor, SubmergedVolume);
        
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->AddForce(BuoyancyForce);
        }
    }
}

void UCore_FluidDynamics::ApplyDragForce(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
        FVector DragForce = CalculateDragForce(Actor, Velocity);
        
        PrimComp->AddForce(-DragForce);
    }
}

void UCore_FluidDynamics::ApplyFlowForce(AActor* Actor)
{
    if (!Actor || FluidProperties.FlowSpeed <= 0.0f)
    {
        return;
    }
    
    float SubmergedVolume = CalculateSubmergedVolume(Actor);
    if (SubmergedVolume > 0.0f)
    {
        FVector FlowForce = FluidProperties.FlowDirection.GetSafeNormal() * 
                           FluidProperties.FlowSpeed * 
                           FluidProperties.Density * 
                           SubmergedVolume * 0.001f;
        
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->AddForce(FlowForce);
        }
    }
}

FVector UCore_FluidDynamics::CalculateBuoyancyForce(AActor* Actor, float SubmergedVolume)
{
    if (!Actor || SubmergedVolume <= 0.0f)
    {
        return FVector::ZeroVector;
    }
    
    // Buoyancy = Fluid Density * Gravity * Submerged Volume
    float BuoyancyMagnitude = FluidProperties.Density * FluidProperties.BuoyancyForce * SubmergedVolume * 0.000001f;
    
    return FVector(0, 0, BuoyancyMagnitude);
}

FVector UCore_FluidDynamics::CalculateDragForce(AActor* Actor, const FVector& Velocity)
{
    if (!Actor || Velocity.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }
    
    float SubmergedVolume = CalculateSubmergedVolume(Actor);
    if (SubmergedVolume <= 0.0f)
    {
        return FVector::ZeroVector;
    }
    
    // Drag = 0.5 * Density * Velocity^2 * Drag Coefficient * Cross-sectional Area
    float VelocitySquared = Velocity.SizeSquared();
    float CrossSectionalArea = FMath::Sqrt(SubmergedVolume) * 0.1f; // Approximation
    
    float DragMagnitude = 0.5f * FluidProperties.Density * VelocitySquared * 
                         FluidProperties.DragCoefficient * CrossSectionalArea * 0.000001f;
    
    return Velocity.GetSafeNormal() * DragMagnitude;
}

void UCore_FluidDynamics::SetFluidType(ECore_FluidType NewFluidType)
{
    FluidType = NewFluidType;
    UpdateFluidProperties();
}

void UCore_FluidDynamics::UpdateFluidProperties()
{
    // Set properties based on fluid type
    switch (FluidType)
    {
        case ECore_FluidType::Water:
            FluidProperties.Density = 1000.0f;
            FluidProperties.Viscosity = 0.001f;
            FluidProperties.DragCoefficient = 0.47f;
            break;
            
        case ECore_FluidType::Mud:
            FluidProperties.Density = 1800.0f;
            FluidProperties.Viscosity = 0.1f;
            FluidProperties.DragCoefficient = 1.2f;
            break;
            
        case ECore_FluidType::Quicksand:
            FluidProperties.Density = 2000.0f;
            FluidProperties.Viscosity = 1.0f;
            FluidProperties.DragCoefficient = 2.0f;
            break;
            
        case ECore_FluidType::Lava:
            FluidProperties.Density = 3000.0f;
            FluidProperties.Viscosity = 100.0f;
            FluidProperties.DragCoefficient = 0.8f;
            break;
            
        case ECore_FluidType::Tar:
            FluidProperties.Density = 1200.0f;
            FluidProperties.Viscosity = 10.0f;
            FluidProperties.DragCoefficient = 1.5f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: Updated properties for fluid type: %d"), (int32)FluidType);
}

float UCore_FluidDynamics::CalculateSubmergedVolume(AActor* Actor)
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    FVector Origin, BoxExtent;
    Actor->GetActorBounds(false, Origin, BoxExtent);
    
    float FluidDepth = GetFluidDepthAtLocation(Actor->GetActorLocation());
    if (FluidDepth <= 0.0f)
    {
        return 0.0f;
    }
    
    // Calculate submerged volume based on depth and actor bounds
    float ActorHeight = BoxExtent.Z * 2.0f;
    float SubmergedHeight = FMath::Min(FluidDepth, ActorHeight);
    float SubmergedRatio = SubmergedHeight / ActorHeight;
    
    // Approximate volume as box volume
    float TotalVolume = BoxExtent.X * BoxExtent.Y * BoxExtent.Z * 8.0f;
    return TotalVolume * SubmergedRatio;
}

void UCore_FluidDynamics::ApplyFluidEffects(AActor* Actor, float DeltaTime)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply buoyancy
    ApplyBuoyancyForce(Actor);
    
    // Apply drag
    ApplyDragForce(Actor);
    
    // Apply flow forces
    ApplyFlowForce(Actor);
    
    // Special effects for characters
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float FluidDepth = GetFluidDepthAtLocation(Character->GetActorLocation());
            
            if (FluidDepth > 50.0f) // Partially submerged
            {
                // Reduce movement speed in fluid
                float SpeedMultiplier = 1.0f - (FluidProperties.Viscosity * 0.1f);
                MovementComp->MaxWalkSpeed *= SpeedMultiplier;
                
                // Enable swimming if deeply submerged
                if (FluidDepth > 150.0f)
                {
                    MovementComp->SetMovementMode(MOVE_Swimming);
                }
            }
        }
    }
}

void UCore_FluidDynamics::UpdateFluidFlow(float DeltaTime)
{
    // Update flow direction and speed based on time
    // This could be enhanced with more complex flow patterns
    
    if (FluidProperties.FlowSpeed > 0.0f)
    {
        // Simple sinusoidal flow variation
        float FlowVariation = FMath::Sin(LastUpdateTime * 0.5f) * 0.2f;
        float CurrentFlowSpeed = FluidProperties.FlowSpeed * (1.0f + FlowVariation);
        
        // Update flow direction slightly over time for more realistic water movement
        FVector BaseFlow = FluidProperties.FlowDirection;
        float DirectionVariation = FMath::Sin(LastUpdateTime * 0.3f) * 0.1f;
        FVector FlowVariationVector = FVector(DirectionVariation, DirectionVariation * 0.5f, 0);
        
        FluidProperties.FlowDirection = (BaseFlow + FlowVariationVector).GetSafeNormal();
    }
}