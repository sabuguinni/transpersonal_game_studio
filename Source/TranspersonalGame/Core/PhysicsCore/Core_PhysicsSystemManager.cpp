#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize physics settings for prehistoric survival
    PhysicsSettings.Gravity = FVector(0.0f, 0.0f, -980.0f); // Realistic gravity
    PhysicsSettings.LinearDamping = 0.01f;
    PhysicsSettings.AngularDamping = 0.01f;
    PhysicsSettings.MaxAngularVelocity = 3600.0f;
    
    // Terrain interaction settings
    TerrainSettings.SurfaceFriction = 0.7f;
    TerrainSettings.Restitution = 0.3f;
    TerrainSettings.MaxSlopeAngle = 45.0f;
    TerrainSettings.FootstepDepth = 2.0f;
    
    // Ragdoll settings for dinosaurs
    RagdollSettings.BlendWeight = 1.0f;
    RagdollSettings.bSimulatePhysics = true;
    RagdollSettings.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    
    // Destruction settings
    DestructionSettings.ImpactThreshold = 500.0f;
    DestructionSettings.MaxDebrisCount = 20;
    DestructionSettings.DebrisLifetime = 30.0f;
    
    bIsInitialized = false;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystem();
    SetupTerrainInteraction();
    RegisterPhysicsActors();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics System Manager initialized successfully"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized)
    {
        UpdatePhysicsSimulation(DeltaTime);
        ProcessTerrainInteractions(DeltaTime);
        UpdateRagdollSystems(DeltaTime);
        ProcessDestructionEvents(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get world for physics initialization"));
        return;
    }
    
    // Apply global physics settings
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->DefaultGravityZ = PhysicsSettings.Gravity.Z;
        Settings->DefaultTerminalVelocity = 4000.0f; // Realistic terminal velocity
        Settings->bSubstepping = true; // Enable substepping for accuracy
        Settings->MaxSubstepDeltaTime = 0.0083f; // 120 Hz substeps
        Settings->MaxSubsteps = 6;
    }
    
    // Initialize physics materials for different surfaces
    CreatePhysicsMaterials();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Physics system initialized with realistic prehistoric settings"));
}

