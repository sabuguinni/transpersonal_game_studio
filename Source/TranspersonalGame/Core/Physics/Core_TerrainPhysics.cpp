#include "Core_TerrainPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / PhysicsUpdateRate;
    
    // Initialize default terrain properties
    InitializeTerrainProperties();
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with System Architect
    RegisterWithSystemArchitect();
    
    // Initialize performance tracking
    LastCleanupTime = GetWorld()->GetTimeSeconds();
    FrameCounter = 0;
    AverageFrameTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: System initialized with %d terrain types"), TerrainProperties.Num());
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRealtimePhysics)
        return;
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Process terrain collisions
    ProcessTerrainCollisions(DeltaTime);
    
    // Apply environmental effects
    if (GetOwner())
    {
        ApplyEnvironmentalEffects(GetOwner()->GetActorLocation(), DeltaTime);
    }
    
    // Cleanup inactive interactions periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCleanupTime > InteractionCleanupInterval)
    {
        CleanupInactiveInteractions();
        LastCleanupTime = CurrentTime;
    }
    
    // LOD optimization
    if (bEnableLODOptimization && GetOwner())
    {
        OptimizePhysicsLOD(GetOwner()->GetActorLocation());
    }
}

void UCore_TerrainPhysics::InitializeTerrainProperties()
{
    // Grass terrain
    FCore_TerrainPhysicsProperties GrassProps;
    GrassProps.Friction = 0.7f;
    GrassProps.Restitution = 0.3f;
    GrassProps.Density = 1.2f;
    GrassProps.MovementSpeedMultiplier = 1.0f;
    GrassProps.StabilityFactor = 0.9f;
    GrassProps.bCanSink = false;
    GrassProps.SinkRate = 0.0f;
    GrassProps.TemperatureEffect = 0.0f;
    TerrainProperties.Add(ECore_TerrainType::Grass, GrassProps);
    
    // Rock terrain
    FCore_TerrainPhysicsProperties RockProps;
    RockProps.Friction = 0.9f;
    RockProps.Restitution = 0.1f;
    RockProps.Density = 2.7f;
    RockProps.MovementSpeedMultiplier = 0.8f;
    RockProps.StabilityFactor = 1.0f;
    RockProps.bCanSink = false;
    RockProps.SinkRate = 0.0f;
    RockProps.TemperatureEffect = 0.0f;
    TerrainProperties.Add(ECore_TerrainType::Rock, RockProps);
    
    // Sand terrain
    FCore_TerrainPhysicsProperties SandProps;
    SandProps.Friction = 0.4f;
    SandProps.Restitution = 0.2f;
    SandProps.Density = 1.6f;
    SandProps.MovementSpeedMultiplier = 0.6f;
    SandProps.StabilityFactor = 0.5f;
    SandProps.bCanSink = true;
    SandProps.SinkRate = 0.1f;
    SandProps.TemperatureEffect = 0.2f;
    TerrainProperties.Add(ECore_TerrainType::Sand, SandProps);
    
    // Mud terrain
    FCore_TerrainPhysicsProperties MudProps;
    MudProps.Friction = 0.3f;
    MudProps.Restitution = 0.1f;
    MudProps.Density = 1.8f;
    MudProps.MovementSpeedMultiplier = 0.4f;
    MudProps.StabilityFactor = 0.3f;
    MudProps.bCanSink = true;
    MudProps.SinkRate = 0.3f;
    MudProps.TemperatureEffect = -0.1f;
    TerrainProperties.Add(ECore_TerrainType::Mud, MudProps);
    
    // Snow terrain
    FCore_TerrainPhysicsProperties SnowProps;
    SnowProps.Friction = 0.2f;
    SnowProps.Restitution = 0.4f;
    SnowProps.Density = 0.5f;
    SnowProps.MovementSpeedMultiplier = 0.5f;
    SnowProps.StabilityFactor = 0.4f;
    SnowProps.bCanSink = true;
    SnowProps.SinkRate = 0.2f;
    SnowProps.TemperatureEffect = -0.5f;
    TerrainProperties.Add(ECore_TerrainType::Snow, SnowProps);
    
    // Water terrain
    FCore_TerrainPhysicsProperties WaterProps;
    WaterProps.Friction = 0.1f;
    WaterProps.Restitution = 0.0f;
    WaterProps.Density = 1.0f;
    WaterProps.MovementSpeedMultiplier = 0.3f;
    WaterProps.StabilityFactor = 0.0f;
    WaterProps.bCanSink = true;
    WaterProps.SinkRate = 1.0f;
    WaterProps.TemperatureEffect = -0.3f;
    TerrainProperties.Add(ECore_TerrainType::Water, WaterProps);
    
    // Lava terrain
    FCore_TerrainPhysicsProperties LavaProps;
    LavaProps.Friction = 0.8f;
    LavaProps.Restitution = 0.0f;
    LavaProps.Density = 2.5f;
    LavaProps.MovementSpeedMultiplier = 0.1f;
    LavaProps.StabilityFactor = 0.0f;
    LavaProps.bCanSink = true;
    LavaProps.SinkRate = 2.0f;
    LavaProps.TemperatureEffect = 1.0f;
    TerrainProperties.Add(ECore_TerrainType::Lava, LavaProps);
    
    // Ice terrain
    FCore_TerrainPhysicsProperties IceProps;
    IceProps.Friction = 0.05f;
    IceProps.Restitution = 0.8f;
    IceProps.Density = 0.9f;
    IceProps.MovementSpeedMultiplier = 1.2f;
    IceProps.StabilityFactor = 0.2f;
    IceProps.bCanSink = false;
    IceProps.SinkRate = 0.0f;
    IceProps.TemperatureEffect = -0.8f;
    TerrainProperties.Add(ECore_TerrainType::Ice, IceProps);
}

