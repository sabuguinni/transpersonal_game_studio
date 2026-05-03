#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "Engine/StaticMeshActor.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize physics settings
    bEnablePhysicsSimulation = true;
    bEnableRagdollPhysics = true;
    bEnableDestructionPhysics = true;
    bEnableVehiclePhysics = true;
    
    // Default physics materials
    DefaultPhysicsMaterial = nullptr;
    StonePhysicsMaterial = nullptr;
    WoodPhysicsMaterial = nullptr;
    MetalPhysicsMaterial = nullptr;
    FleshPhysicsMaterial = nullptr;
    
    // Physics simulation parameters
    GlobalGravityScale = 1.0f;
    DefaultMass = 100.0f;
    DefaultLinearDamping = 0.1f;
    DefaultAngularDamping = 0.1f;
    
    // Destruction parameters
    DestructionThreshold = 1000.0f;
    MaxDestructiblePieces = 50;
    
    // Ragdoll parameters
    RagdollBlendTime = 0.5f;
    RagdollLifetime = 10.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager initialized"));
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePhysicsSystem();
    LoadPhysicsMaterials();
    SetupPhysicsCallbacks();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager BeginPlay completed"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnablePhysicsSimulation)
    {
        UpdatePhysicsSimulation(DeltaTime);
        UpdateRagdollSystems(DeltaTime);
        UpdateDestructionSystems(DeltaTime);
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
    
    // Set global physics settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
        PhysicsSettings->bSubstepping = true;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016f;
        PhysicsSettings->MaxSubsteps = 6;
        
        UE_LOG(LogTemp, Log, TEXT("Physics settings configured"));
    }
    
    // Initialize physics materials registry
    PhysicsMaterialsRegistry.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::LoadPhysicsMaterials()
{
    // Load default physics materials from engine content
    DefaultPhysicsMaterial = LoadObject<UPhysicalMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultPhysicalMaterial"));
    
    // Create custom physics materials for different surface types
    CreateStonePhysicsMaterial();
    CreateWoodPhysicsMaterial();
    CreateMetalPhysicsMaterial();
    CreateFleshPhysicsMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Physics materials loaded"));
}

void UCore_PhysicsSystemManager::CreateStonePhysicsMaterial()
{
    StonePhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("StonePhysicsMaterial"));
    if (StonePhysicsMaterial)
    {
        StonePhysicsMaterial->Friction = 0.8f;
        StonePhysicsMaterial->Restitution = 0.2f;
        StonePhysicsMaterial->Density = 2.7f; // Stone density g/cm³
        StonePhysicsMaterial->DestructibleDamageThresholdScale = 1.5f;
        
        PhysicsMaterialsRegistry.Add(ECore_PhysicsMaterialType::Stone, StonePhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Stone physics material created"));
    }
}

void UCore_PhysicsSystemManager::CreateWoodPhysicsMaterial()
{
    WoodPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("WoodPhysicsMaterial"));
    if (WoodPhysicsMaterial)
    {
        WoodPhysicsMaterial->Friction = 0.6f;
        WoodPhysicsMaterial->Restitution = 0.4f;
        WoodPhysicsMaterial->Density = 0.8f; // Wood density g/cm³
        WoodPhysicsMaterial->DestructibleDamageThresholdScale = 0.8f;
        
        PhysicsMaterialsRegistry.Add(ECore_PhysicsMaterialType::Wood, WoodPhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Wood physics material created"));
    }
}

void UCore_PhysicsSystemManager::CreateMetalPhysicsMaterial()
{
    MetalPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("MetalPhysicsMaterial"));
    if (MetalPhysicsMaterial)
    {
        MetalPhysicsMaterial->Friction = 0.4f;
        MetalPhysicsMaterial->Restitution = 0.1f;
        MetalPhysicsMaterial->Density = 7.8f; // Steel density g/cm³
        MetalPhysicsMaterial->DestructibleDamageThresholdScale = 2.0f;
        
        PhysicsMaterialsRegistry.Add(ECore_PhysicsMaterialType::Metal, MetalPhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Metal physics material created"));
    }
}

