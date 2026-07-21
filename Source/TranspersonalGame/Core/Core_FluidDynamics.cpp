#include "Core_FluidDynamics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_FluidDynamics::UCore_FluidDynamics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS

    // Initialize default fluid properties
    FluidProperties.Viscosity = 1.0f;
    FluidProperties.Density = 1000.0f;
    FluidProperties.FlowRate = 100.0f;
    FluidProperties.Temperature = 20.0f;
    FluidProperties.bIsContaminated = false;

    // Initialize default current data
    CurrentData.FlowDirection = FVector(1.0f, 0.0f, 0.0f);
    CurrentData.CurrentStrength = 50.0f;
    CurrentData.WaveHeight = 10.0f;
    CurrentData.WaveFrequency = 1.0f;
    CurrentData.bHasUndercurrent = false;

    FluidType = ECore_FluidType::Water;
    FluidLevel = 100.0f;
    bSimulateFlow = true;
    bAffectsCharacterMovement = true;
    DragCoefficient = 0.5f;
    CurrentFlowSpeed = 100.0f;
    bIsFluidSystemActive = false;
    LastUpdateTime = 0.0f;
    CurrentLODLevel = 0;
}

void UCore_FluidDynamics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFluidSystem();
}

void UCore_FluidDynamics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsFluidSystemActive && bSimulateFlow)
    {
        UpdateFluidFlow(DeltaTime);
        UpdateInternalFluidState(DeltaTime);
        ProcessFluidInteractions();
    }
}

void UCore_FluidDynamics::InitializeFluidSystem()
{
    bIsFluidSystemActive = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Set fluid properties based on type
    switch (FluidType)
    {
        case ECore_FluidType::Water:
            FluidProperties.Viscosity = 1.0f;
            FluidProperties.Density = 1000.0f;
            DragCoefficient = 0.5f;
            break;
            
        case ECore_FluidType::Mud:
            FluidProperties.Viscosity = 50.0f;
            FluidProperties.Density = 1400.0f;
            DragCoefficient = 2.0f;
            break;
            
        case ECore_FluidType::Quicksand:
            FluidProperties.Viscosity = 100.0f;
            FluidProperties.Density = 1600.0f;
            DragCoefficient = 5.0f;
            break;
            
        case ECore_FluidType::TarPit:
            FluidProperties.Viscosity = 1000.0f;
            FluidProperties.Density = 1200.0f;
            DragCoefficient = 10.0f;
            break;
            
        case ECore_FluidType::HotSpring:
            FluidProperties.Viscosity = 0.8f;
            FluidProperties.Density = 950.0f;
            FluidProperties.Temperature = 60.0f;
            DragCoefficient = 0.4f;
            break;
            
        case ECore_FluidType::BloodPool:
            FluidProperties.Viscosity = 3.0f;
            FluidProperties.Density = 1060.0f;
            DragCoefficient = 0.8f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: System initialized with type %d"), (int32)FluidType);
}

void UCore_FluidDynamics::UpdateFluidFlow(float DeltaTime)
{
    if (!bSimulateFlow) return;

    LastUpdateTime += DeltaTime;
    
    // Update wave simulation
    float WavePhase = LastUpdateTime * CurrentData.WaveFrequency;
    float CurrentWaveHeight = CurrentData.WaveHeight * FMath::Sin(WavePhase);
    
    // Update flow direction with wave influence
    FVector FlowVelocity = CurrentData.FlowDirection * CurrentData.CurrentStrength;
    FlowVelocity.Z += CurrentWaveHeight * 0.1f;
    
    // Apply environmental factors
    if (FluidType == ECore_FluidType::HotSpring)
    {
        // Hot springs have upward bubbling motion
        FlowVelocity.Z += FMath::Sin(LastUpdateTime * 2.0f) * 20.0f;
    }
    else if (FluidType == ECore_FluidType::TarPit)
    {
        // Tar pits have slow, viscous movement
        FlowVelocity *= 0.1f;
    }
}

void UCore_FluidDynamics::SimulateWaterCurrent(const FVector& InputForce)
{
    FVector NewFlowDirection = CurrentData.FlowDirection + (InputForce * 0.1f);
    NewFlowDirection.Normalize();
    
    CurrentData.FlowDirection = FMath::VInterpTo(CurrentData.FlowDirection, NewFlowDirection, GetWorld()->GetDeltaSeconds(), 2.0f);
    
    // Adjust current strength based on input force magnitude
    float ForceMagnitude = InputForce.Size();
    CurrentData.CurrentStrength = FMath::Clamp(CurrentData.CurrentStrength + ForceMagnitude * 0.1f, 10.0f, 200.0f);
}

void UCore_FluidDynamics::ApplyFluidForceToActor(AActor* TargetActor, float ForceMultiplier)
{
    if (!TargetActor || !bAffectsCharacterMovement) return;

    if (ACharacter* Character = Cast<ACharacter>(TargetActor))
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            FVector FluidForce = CalculateFluidForce(MovementComp->Velocity);
            FluidForce *= ForceMultiplier;
            
            // Apply drag based on fluid type
            float DragForce = CalculateFluidDrag(MovementComp->Velocity);
            FVector DragVector = -MovementComp->Velocity.GetSafeNormal() * DragForce;
            
            // Combine flow force and drag
            FVector TotalForce = FluidForce + DragVector;
            
            // Apply force to character
            MovementComp->AddForce(TotalForce);
            
            // Special effects for different fluid types
            if (FluidType == ECore_FluidType::Quicksand)
            {
                SimulateQuicksandSinking(TargetActor, 50.0f);
            }
            else if (FluidType == ECore_FluidType::TarPit)
            {
                SimulateTarPitEffect(TargetActor);
            }
        }
    }
}

