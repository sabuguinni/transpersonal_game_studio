#include "DestructionPhysicsComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/BodySetup.h"

UDestructionPhysicsComponent::UDestructionPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default state
    CurrentState = ECore_DestructionState::Intact;
    AccumulatedDamage = 0.0f;
    
    // Initialize default settings
    DestructionSettings.DestructionType = ECore_DestructionType::Fracture;
    DestructionSettings.DestructionThreshold = 100.0f;
    DestructionSettings.MaxFragments = 20;
    DestructionSettings.FragmentMinSize = 0.1f;
    DestructionSettings.FragmentMaxSize = 1.0f;
    DestructionSettings.ExplosionForce = 500.0f;
    DestructionSettings.FragmentLifeTime = 15.0f;
    DestructionSettings.bGenerateDebris = true;
    DestructionSettings.bCreateDust = true;
    
    // Initialize component references
    StaticMeshComponent = nullptr;
    FragmentInstancedMesh = nullptr;
    OriginalMesh = nullptr;
    OriginalMaterial = nullptr;
    
    // Initialize effects
    DestructionEffect = nullptr;
    DestructionSound = nullptr;
    DamagedMaterial = nullptr;
}

void UDestructionPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the static mesh component on the owner
    StaticMeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    
    if (StaticMeshComponent)
    {
        // Store original mesh and material for restoration
        OriginalMesh = StaticMeshComponent->GetStaticMesh();
        if (StaticMeshComponent->GetNumMaterials() > 0)
        {
            OriginalMaterial = StaticMeshComponent->GetMaterial(0);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DestructionPhysicsComponent: No StaticMeshComponent found on owner %s"), 
               *GetOwner()->GetName());
    }
    
    // Create instanced mesh component for fragments if needed
    if (!FragmentInstancedMesh && DestructionSettings.bGenerateDebris)
    {
        FragmentInstancedMesh = NewObject<UInstancedStaticMeshComponent>(GetOwner());
        FragmentInstancedMesh->AttachToComponent(GetOwner()->GetRootComponent(), 
                                                FAttachmentTransformRules::KeepWorldTransform);
        FragmentInstancedMesh->RegisterComponent();
        FragmentInstancedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        FragmentInstancedMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    }
}

void UDestructionPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update destruction state logic
    UpdateDestructionState();
}

void UDestructionPhysicsComponent::TriggerDestruction(const FCore_DestructionImpact& Impact)
{
    if (CurrentState == ECore_DestructionState::Destroyed || CurrentState == ECore_DestructionState::Cleanup)
    {
        return; // Already destroyed
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Triggering destruction on %s"), *GetOwner()->GetName());
    
    // Execute the destruction
    ExecuteDestruction(Impact);
    
    // Broadcast destruction event
    OnDestructionTriggered.Broadcast(DestructionSettings.DestructionType, Impact);
    
    // Change state to destroyed
    CurrentState = ECore_DestructionState::Destroyed;
    OnDestructionStateChanged.Broadcast(CurrentState);
    
    // Schedule cleanup
    ScheduleCleanup();
}

void UDestructionPhysicsComponent::TriggerDestructionAtLocation(const FVector& Location, float Force, AActor* Causer)
{
    FCore_DestructionImpact Impact;
    Impact.ImpactLocation = Location;
    Impact.ImpactDirection = (Location - GetOwner()->GetActorLocation()).GetSafeNormal();
    Impact.ImpactForce = Force;
    Impact.ImpactCauser = Causer;
    
    TriggerDestruction(Impact);
}

void UDestructionPhysicsComponent::SetDestructionSettings(const FCore_DestructionSettings& NewSettings)
{
    DestructionSettings = NewSettings;
    
    // Regenerate fragments if settings changed significantly
    if (CustomFragments.Num() == 0)
    {
        GenerateProceduralFragments();
    }
}

void UDestructionPhysicsComponent::AddDamage(float DamageAmount, const FVector& DamageLocation)
{
    if (CurrentState == ECore_DestructionState::Destroyed)
    {
        return;
    }
    
    AccumulatedDamage += DamageAmount;
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Added %f damage. Total: %f/%f"), 
           DamageAmount, AccumulatedDamage, DestructionSettings.DestructionThreshold);
    
    // Update visual damage state
    if (CurrentState == ECore_DestructionState::Intact && AccumulatedDamage > DestructionSettings.DestructionThreshold * 0.3f)
    {
        CurrentState = ECore_DestructionState::Damaged;
        OnDestructionStateChanged.Broadcast(CurrentState);
        ApplyDamageVisuals();
    }
    
    // Check if we should trigger destruction
    if (ShouldTriggerDestruction())
    {
        FCore_DestructionImpact Impact;
        Impact.ImpactLocation = DamageLocation.IsZero() ? GetOwner()->GetActorLocation() : DamageLocation;
        Impact.ImpactDirection = FVector::UpVector;
        Impact.ImpactForce = DamageAmount;
        Impact.ImpactCauser = nullptr;
        
        TriggerDestruction(Impact);
    }
}