ECore_TerrainType UCore_TerrainPhysics::DetectTerrainType(const FVector& Location)
{
    return SampleTerrainAtLocation(Location);
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysics::GetTerrainProperties(ECore_TerrainType TerrainType)
{
    if (TerrainProperties.Contains(TerrainType))
    {
        return TerrainProperties[TerrainType];
    }
    
    // Return default grass properties if type not found
    return TerrainProperties[ECore_TerrainType::Grass];
}

void UCore_TerrainPhysics::ApplyTerrainPhysics(AActor* Actor, const FVector& Location)
{
    if (!Actor)
        return;
    
    ECore_TerrainType TerrainType = DetectTerrainType(Location);
    ApplyTerrainSpecificPhysics(Actor, TerrainType);
    
    // Update terrain interaction
    UpdateTerrainInteraction(Actor, Location);
}

float UCore_TerrainPhysics::CalculateMovementModifier(ECore_TerrainType TerrainType, float BaseSpeed)
{
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    return BaseSpeed * Props.MovementSpeedMultiplier;
}

void UCore_TerrainPhysics::UpdateTerrainInteraction(AActor* Actor, const FVector& ContactPoint)
{
    if (!Actor)
        return;
    
    // Check if we already have an interaction for this actor
    FCore_TerrainInteraction* ExistingInteraction = nullptr;
    for (FCore_TerrainInteraction& Interaction : ActiveInteractions)
    {
        if (Interaction.InteractingActor == Actor)
        {
            ExistingInteraction = &Interaction;
            break;
        }
    }
    
    ECore_TerrainType TerrainType = DetectTerrainType(ContactPoint);
    
    if (ExistingInteraction)
    {
        // Update existing interaction
        ExistingInteraction->ContactPoint = ContactPoint;
        ExistingInteraction->TerrainType = TerrainType;
        ExistingInteraction->Duration += GetWorld()->GetDeltaSeconds();
    }
    else if (ActiveInteractions.Num() < MaxSimultaneousInteractions)
    {
        // Create new interaction
        FCore_TerrainInteraction NewInteraction;
        NewInteraction.InteractingActor = Actor;
        NewInteraction.ContactPoint = ContactPoint;
        NewInteraction.TerrainType = TerrainType;
        NewInteraction.InteractionStrength = 1.0f;
        NewInteraction.Duration = 0.0f;
        
        ActiveInteractions.Add(NewInteraction);
    }
}

void UCore_TerrainPhysics::SetTerrainMaterialProperties(ECore_TerrainType TerrainType, float Friction, float Restitution)
{
    if (TerrainProperties.Contains(TerrainType))
    {
        TerrainProperties[TerrainType].Friction = FMath::Clamp(Friction, 0.0f, 2.0f);
        TerrainProperties[TerrainType].Restitution = FMath::Clamp(Restitution, 0.0f, 1.0f);
    }
}

void UCore_TerrainPhysics::ApplyEnvironmentalEffects(const FVector& Location, float DeltaTime)
{
    ECore_TerrainType TerrainType = DetectTerrainType(Location);
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    
    // Apply temperature effects
    if (FMath::Abs(Props.TemperatureEffect) > 0.01f)
    {
        // Temperature effects could affect character stats, equipment durability, etc.
        // This would integrate with survival systems
    }
    
    // Apply sinking effects
    if (Props.bCanSink && Props.SinkRate > 0.0f)
    {
        // Characters and objects could slowly sink into unstable terrain
        // This would require integration with character movement systems
    }
}

bool UCore_TerrainPhysics::IsTerrainStable(const FVector& Location, float ActorMass)
{
    ECore_TerrainType TerrainType = DetectTerrainType(Location);
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    
    float StabilityThreshold = Props.StabilityFactor * (100.0f / FMath::Max(ActorMass, 1.0f));
    return StabilityThreshold > 0.5f;
}

void UCore_TerrainPhysics::SimulateTerrainDeformation(const FVector& ImpactPoint, float Force, float Radius)
{
    // Simulate terrain deformation from impacts
    ECore_TerrainType TerrainType = DetectTerrainType(ImpactPoint);
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    
    if (Props.StabilityFactor < 0.8f) // Only deformable terrain
    {
        float DeformationAmount = Force * (1.0f - Props.StabilityFactor) * 0.01f;
        
        // Trigger Blueprint event for visual deformation
        OnTerrainDeformation(ImpactPoint, DeformationAmount);
        
        // Debug visualization
        if (GEngine && GEngine->bEnableOnScreenDebugMessages)
        {
            DrawDebugSphere(GetWorld(), ImpactPoint, Radius, 12, FColor::Orange, false, 2.0f);
        }
    }
}

void UCore_TerrainPhysics::UpdateGravityEffects(AActor* Actor, ECore_TerrainType TerrainType)
{
    if (!Actor)
        return;
    
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    
    // Modify gravity based on terrain density and properties
    float GravityModifier = Props.Density * GravityMultiplier;
    
    // Apply to character movement if applicable
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float DefaultGravity = MovementComp->GetGravityZ();
            MovementComp->GravityScale = GravityModifier;
        }
    }
}