bool UCore_FluidDynamics::IsActorInFluid(AActor* TestActor) const
{
    if (!TestActor) return false;
    
    return IsLocationInFluidBounds(TestActor->GetActorLocation());
}

float UCore_FluidDynamics::CalculateFluidDrag(const FVector& Velocity) const
{
    float VelocityMagnitude = Velocity.Size();
    float DragForce = 0.5f * FluidProperties.Density * DragCoefficient * VelocityMagnitude * VelocityMagnitude;
    
    // Scale by fluid viscosity
    DragForce *= FluidProperties.Viscosity;
    
    return FMath::Clamp(DragForce, 0.0f, 10000.0f);
}

void UCore_FluidDynamics::SetFluidProperties(const FCore_FluidProperties& NewProperties)
{
    FluidProperties = NewProperties;
    
    // Reinitialize system with new properties
    InitializeFluidSystem();
}

FCore_FluidProperties UCore_FluidDynamics::GetFluidProperties() const
{
    return FluidProperties;
}

void UCore_FluidDynamics::CreateRippleEffect(const FVector& ImpactLocation, float ImpactForce)
{
    // Create ripple effect by modifying wave parameters
    CurrentData.WaveHeight += ImpactForce * 0.1f;
    CurrentData.WaveFrequency += ImpactForce * 0.01f;
    
    // Clamp values to reasonable ranges
    CurrentData.WaveHeight = FMath::Clamp(CurrentData.WaveHeight, 5.0f, 100.0f);
    CurrentData.WaveFrequency = FMath::Clamp(CurrentData.WaveFrequency, 0.5f, 5.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: Ripple created at %s with force %f"), *ImpactLocation.ToString(), ImpactForce);
}

void UCore_FluidDynamics::SimulateRainEffect(float RainIntensity, const FVector& RainDirection)
{
    // Rain affects fluid level and current
    FluidLevel += RainIntensity * GetWorld()->GetDeltaSeconds() * 0.1f;
    
    // Rain creates downward current
    FVector RainInfluence = RainDirection * RainIntensity * 0.1f;
    CurrentData.FlowDirection = FMath::VInterpTo(CurrentData.FlowDirection, CurrentData.FlowDirection + RainInfluence, GetWorld()->GetDeltaSeconds(), 1.0f);
    
    // Increase wave activity during rain
    CurrentData.WaveHeight += RainIntensity * 0.05f;
    CurrentData.WaveFrequency += RainIntensity * 0.02f;
}

void UCore_FluidDynamics::SetupRiverFlow(const TArray<FVector>& FlowPath, float FlowSpeed)
{
    this->FlowPath = FlowPath;
    CurrentFlowSpeed = FlowSpeed;
    
    if (FlowPath.Num() >= 2)
    {
        // Calculate initial flow direction from path
        FVector FlowDir = (FlowPath[1] - FlowPath[0]).GetSafeNormal();
        CurrentData.FlowDirection = FlowDir;
        CurrentData.CurrentStrength = FlowSpeed;
        
        UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: River flow setup with %d points, speed %f"), FlowPath.Num(), FlowSpeed);
    }
}

void UCore_FluidDynamics::UpdateRiverCurrent(float DeltaTime)
{
    if (FlowPath.Num() < 2) return;
    
    // Update flow along the path
    for (int32 i = 0; i < FlowPath.Num() - 1; i++)
    {
        FVector SegmentDirection = (FlowPath[i + 1] - FlowPath[i]).GetSafeNormal();
        // River flow simulation would be more complex in a full implementation
    }
}

FVector UCore_FluidDynamics::GetCurrentVelocityAtLocation(const FVector& Location) const
{
    FVector Velocity = CurrentData.FlowDirection * CurrentData.CurrentStrength;
    
    // Add wave motion
    float WavePhase = LastUpdateTime * CurrentData.WaveFrequency;
    float WaveInfluence = FMath::Sin(WavePhase) * CurrentData.WaveHeight * 0.1f;
    Velocity.Z += WaveInfluence;
    
    return Velocity;
}

void UCore_FluidDynamics::SimulateTarPitEffect(AActor* TrappedActor)
{
    if (!TrappedActor) return;
    
    if (ACharacter* Character = Cast<ACharacter>(TrappedActor))
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // Drastically reduce movement speed in tar
            MovementComp->MaxWalkSpeed *= 0.1f;
            MovementComp->JumpZVelocity *= 0.2f;
            
            // Apply strong downward force
            MovementComp->AddForce(FVector(0.0f, 0.0f, -5000.0f));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_FluidDynamics: Actor %s trapped in tar pit"), *TrappedActor->GetName());
}

void UCore_FluidDynamics::SimulateQuicksandSinking(AActor* SinkingActor, float SinkRate)
{
    if (!SinkingActor) return;
    
    FVector CurrentLocation = SinkingActor->GetActorLocation();
    FVector SinkVector = FVector(0.0f, 0.0f, -SinkRate * GetWorld()->GetDeltaSeconds());
    
    SinkingActor->SetActorLocation(CurrentLocation + SinkVector);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_FluidDynamics: Actor %s sinking in quicksand at rate %f"), *SinkingActor->GetName(), SinkRate);
}

void UCore_FluidDynamics::CreateHotSpringBubbles(const FVector& SpringLocation)
{
    // Create bubble effect for hot springs
    for (int32 i = 0; i < 5; i++)
    {
        FVector BubbleLocation = SpringLocation + FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(0.0f, 50.0f)
        );
        
        // In a full implementation, this would spawn particle effects
        UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: Hot spring bubble at %s"), *BubbleLocation.ToString());
    }
}

