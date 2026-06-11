#include "Core_PhysicsSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/PhysicalMaterial.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    GravityMultiplier = 1.0f;
    bPhysicsEnabled = true;
    MaxPhysicsSubsteps = 4;
    PhysicsTimestep = 0.016f;
    
    // Collision defaults
    DefaultDynamicProfile = "BlockAll";
    DefaultStaticProfile = "BlockAllDynamic";
    bUseComplexCollisionForStatic = true;
    
    // Ragdoll defaults
    bRagdollEnabled = true;
    RagdollTimeout = 10.0f;
    RagdollLinearDamping = 0.1f;
    RagdollAngularDamping = 0.1f;
    
    // Destruction defaults
    bDestructibleEnabled = true;
    MaxDestructibleChunks = 20;
    DebrisLifetime = 30.0f;
    
    // Initialize arrays
    ActiveRagdolls.Empty();
    RagdollTimestamps.Empty();
    ActiveDebris.Empty();
    DebrisTimestamps.Empty();
    
    CachedWorld = nullptr;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache world reference
    CachedWorld = GetWorld();
    
    // Initialize physics system
    InitializePhysicsSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics system initialized"));
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsEnabled || !CachedWorld)
    {
        return;
    }
    
    // Update ragdoll timeout system
    if (bRagdollEnabled && RagdollTimeout > 0.0f)
    {
        UpdateRagdollSystem(DeltaTime);
    }
    
    // Update debris cleanup system
    if (bDestructibleEnabled && DebrisLifetime > 0.0f)
    {
        UpdateDebrisSystem(DeltaTime);
    }
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: No valid world reference"));
        return;
    }
    
    // Load default physics materials
    LoadDefaultPhysicsMaterials();
    
    // Configure world physics settings
    ConfigureWorldPhysics();
    
    // Setup collision profiles
    SetupCollisionProfiles();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics system configuration complete"));
}

void UCore_PhysicsSystemManager::ApplyPhysicsSettings()
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Apply gravity settings
    AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
    if (WorldSettings)
    {
        float BaseGravity = -980.0f; // Standard Earth gravity in cm/s²
        WorldSettings->DefaultGravityZ = BaseGravity * GravityMultiplier;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied gravity multiplier %.2f (%.2f cm/s²)"), 
               GravityMultiplier, WorldSettings->DefaultGravityZ);
    }
    
    // Apply physics solver settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->MaxSubsteps = MaxPhysicsSubsteps;
        PhysicsSettings->FixedTimeStep = PhysicsTimestep;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Applied physics solver settings"));
    }
}

bool UCore_PhysicsSystemManager::EnableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh || !bRagdollEnabled)
    {
        return false;
    }
    
    // Check if physics asset exists
    if (!SkeletalMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: No physics asset on skeletal mesh %s"), 
               *SkeletalMesh->GetName());
        return false;
    }
    
    // Enable ragdoll physics
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->SetAllBodiesSimulatePhysics(true);
    
    // Apply damping settings
    SkeletalMesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
    SkeletalMesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    
    // Track ragdoll for timeout system
    if (RagdollTimeout > 0.0f)
    {
        ActiveRagdolls.Add(SkeletalMesh);
        RagdollTimestamps.Add(CachedWorld->GetTimeSeconds());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Enabled ragdoll on %s"), 
           *SkeletalMesh->GetName());
    
    return true;
}

bool UCore_PhysicsSystemManager::DisableRagdoll(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        return false;
    }
    
    // Disable ragdoll physics
    SkeletalMesh->SetSimulatePhysics(false);
    SkeletalMesh->SetAllBodiesSimulatePhysics(false);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    // Remove from tracking
    int32 Index = ActiveRagdolls.Find(SkeletalMesh);
    if (Index != INDEX_NONE)
    {
        ActiveRagdolls.RemoveAt(Index);
        RagdollTimestamps.RemoveAt(Index);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Disabled ragdoll on %s"), 
           *SkeletalMesh->GetName());
    
    return true;
}