float UDestructionPhysicsComponent::GetDamagePercentage() const
{
    if (DestructionSettings.DestructionThreshold <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(AccumulatedDamage / DestructionSettings.DestructionThreshold, 0.0f, 1.0f);
}

void UDestructionPhysicsComponent::AddCustomFragment(const FCore_DestructionFragment& Fragment)
{
    CustomFragments.Add(Fragment);
}

void UDestructionPhysicsComponent::ClearFragments()
{
    CustomFragments.Empty();
    CleanupDebris();
}

void UDestructionPhysicsComponent::CleanupDebris()
{
    // Clean up spawned fragment actors
    for (AActor* Fragment : SpawnedFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    SpawnedFragments.Empty();
    
    // Clear instanced mesh instances
    if (FragmentInstancedMesh)
    {
        FragmentInstancedMesh->ClearInstances();
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Cleaned up debris"));
}

void UDestructionPhysicsComponent::PreviewDestruction()
{
    if (CurrentState == ECore_DestructionState::Destroyed)
    {
        return;
    }
    
    // Create a preview of destruction without actually destroying
    FCore_DestructionImpact PreviewImpact;
    PreviewImpact.ImpactLocation = GetOwner()->GetActorLocation();
    PreviewImpact.ImpactDirection = FVector::UpVector;
    PreviewImpact.ImpactForce = DestructionSettings.ExplosionForce;
    
    // Temporarily spawn fragments for preview
    SpawnFragments(PreviewImpact);
    
    // Schedule cleanup of preview fragments
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimerHandle,
        this,
        &UDestructionPhysicsComponent::CleanupDebris,
        2.0f,
        false
    );
}

void UDestructionPhysicsComponent::RestoreFromDestruction()
{
    if (CurrentState == ECore_DestructionState::Intact)
    {
        return;
    }
    
    // Clean up any debris
    CleanupDebris();
    
    // Restore original mesh and material
    if (StaticMeshComponent && OriginalMesh)
    {
        StaticMeshComponent->SetStaticMesh(OriginalMesh);
        StaticMeshComponent->SetVisibility(true);
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        if (OriginalMaterial)
        {
            StaticMeshComponent->SetMaterial(0, OriginalMaterial);
        }
    }
    
    // Reset state
    CurrentState = ECore_DestructionState::Intact;
    AccumulatedDamage = 0.0f;
    OnDestructionStateChanged.Broadcast(CurrentState);
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Restored from destruction"));
}

void UDestructionPhysicsComponent::GenerateFragments()
{
    CustomFragments.Empty();
    GenerateProceduralFragments();
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Generated %d fragments"), CustomFragments.Num());
}

void UDestructionPhysicsComponent::ExecuteDestruction(const FCore_DestructionImpact& Impact)
{
    if (!StaticMeshComponent)
    {
        return;
    }
    
    // Hide the original mesh
    StaticMeshComponent->SetVisibility(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Spawn fragments
    SpawnFragments(Impact);
    
    // Create destruction effects
    CreateDestructionEffects(Impact.ImpactLocation);
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Executed destruction"));
}

void UDestructionPhysicsComponent::SpawnFragments(const FCore_DestructionImpact& Impact)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Use custom fragments if available, otherwise generate procedural ones
    TArray<FCore_DestructionFragment> FragmentsToSpawn = CustomFragments;
    if (FragmentsToSpawn.Num() == 0)
    {
        GenerateProceduralFragments();
        FragmentsToSpawn = CustomFragments;
    }
    
    // Limit number of fragments for performance
    int32 FragmentCount = FMath::Min(FragmentsToSpawn.Num(), DestructionSettings.MaxFragments);
    
    for (int32 i = 0; i < FragmentCount; i++)
    {
        const FCore_DestructionFragment& Fragment = FragmentsToSpawn[i];
        
        // Calculate spawn location with some randomness
        FVector SpawnLocation = GetOwner()->GetActorLocation() + 
                               FMath::VRand() * 50.0f; // Random offset within 50 units
        
        // Calculate velocity based on impact and explosion force
        FVector Velocity = (SpawnLocation - Impact.ImpactLocation).GetSafeNormal() * DestructionSettings.ExplosionForce;
        Velocity += FMath::VRand() * 100.0f; // Add some randomness
        
        SpawnSingleFragment(Fragment, SpawnLocation, Velocity);
    }
}

void UDestructionPhysicsComponent::SpawnSingleFragment(const FCore_DestructionFragment& Fragment, const FVector& SpawnLocation, const FVector& Velocity)
{
    if (!GetWorld() || !Fragment.FragmentMesh)
    {
        return;
    }
    
    // Spawn a static mesh actor for the fragment
    AStaticMeshActor* FragmentActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
    if (!FragmentActor)
    {
        return;
    }
    
    // Configure the fragment
    UStaticMeshComponent* FragmentMesh = FragmentActor->GetStaticMeshComponent();
    if (FragmentMesh)
    {
        FragmentMesh->SetStaticMesh(Fragment.FragmentMesh);
        FragmentMesh->SetWorldScale3D(Fragment.Scale);
        
        if (Fragment.bSimulatePhysics)
        {
            FragmentMesh->SetSimulatePhysics(true);
            FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            FragmentMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            
            // Set mass and apply initial velocity
            if (FragmentMesh->GetBodyInstance())
            {
                FragmentMesh->GetBodyInstance()->SetMassOverride(Fragment.Mass, true);
                FragmentMesh->SetPhysicsLinearVelocity(Velocity);
                
                // Add some angular velocity for more realistic motion
                FVector AngularVelocity = FMath::VRand() * 360.0f;
                FragmentMesh->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
            }
        }
        
        // Copy material from original mesh
        if (OriginalMaterial)
        {
            FragmentMesh->SetMaterial(0, OriginalMaterial);
        }
    }
    
    // Add to spawned fragments list
    SpawnedFragments.Add(FragmentActor);
    
    // Broadcast fragment spawned event
    OnFragmentSpawned.Broadcast(FragmentActor);
    
    // Schedule fragment cleanup
    if (Fragment.LifeTime > 0.0f)
    {
        FTimerHandle FragmentCleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(
            FragmentCleanupTimer,
            [FragmentActor, this]()
            {
                if (IsValid(FragmentActor))
                {
                    SpawnedFragments.Remove(FragmentActor);
                    FragmentActor->Destroy();
                }
            },
            Fragment.LifeTime,
            false
        );
    }
}

void UDestructionPhysicsComponent::CreateDestructionEffects(const FVector& Location)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Spawn particle effect
    if (DestructionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionEffect, Location);
    }
    
    // Play destruction sound
    if (DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, Location);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Created destruction effects at %s"), *Location.ToString());
}