void UCore_TerrainPhysics::ProcessTerrainCollisions(float DeltaTime)
{
    // Process all active terrain interactions
    for (FCore_TerrainInteraction& Interaction : ActiveInteractions)
    {
        if (Interaction.InteractingActor && IsValid(Interaction.InteractingActor))
        {
            ApplyTerrainSpecificPhysics(Interaction.InteractingActor, Interaction.TerrainType);
        }
    }
}

void UCore_TerrainPhysics::OptimizePhysicsLOD(const FVector& ViewerLocation)
{
    if (!GetOwner())
        return;
    
    float DistanceToViewer = FVector::Dist(GetOwner()->GetActorLocation(), ViewerLocation);
    
    int32 NewLODLevel = 0;
    if (DistanceToViewer > LODDistance3)
        NewLODLevel = 3;
    else if (DistanceToViewer > LODDistance2)
        NewLODLevel = 2;
    else if (DistanceToViewer > LODDistance1)
        NewLODLevel = 1;
    
    if (NewLODLevel != CurrentLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        // Adjust physics update rate based on LOD
        switch (CurrentLODLevel)
        {
        case 0: PrimaryComponentTick.TickInterval = 1.0f / PhysicsUpdateRate; break;
        case 1: PrimaryComponentTick.TickInterval = 1.0f / (PhysicsUpdateRate * 0.5f); break;
        case 2: PrimaryComponentTick.TickInterval = 1.0f / (PhysicsUpdateRate * 0.25f); break;
        case 3: PrimaryComponentTick.TickInterval = 1.0f / (PhysicsUpdateRate * 0.1f); break;
        }
    }
}

void UCore_TerrainPhysics::CleanupInactiveInteractions()
{
    // Remove interactions with invalid actors or old interactions
    ActiveInteractions.RemoveAll([](const FCore_TerrainInteraction& Interaction)
    {
        return !IsValid(Interaction.InteractingActor) || Interaction.Duration > 30.0f;
    });
}

int32 UCore_TerrainPhysics::GetActiveInteractionCount() const
{
    return ActiveInteractions.Num();
}

void UCore_TerrainPhysics::RegisterWithSystemArchitect()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            SystemArchitect = GameInstance->GetSubsystem<UEng_SystemArchitect>();
            if (SystemArchitect)
            {
                // Register as a Core layer system
                bRegisteredWithArchitect = true;
                UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: Successfully registered with System Architect"));
            }
        }
    }
}