void UCore_FluidDynamics::OptimizeFluidSimulation(int32 MaxParticles)
{
    // Optimize simulation based on performance requirements
    if (MaxParticles < 500)
    {
        SetFluidLOD(2); // Low detail
        PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS
    }
    else if (MaxParticles < 1000)
    {
        SetFluidLOD(1); // Medium detail
        PrimaryComponentTick.TickInterval = 0.025f; // 40 FPS
    }
    else
    {
        SetFluidLOD(0); // High detail
        PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamics: Optimized for %d particles, LOD %d"), MaxParticles, CurrentLODLevel);
}

void UCore_FluidDynamics::SetFluidLOD(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 2);
    
    // Adjust simulation complexity based on LOD
    switch (CurrentLODLevel)
    {
        case 0: // High detail
            CurrentData.WaveFrequency = 1.0f;
            break;
        case 1: // Medium detail
            CurrentData.WaveFrequency = 0.5f;
            break;
        case 2: // Low detail
            CurrentData.WaveFrequency = 0.25f;
            break;
    }
}

bool UCore_FluidDynamics::ValidateFluidSystem() const
{
    bool bIsValid = true;
    
    if (FluidProperties.Density <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_FluidDynamics: Invalid density %f"), FluidProperties.Density);
        bIsValid = false;
    }
    
    if (FluidProperties.Viscosity < 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_FluidDynamics: Invalid viscosity %f"), FluidProperties.Viscosity);
        bIsValid = false;
    }
    
    if (DragCoefficient < 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_FluidDynamics: Invalid drag coefficient %f"), DragCoefficient);
        bIsValid = false;
    }
    
    return bIsValid;
}