void UCore_PhysicsSystemManager::SetupTerrainInteraction()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape actors for terrain interaction
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            // Configure landscape collision for realistic foot interaction
            ULandscapeHeightfieldCollisionComponent* CollisionComp = Landscape->GetLandscapeInfo()->GetLandscapeProxy()->CollisionComponent;
            if (CollisionComp)
            {
                CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
                
                // Apply terrain physics material
                if (TerrainPhysicsMaterial)
                {
                    CollisionComp->SetPhysMaterialOverride(TerrainPhysicsMaterial);
                }
            }
            
            RegisteredTerrainActors.Add(Landscape);
            UE_LOG(LogTemp, Log, TEXT("Registered landscape for terrain interaction: %s"), *Landscape->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::RegisterPhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Register all actors with physics components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                FCore_PhysicsActorData ActorData;
                ActorData.Actor = Actor;
                ActorData.PrimitiveComponent = PrimComp;
                ActorData.bIsRagdoll = Actor->IsA<ACharacter>();
                ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
                
                RegisteredPhysicsActors.Add(ActorData);
                UE_LOG(LogTemp, Log, TEXT("Registered physics actor: %s"), *Actor->GetName());
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Update physics for all registered actors
    for (FCore_PhysicsActorData& ActorData : RegisteredPhysicsActors)
    {
        if (IsValid(ActorData.Actor) && IsValid(ActorData.PrimitiveComponent))
        {
            // Apply environmental forces (wind, gravity variations)
            ApplyEnvironmentalForces(ActorData, DeltaTime);
            
            // Update physics properties based on conditions
            UpdatePhysicsProperties(ActorData, DeltaTime);
            
            ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UCore_PhysicsSystemManager::ProcessTerrainInteractions(float DeltaTime)
{
    for (const FCore_PhysicsActorData& ActorData : RegisteredPhysicsActors)
    {
        if (IsValid(ActorData.Actor))
        {
            // Check for ground contact and apply terrain effects
            FVector ActorLocation = ActorData.Actor->GetActorLocation();
            FVector GroundLocation;
            
            if (GetGroundLocationAtPosition(ActorLocation, GroundLocation))
            {
                float DistanceToGround = ActorLocation.Z - GroundLocation.Z;
                
                // Apply footstep depression for heavy actors
                if (DistanceToGround < 50.0f && ActorData.PrimitiveComponent->GetMass() > 100.0f)
                {
                    CreateFootstepDepression(GroundLocation, ActorData.PrimitiveComponent->GetMass());
                }
                
                // Apply surface-specific friction and effects
                ApplyTerrainEffects(ActorData, GroundLocation, DeltaTime);
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdateRagdollSystems(float DeltaTime)
{
    for (const FCore_PhysicsActorData& ActorData : RegisteredPhysicsActors)
    {
        if (ActorData.bIsRagdoll && IsValid(ActorData.Actor))
        {
            ACharacter* Character = Cast<ACharacter>(ActorData.Actor);
            if (Character && Character->GetMesh())
            {
                USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
                
                // Check if ragdoll should be activated
                if (ShouldActivateRagdoll(Character))
                {
                    ActivateRagdoll(SkeletalMesh);
                }
                else if (ShouldDeactivateRagdoll(Character))
                {
                    DeactivateRagdoll(SkeletalMesh);
                }
                
                // Update ragdoll physics if active
                if (SkeletalMesh->IsSimulatingPhysics())
                {
                    UpdateRagdollPhysics(SkeletalMesh, DeltaTime);
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::ProcessDestructionEvents(float DeltaTime)
{
    // Process pending destruction events
    for (int32 i = PendingDestructions.Num() - 1; i >= 0; i--)
    {
        FCore_DestructionEvent& Event = PendingDestructions[i];
        
        if (GetWorld()->GetTimeSeconds() - Event.EventTime > 0.1f) // Small delay for impact processing
        {
            ExecuteDestruction(Event);
            PendingDestructions.RemoveAt(i);
        }
    }
    
    // Clean up old debris
    CleanupOldDebris(DeltaTime);
}

void UCore_PhysicsSystemManager::CreatePhysicsMaterials()
{
    // Create terrain physics material
    TerrainPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("TerrainPhysicsMaterial"));
    if (TerrainPhysicsMaterial)
    {
        TerrainPhysicsMaterial->Friction = TerrainSettings.SurfaceFriction;
        TerrainPhysicsMaterial->Restitution = TerrainSettings.Restitution;
        TerrainPhysicsMaterial->Density = 2.0f; // Soil density
    }
    
    // Create rock physics material
    RockPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("RockPhysicsMaterial"));
    if (RockPhysicsMaterial)
    {
        RockPhysicsMaterial->Friction = 0.9f;
        RockPhysicsMaterial->Restitution = 0.1f;
        RockPhysicsMaterial->Density = 2.7f; // Rock density
    }
    
    // Create wood physics material
    WoodPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("WoodPhysicsMaterial"));
    if (WoodPhysicsMaterial)
    {
        WoodPhysicsMaterial->Friction = 0.6f;
        WoodPhysicsMaterial->Restitution = 0.4f;
        WoodPhysicsMaterial->Density = 0.8f; // Wood density
    }
}

void UCore_PhysicsSystemManager::ApplyEnvironmentalForces(FCore_PhysicsActorData& ActorData, float DeltaTime)
{
    if (!IsValid(ActorData.PrimitiveComponent)) return;
    
    // Apply wind force (simulated environmental effect)
    FVector WindForce = FVector(50.0f, 0.0f, 0.0f) * DeltaTime;
    ActorData.PrimitiveComponent->AddForce(WindForce);
    
    // Apply gravity variations based on altitude or special zones
    FVector GravityVariation = FVector(0.0f, 0.0f, -10.0f) * DeltaTime;
    ActorData.PrimitiveComponent->AddForce(GravityVariation);
}

void UCore_PhysicsSystemManager::UpdatePhysicsProperties(FCore_PhysicsActorData& ActorData, float DeltaTime)
{
    if (!IsValid(ActorData.PrimitiveComponent)) return;
    
    // Adjust physics properties based on environmental conditions
    float CurrentLinearDamping = ActorData.PrimitiveComponent->GetLinearDamping();
    float TargetDamping = PhysicsSettings.LinearDamping;
    
    // Increase damping in water or mud (simulated)
    FVector ActorLocation = ActorData.Actor->GetActorLocation();
    if (IsInWater(ActorLocation))
    {
        TargetDamping *= 5.0f; // Higher damping in water
    }
    
    // Smoothly interpolate to target damping
    float NewDamping = FMath::FInterpTo(CurrentLinearDamping, TargetDamping, DeltaTime, 2.0f);
    ActorData.PrimitiveComponent->SetLinearDamping(NewDamping);
}

bool UCore_PhysicsSystemManager::GetGroundLocationAtPosition(const FVector& Position, FVector& GroundLocation) const
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Perform line trace downward to find ground
    FVector Start = Position + FVector(0.0f, 0.0f, 100.0f);
    FVector End = Position - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        GroundLocation = HitResult.Location;
        return true;
    }
    
    return false;
}

void UCore_PhysicsSystemManager::CreateFootstepDepression(const FVector& Location, float ActorMass)
{
    // Create visual and physical footstep depression
    // This would typically involve modifying the landscape heightmap
    // For now, we log the event for debugging
    UE_LOG(LogTemp, Log, TEXT("Creating footstep depression at %s for mass %f"), *Location.ToString(), ActorMass);
}

void UCore_PhysicsSystemManager::ApplyTerrainEffects(const FCore_PhysicsActorData& ActorData, const FVector& GroundLocation, float DeltaTime)
{
    // Apply terrain-specific effects like mud slowing, ice sliding, etc.
    if (IsValid(ActorData.PrimitiveComponent))
    {
        // Example: Reduce velocity on soft terrain
        FVector Velocity = ActorData.PrimitiveComponent->GetPhysicsLinearVelocity();
        if (Velocity.Size() > 100.0f) // Only apply to fast-moving objects
        {
            FVector DampingForce = -Velocity * 0.1f * DeltaTime;
            ActorData.PrimitiveComponent->AddForce(DampingForce);
        }
    }
}

bool UCore_PhysicsSystemManager::ShouldActivateRagdoll(ACharacter* Character) const
{
    if (!Character) return false;
    
    // Activate ragdoll on death or high impact
    return Character->GetVelocity().Size() > 1000.0f; // High velocity impact
}

bool UCore_PhysicsSystemManager::ShouldDeactivateRagdoll(ACharacter* Character) const
{
    if (!Character) return false;
    
    // Deactivate ragdoll when velocity is low and stable
    return Character->GetVelocity().Size() < 50.0f;
}

void UCore_PhysicsSystemManager::ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh) return;
    
    SkeletalMesh->SetCollisionEnabled(RagdollSettings.CollisionEnabled);
    SkeletalMesh->SetSimulatePhysics(RagdollSettings.bSimulatePhysics);
    SkeletalMesh->WakeAllRigidBodies();
    
    UE_LOG(LogTemp, Log, TEXT("Activated ragdoll for %s"), *SkeletalMesh->GetOwner()->GetName());
}

void UCore_PhysicsSystemManager::DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh) return;
    
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SkeletalMesh->PutAllRigidBodiesToSleep();
    
    UE_LOG(LogTemp, Log, TEXT("Deactivated ragdoll for %s"), *SkeletalMesh->GetOwner()->GetName());
}

void UCore_PhysicsSystemManager::UpdateRagdollPhysics(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    if (!SkeletalMesh) return;
    
    // Apply additional forces to ragdoll for realistic behavior
    TArray<FName> BoneNames;
    SkeletalMesh->GetBoneNames(BoneNames);
    
    for (const FName& BoneName : BoneNames)
    {
        FVector BoneLocation = SkeletalMesh->GetBoneLocation(BoneName);
        
        // Apply wind resistance to individual bones
        FVector WindResistance = FVector(10.0f, 0.0f, 0.0f) * DeltaTime;
        SkeletalMesh->AddForceAtLocation(WindResistance, BoneLocation, BoneName);
    }
}

void UCore_PhysicsSystemManager::ExecuteDestruction(const FCore_DestructionEvent& Event)
{
    if (!IsValid(Event.Actor)) return;
    
    // Create debris pieces
    CreateDebris(Event.Actor, Event.ImpactLocation, Event.ImpactForce);
    
    // Destroy or hide the original actor
    Event.Actor->SetActorHiddenInGame(true);
    Event.Actor->SetActorEnableCollision(false);
    
    UE_LOG(LogTemp, Log, TEXT("Executed destruction for %s"), *Event.Actor->GetName());
}

void UCore_PhysicsSystemManager::CreateDebris(AActor* OriginalActor, const FVector& ImpactLocation, const FVector& ImpactForce)
{
    if (!OriginalActor) return;
    
    // Create multiple debris pieces
    int32 DebrisCount = FMath::Min(DestructionSettings.MaxDebrisCount, 10);
    
    for (int32 i = 0; i < DebrisCount; i++)
    {
        // Spawn debris actor (simplified - would use actual debris mesh)
        FVector DebrisLocation = ImpactLocation + FMath::VRand() * 100.0f;
        FRotator DebrisRotation = FRotator(FMath::RandRange(-180, 180), FMath::RandRange(-180, 180), FMath::RandRange(-180, 180));
        
        // This would spawn actual debris actors in a full implementation
        UE_LOG(LogTemp, Log, TEXT("Creating debris piece %d at %s"), i, *DebrisLocation.ToString());
    }
}

void UCore_PhysicsSystemManager::CleanupOldDebris(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove debris that has exceeded its lifetime
    for (int32 i = DebrisActors.Num() - 1; i >= 0; i--)
    {
        if (IsValid(DebrisActors[i]))
        {
            // Check if debris should be cleaned up based on lifetime
            if (CurrentTime - DebrisSpawnTimes[i] > DestructionSettings.DebrisLifetime)
            {
                DebrisActors[i]->Destroy();
                DebrisActors.RemoveAt(i);
                DebrisSpawnTimes.RemoveAt(i);
            }
        }
        else
        {
            DebrisActors.RemoveAt(i);
            DebrisSpawnTimes.RemoveAt(i);
        }
    }
}

bool UCore_PhysicsSystemManager::IsInWater(const FVector& Location) const
{
    // Simplified water detection - would check against water volumes in full implementation
    return Location.Z < 0.0f; // Below sea level
}

void UCore_PhysicsSystemManager::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!SelfActor || !OtherActor) return;
    
    float ImpactMagnitude = NormalImpulse.Size();
    
    // Check if impact is strong enough to cause destruction
    if (ImpactMagnitude > DestructionSettings.ImpactThreshold)
    {
        FCore_DestructionEvent Event;
        Event.Actor = SelfActor;
        Event.ImpactLocation = Hit.Location;
        Event.ImpactForce = NormalImpulse;
        Event.EventTime = GetWorld()->GetTimeSeconds();
        
        PendingDestructions.Add(Event);
        
        UE_LOG(LogTemp, Warning, TEXT("High impact detected: %f - queuing destruction for %s"), ImpactMagnitude, *SelfActor->GetName());
    }
}