void UCore_PhysicsSystemManager::CreateFleshPhysicsMaterial()
{
    FleshPhysicsMaterial = NewObject<UPhysicalMaterial>(this, TEXT("FleshPhysicsMaterial"));
    if (FleshPhysicsMaterial)
    {
        FleshPhysicsMaterial->Friction = 0.7f;
        FleshPhysicsMaterial->Restitution = 0.1f;
        FleshPhysicsMaterial->Density = 1.0f; // Flesh density g/cm³
        FleshPhysicsMaterial->DestructibleDamageThresholdScale = 0.5f;
        
        PhysicsMaterialsRegistry.Add(ECore_PhysicsMaterialType::Flesh, FleshPhysicsMaterial);
        UE_LOG(LogTemp, Log, TEXT("Flesh physics material created"));
    }
}

void UCore_PhysicsSystemManager::SetupPhysicsCallbacks()
{
    // Setup collision callbacks for physics events
    UWorld* World = GetWorld();
    if (World)
    {
        // Register for physics collision events
        World->GetPhysicsScene()->GetPxScene()->setSimulationEventCallback(this);
        UE_LOG(LogTemp, Log, TEXT("Physics callbacks registered"));
    }
}

void UCore_PhysicsSystemManager::UpdatePhysicsSimulation(float DeltaTime)
{
    // Update physics simulation parameters
    if (PhysicsSimulationActors.Num() > 0)
    {
        for (TWeakObjectPtr<AActor> ActorPtr : PhysicsSimulationActors)
        {
            if (AActor* Actor = ActorPtr.Get())
            {
                UpdateActorPhysics(Actor, DeltaTime);
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdateRagdollSystems(float DeltaTime)
{
    // Update active ragdoll characters
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        FCore_RagdollData& RagdollData = ActiveRagdolls[i];
        RagdollData.RemainingLifetime -= DeltaTime;
        
        if (RagdollData.RemainingLifetime <= 0.0f || !RagdollData.Character.IsValid())
        {
            // Remove expired or invalid ragdolls
            ActiveRagdolls.RemoveAt(i);
        }
        else
        {
            // Update ragdoll physics
            UpdateRagdollPhysics(RagdollData, DeltaTime);
        }
    }
}

void UCore_PhysicsSystemManager::UpdateDestructionSystems(float DeltaTime)
{
    // Update destruction events and cleanup
    for (int32 i = PendingDestructions.Num() - 1; i >= 0; i--)
    {
        FCore_DestructionData& DestructionData = PendingDestructions[i];
        DestructionData.TimeToDestroy -= DeltaTime;
        
        if (DestructionData.TimeToDestroy <= 0.0f)
        {
            ProcessDestruction(DestructionData);
            PendingDestructions.RemoveAt(i);
        }
    }
}

void UCore_PhysicsSystemManager::UpdateActorPhysics(AActor* Actor, float DeltaTime)
{
    if (!Actor) return;
    
    // Update actor-specific physics properties
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (MeshComp->IsSimulatingPhysics())
        {
            // Apply custom physics updates
            FVector Velocity = MeshComp->GetPhysicsLinearVelocity();
            if (Velocity.Size() > MaxPhysicsVelocity)
            {
                // Clamp velocity to prevent physics explosions
                Velocity = Velocity.GetClampedToMaxSize(MaxPhysicsVelocity);
                MeshComp->SetPhysicsLinearVelocity(Velocity);
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdateRagdollPhysics(FCore_RagdollData& RagdollData, float DeltaTime)
{
    if (ACharacter* Character = RagdollData.Character.Get())
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            // Update ragdoll physics simulation
            if (MeshComp->IsSimulatingPhysics())
            {
                // Apply drag to ragdoll over time
                TArray<FName> BoneNames;
                MeshComp->GetBoneNames(BoneNames);
                
                for (const FName& BoneName : BoneNames)
                {
                    FVector BoneVelocity = MeshComp->GetBoneLinearVelocity(BoneName);
                    if (BoneVelocity.Size() > 0.1f)
                    {
                        // Apply gradual damping
                        BoneVelocity *= (1.0f - RagdollData.DampingFactor * DeltaTime);
                        MeshComp->SetBoneLinearVelocity(BoneName, BoneVelocity);
                    }
                }
            }
        }
    }
}

void UCore_PhysicsSystemManager::ProcessDestruction(const FCore_DestructionData& DestructionData)
{
    if (AActor* Actor = DestructionData.ActorToDestroy.Get())
    {
        // Create destruction effect
        CreateDestructionEffect(Actor, DestructionData.ImpactLocation, DestructionData.ImpactForce);
        
        // Destroy the actor
        Actor->Destroy();
        
        UE_LOG(LogTemp, Log, TEXT("Destruction processed for actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::CreateDestructionEffect(AActor* Actor, const FVector& Location, float Force)
{
    if (!Actor) return;
    
    // Create debris particles and physics objects
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp && MeshComp->GetStaticMesh())
    {
        // Spawn debris pieces
        int32 DebrisCount = FMath::Min(MaxDestructiblePieces, FMath::RandRange(5, 15));
        
        for (int32 i = 0; i < DebrisCount; i++)
        {
            CreateDebrisPiece(Location, Force, MeshComp->GetStaticMesh());
        }
    }
}

void UCore_PhysicsSystemManager::CreateDebrisPiece(const FVector& Location, float Force, UStaticMesh* OriginalMesh)
{
    UWorld* World = GetWorld();
    if (!World || !OriginalMesh) return;
    
    // Spawn debris actor
    AStaticMeshActor* DebrisActor = World->SpawnActor<AStaticMeshActor>();
    if (DebrisActor)
    {
        UStaticMeshComponent* DebrisMesh = DebrisActor->GetStaticMeshComponent();
        if (DebrisMesh)
        {
            // Set mesh and enable physics
            DebrisMesh->SetStaticMesh(OriginalMesh);
            DebrisMesh->SetSimulatePhysics(true);
            DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Apply random impulse
            FVector RandomDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(0.5f, 1.0f)
            ).GetSafeNormal();
            
            FVector Impulse = RandomDirection * Force * FMath::RandRange(0.5f, 1.5f);
            DebrisMesh->AddImpulseAtLocation(Impulse, Location);
            
            // Set debris lifetime
            DebrisActor->SetLifeSpan(10.0f);
        }
    }
}

void UCore_PhysicsSystemManager::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && !PhysicsSimulationActors.Contains(Actor))
    {
        PhysicsSimulationActors.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("Registered physics actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (Actor)
    {
        PhysicsSimulationActors.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("Unregistered physics actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsSystemManager::EnableRagdollForCharacter(ACharacter* Character, float Lifetime)
{
    if (!Character || !bEnableRagdollPhysics) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    // Enable ragdoll physics
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create ragdoll data
    FCore_RagdollData RagdollData;
    RagdollData.Character = Character;
    RagdollData.RemainingLifetime = (Lifetime > 0.0f) ? Lifetime : RagdollLifetime;
    RagdollData.DampingFactor = 0.95f;
    
    ActiveRagdolls.Add(RagdollData);
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll enabled for character: %s"), *Character->GetName());
}

void UCore_PhysicsSystemManager::ApplyPhysicsMaterial(UStaticMeshComponent* MeshComponent, ECore_PhysicsMaterialType MaterialType)
{
    if (!MeshComponent) return;
    
    UPhysicalMaterial** FoundMaterial = PhysicsMaterialsRegistry.Find(MaterialType);
    if (FoundMaterial && *FoundMaterial)
    {
        MeshComponent->SetPhysMaterialOverride(*FoundMaterial);
        UE_LOG(LogTemp, Log, TEXT("Applied physics material type %d to mesh component"), (int32)MaterialType);
    }
}

void UCore_PhysicsSystemManager::TriggerDestruction(AActor* Actor, const FVector& ImpactLocation, float ImpactForce, float Delay)
{
    if (!Actor || !bEnableDestructionPhysics) return;
    
    if (ImpactForce >= DestructionThreshold)
    {
        FCore_DestructionData DestructionData;
        DestructionData.ActorToDestroy = Actor;
        DestructionData.ImpactLocation = ImpactLocation;
        DestructionData.ImpactForce = ImpactForce;
        DestructionData.TimeToDestroy = FMath::Max(0.0f, Delay);
        
        PendingDestructions.Add(DestructionData);
        
        UE_LOG(LogTemp, Log, TEXT("Destruction triggered for actor: %s"), *Actor->GetName());
    }
}

bool UCore_PhysicsSystemManager::IsPhysicsSimulationEnabled() const
{
    return bEnablePhysicsSimulation;
}

void UCore_PhysicsSystemManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bEnablePhysicsSimulation = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Physics simulation %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

float UCore_PhysicsSystemManager::GetGlobalGravityScale() const
{
    return GlobalGravityScale;
}

void UCore_PhysicsSystemManager::SetGlobalGravityScale(float NewScale)
{
    GlobalGravityScale = FMath::Clamp(NewScale, 0.1f, 10.0f);
    
    // Update physics settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global gravity scale set to: %f"), GlobalGravityScale);
}