void UCore_FluidDynamics::UpdateInternalFluidState(float DeltaTime)
{
    // Update internal simulation state
    if (FluidType == ECore_FluidType::HotSpring)
    {
        // Hot springs have temperature effects
        FluidProperties.Temperature = 60.0f + FMath::Sin(LastUpdateTime) * 5.0f;
    }
    
    // Decay wave height over time
    CurrentData.WaveHeight = FMath::Max(CurrentData.WaveHeight - DeltaTime * 2.0f, 5.0f);
    CurrentData.WaveFrequency = FMath::Max(CurrentData.WaveFrequency - DeltaTime * 0.1f, 0.5f);
}

FVector UCore_FluidDynamics::CalculateFluidForce(const FVector& ActorVelocity) const
{
    FVector FlowForce = CurrentData.FlowDirection * CurrentData.CurrentStrength;
    
    // Apply buoyancy for different fluid types
    FVector BuoyancyForce = FVector(0.0f, 0.0f, FluidProperties.Density * 0.1f);
    
    return FlowForce + BuoyancyForce;
}

void UCore_FluidDynamics::ProcessFluidInteractions()
{
    // Process interactions with nearby actors
    UWorld* World = GetWorld();
    if (!World) return;
    
    FVector ComponentLocation = GetOwner()->GetActorLocation();
    float InteractionRadius = 1000.0f;
    
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsInRadiusOfClass(World, ComponentLocation, InteractionRadius, AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != GetOwner() && IsActorInFluid(Actor))
        {
            ApplyFluidForceToActor(Actor);
        }
    }
}

bool UCore_FluidDynamics::IsLocationInFluidBounds(const FVector& Location) const
{
    // Simple bounds check - in a full implementation this would be more sophisticated
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToFluid = FVector::Dist(Location, OwnerLocation);
    
    return DistanceToFluid <= 500.0f && Location.Z <= (OwnerLocation.Z + FluidLevel);
}

// Fluid Dynamics Actor Implementation
ACore_FluidDynamicsActor::ACore_FluidDynamicsActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create components
    FluidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FluidMesh"));
    RootComponent = FluidMesh;
    
    FluidDynamicsComponent = CreateDefaultSubobject<UCore_FluidDynamics>(TEXT("FluidDynamicsComponent"));
    
    FluidBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("FluidBounds"));
    FluidBounds->SetupAttachment(RootComponent);
    
    // Set default values
    bAutoStartSimulation = true;
    FluidDepth = 200.0f;
    FluidDimensions = FVector(1000.0f, 1000.0f, 200.0f);
}

void ACore_FluidDynamicsActor::BeginPlay()
{
    Super::BeginPlay();
    
    SetupFluidBounds(FluidDimensions);
    
    if (bAutoStartSimulation)
    {
        StartFluidSimulation();
    }
}

void ACore_FluidDynamicsActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Actor-specific fluid simulation updates
}

void ACore_FluidDynamicsActor::SetupFluidBounds(const FVector& Dimensions)
{
    FluidDimensions = Dimensions;
    
    if (FluidBounds)
    {
        FluidBounds->SetBoxExtent(Dimensions * 0.5f);
    }
    
    if (FluidMesh)
    {
        FluidMesh->SetWorldScale3D(Dimensions / 100.0f); // Scale to match dimensions
    }
}

void ACore_FluidDynamicsActor::StartFluidSimulation()
{
    if (FluidDynamicsComponent)
    {
        FluidDynamicsComponent->InitializeFluidSystem();
        UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamicsActor: Started fluid simulation"));
    }
}

void ACore_FluidDynamicsActor::StopFluidSimulation()
{
    if (FluidDynamicsComponent)
    {
        FluidDynamicsComponent->bSimulateFlow = false;
        UE_LOG(LogTemp, Log, TEXT("Core_FluidDynamicsActor: Stopped fluid simulation"));
    }
}

bool ACore_FluidDynamicsActor::IsFluidSimulationActive() const
{
    return FluidDynamicsComponent && FluidDynamicsComponent->bSimulateFlow;
}