void UCore_TerrainPhysics::ValidateSystemIntegrity()
{
    bool bSystemValid = true;
    
    // Validate terrain properties
    if (TerrainProperties.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysics: No terrain properties defined"));
        bSystemValid = false;
    }
    
    // Validate performance settings
    if (PhysicsUpdateRate <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysics: Invalid physics update rate"));
        bSystemValid = false;
    }
    
    // Validate LOD settings
    if (LODDistance1 >= LODDistance2 || LODDistance2 >= LODDistance3)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysics: Invalid LOD distance configuration"));
        bSystemValid = false;
    }
    
    if (bSystemValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: System integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysics: System integrity validation FAILED"));
    }
}

void UCore_TerrainPhysics::RunTerrainPhysicsTests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN PHYSICS SYSTEM TESTS ==="));
    
    // Test 1: Terrain type detection
    FVector TestLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    ECore_TerrainType DetectedType = DetectTerrainType(TestLocation);
    UE_LOG(LogTemp, Warning, TEXT("Test 1 - Terrain Detection: %s"), 
           *UEnum::GetValueAsString(DetectedType));
    
    // Test 2: Properties retrieval
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(DetectedType);
    UE_LOG(LogTemp, Warning, TEXT("Test 2 - Properties: Friction=%.2f, Restitution=%.2f, Speed=%.2f"), 
           Props.Friction, Props.Restitution, Props.MovementSpeedMultiplier);
    
    // Test 3: Stability check
    bool bStable = IsTerrainStable(TestLocation, 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Test 3 - Stability: %s"), bStable ? TEXT("STABLE") : TEXT("UNSTABLE"));
    
    // Test 4: Active interactions
    UE_LOG(LogTemp, Warning, TEXT("Test 4 - Active Interactions: %d"), GetActiveInteractionCount());
    
    // Test 5: System registration
    UE_LOG(LogTemp, Warning, TEXT("Test 5 - System Registration: %s"), 
           bRegisteredWithArchitect ? TEXT("REGISTERED") : TEXT("NOT REGISTERED"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN PHYSICS TESTS COMPLETE ==="));
}

void UCore_TerrainPhysics::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameCounter++;
    AverageFrameTime = ((AverageFrameTime * (FrameCounter - 1)) + DeltaTime) / FrameCounter;
    
    // Reset counter periodically to prevent overflow
    if (FrameCounter > 1000)
    {
        FrameCounter = 100;
        AverageFrameTime = DeltaTime;
    }
}

ECore_TerrainType UCore_TerrainPhysics::SampleTerrainAtLocation(const FVector& Location)
{
    // Simplified terrain sampling - in a real implementation, this would
    // sample landscape materials, height maps, or other terrain data
    
    // For now, use height-based heuristics
    float Height = Location.Z;
    
    if (Height < -500.0f)
        return ECore_TerrainType::Water;
    else if (Height < 0.0f)
        return ECore_TerrainType::Sand;
    else if (Height < 500.0f)
        return ECore_TerrainType::Grass;
    else if (Height < 1000.0f)
        return ECore_TerrainType::Rock;
    else if (Height < 2000.0f)
        return ECore_TerrainType::Snow;
    else
        return ECore_TerrainType::Ice;
}

void UCore_TerrainPhysics::ApplyTerrainSpecificPhysics(AActor* Actor, ECore_TerrainType TerrainType)
{
    if (!Actor)
        return;
    
    FCore_TerrainPhysicsProperties Props = GetTerrainProperties(TerrainType);
    
    // Apply physics properties to actor's primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            // Apply friction and restitution
            PrimComp->SetPhysMaterialOverride(nullptr); // Would set custom physics material here
        }
    }
    
    // Apply movement modifiers for characters
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float ModifiedSpeed = MovementComp->MaxWalkSpeed * Props.MovementSpeedMultiplier;
            MovementComp->MaxWalkSpeed = ModifiedSpeed;
        }
    }
    
    // Handle sinking terrain
    if (Props.bCanSink && Props.SinkRate > 0.0f)
    {
        FVector CurrentLocation = Actor->GetActorLocation();
        FVector NewLocation = CurrentLocation + FVector(0, 0, -Props.SinkRate * GetWorld()->GetDeltaSeconds());
        Actor->SetActorLocation(NewLocation);
        
        // Trigger unstable terrain event
        OnUnstableTerrainDetected(CurrentLocation, Actor);
    }
}