void UDestructionPhysicsComponent::UpdateDestructionState()
{
    // This function can be used for ongoing state management
    // For now, it's a placeholder for future functionality
}

void UDestructionPhysicsComponent::ScheduleCleanup()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Schedule cleanup after fragment lifetime
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimerHandle,
        this,
        &UDestructionPhysicsComponent::PerformCleanup,
        DestructionSettings.FragmentLifeTime + 5.0f, // Extra 5 seconds buffer
        false
    );
}

void UDestructionPhysicsComponent::PerformCleanup()
{
    CurrentState = ECore_DestructionState::Cleanup;
    OnDestructionStateChanged.Broadcast(CurrentState);
    
    CleanupDebris();
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Performed cleanup"));
}

void UDestructionPhysicsComponent::GenerateProceduralFragments()
{
    if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
    {
        return;
    }
    
    CustomFragments.Empty();
    
    // Get the bounds of the original mesh
    FBoxSphereBounds Bounds = StaticMeshComponent->GetStaticMesh()->GetBounds();
    FVector BaseSize = Bounds.BoxExtent * 2.0f; // Full size, not half-extents
    
    // Generate random fragments
    int32 NumFragments = FMath::RandRange(5, DestructionSettings.MaxFragments);
    
    for (int32 i = 0; i < NumFragments; i++)
    {
        FCore_DestructionFragment NewFragment = CreateRandomFragment(BaseSize);
        CustomFragments.Add(NewFragment);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Generated %d procedural fragments"), NumFragments);
}

FCore_DestructionFragment UDestructionPhysicsComponent::CreateRandomFragment(const FVector& BaseSize)
{
    FCore_DestructionFragment Fragment;
    
    // For now, use the original mesh as fragment mesh (in a real implementation, you'd create smaller meshes)
    Fragment.FragmentMesh = OriginalMesh;
    
    // Random scale between min and max size
    float ScaleFactor = FMath::RandRange(DestructionSettings.FragmentMinSize, DestructionSettings.FragmentMaxSize);
    Fragment.Scale = FVector(ScaleFactor);
    
    // Random mass based on scale
    Fragment.Mass = ScaleFactor * ScaleFactor * ScaleFactor; // Volume-based mass
    
    // Use settings for lifetime
    Fragment.LifeTime = DestructionSettings.FragmentLifeTime;
    Fragment.bSimulatePhysics = true;
    
    return Fragment;
}

UStaticMesh* UDestructionPhysicsComponent::CreateFragmentMesh(const FVector& Size)
{
    // This is a placeholder - in a real implementation, you would procedurally generate
    // or use pre-made fragment meshes based on the destruction type
    return OriginalMesh;
}

void UDestructionPhysicsComponent::ApplyDamageVisuals()
{
    if (!StaticMeshComponent)
    {
        return;
    }
    
    // Apply damaged material if available
    if (DamagedMaterial)
    {
        StaticMeshComponent->SetMaterial(0, DamagedMaterial);
    }
    else if (OriginalMaterial)
    {
        // Create a dynamic material instance to show damage
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, this);
        if (DynamicMaterial)
        {
            // Darken the material to show damage
            DynamicMaterial->SetScalarParameterValue(TEXT("DamageAmount"), GetDamagePercentage());
            StaticMeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DestructionPhysicsComponent: Applied damage visuals"));
}

bool UDestructionPhysicsComponent::ShouldTriggerDestruction() const
{
    return AccumulatedDamage >= DestructionSettings.DestructionThreshold;
}