AActor* UCore_PhysicsSystemManager::CreateDestructibleFromMesh(UStaticMesh* SourceMesh, FVector Location, FRotator Rotation)
{
    if (!SourceMesh || !bDestructibleEnabled || !CachedWorld)
    {
        return nullptr;
    }
    
    // Create basic destructible actor (placeholder implementation)
    // In a full implementation, this would create a proper destructible mesh
    AActor* DestructibleActor = CachedWorld->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
    if (DestructibleActor)
    {
        // Add static mesh component
        UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestructibleMesh"));
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(SourceMesh);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetSimulatePhysics(true);
            DestructibleActor->SetRootComponent(MeshComp);
        }
        
        // Track for cleanup
        ActiveDebris.Add(DestructibleActor);
        DebrisTimestamps.Add(CachedWorld->GetTimeSeconds());
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Created destructible from mesh %s"), 
               *SourceMesh->GetName());
    }
    
    return DestructibleActor;
}

void UCore_PhysicsSystemManager::ApplyPhysicsMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material)
{
    if (!Component || !Material)
    {
        return;
    }
    
    Component->SetPhysMaterialOverride(Material);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSystemManager: Applied physics material %s to %s"), 
           *Material->GetName(), *Component->GetName());
}

UPhysicalMaterial* UCore_PhysicsSystemManager::GetPhysicsMaterialBySurface(const FString& SurfaceType)
{
    FString LowerSurface = SurfaceType.ToLower();
    
    if (LowerSurface.Contains("organic") || LowerSurface.Contains("flesh") || LowerSurface.Contains("wood"))
    {
        return OrganicMaterial;
    }
    else if (LowerSurface.Contains("stone") || LowerSurface.Contains("rock") || LowerSurface.Contains("concrete"))
    {
        return StoneMaterial;
    }
    else if (LowerSurface.Contains("metal") || LowerSurface.Contains("steel") || LowerSurface.Contains("iron"))
    {
        return MetalMaterial;
    }
    else if (LowerSurface.Contains("water") || LowerSurface.Contains("liquid"))
    {
        return WaterMaterial;
    }
    
    // Default to organic material
    return OrganicMaterial;
}

void UCore_PhysicsSystemManager::GetPhysicsStats(int32& ActiveBodies, int32& Constraints, float& SimulationTime)
{
    // Placeholder implementation - would query actual physics world stats
    ActiveBodies = ActiveRagdolls.Num() + ActiveDebris.Num();
    Constraints = ActiveBodies * 2; // Rough estimate
    SimulationTime = PhysicsTimestep;
}

void UCore_PhysicsSystemManager::SetPhysicsProfilingEnabled(bool bEnabled)
{
    // Placeholder for physics profiling toggle
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics profiling %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsSystemManager::UpdateRagdollSystem(float DeltaTime)
{
    if (!CachedWorld)
    {
        return;
    }
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Check for expired ragdolls
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveRagdolls[i]) || (CurrentTime - RagdollTimestamps[i]) >= RagdollTimeout)
        {
            if (IsValid(ActiveRagdolls[i]))
            {
                DisableRagdoll(ActiveRagdolls[i]);
            }
            else
            {
                ActiveRagdolls.RemoveAt(i);
                RagdollTimestamps.RemoveAt(i);
            }
        }
    }
}

void UCore_PhysicsSystemManager::UpdateDebrisSystem(float DeltaTime)
{
    if (!CachedWorld)
    {
        return;
    }
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Check for expired debris
    for (int32 i = ActiveDebris.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveDebris[i]) || (CurrentTime - DebrisTimestamps[i]) >= DebrisLifetime)
        {
            if (IsValid(ActiveDebris[i]))
            {
                ActiveDebris[i]->Destroy();
            }
            ActiveDebris.RemoveAt(i);
            DebrisTimestamps.RemoveAt(i);
        }
    }
}

void UCore_PhysicsSystemManager::LoadDefaultPhysicsMaterials()
{
    // Load default physics materials from content
    // In a real implementation, these would be actual asset references
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Loading default physics materials"));
    
    // Placeholder - would load actual physics material assets
    OrganicMaterial = nullptr;
    StoneMaterial = nullptr;
    MetalMaterial = nullptr;
    WaterMaterial = nullptr;
}

void UCore_PhysicsSystemManager::ConfigureWorldPhysics()
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Apply current physics settings
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: World physics configured"));
}

void UCore_PhysicsSystemManager::SetupCollisionProfiles()
{
    // Setup custom collision profiles
    // This would typically be done through project settings or collision profile assets
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Collision profiles configured"));
}