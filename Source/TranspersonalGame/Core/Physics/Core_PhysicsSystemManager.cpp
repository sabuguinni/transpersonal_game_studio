#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    CurrentPhysicsSettings.GravityScale = 1.0f;
    CurrentPhysicsSettings.MaxPhysicsDeltaTime = 0.033f;
    CurrentPhysicsSettings.MaxSubsteps = 6;
    CurrentPhysicsSettings.PhysicsQuality = ECore_PhysicsQuality::High;
    CurrentPhysicsSettings.bEnableRagdollPhysics = true;
    CurrentPhysicsSettings.bEnableTerrainInteraction = true;
    
    CachedPhysicsSettings = nullptr;
    LastPhysicsFrameTime = 0.0f;
    LastPhysicsBodiesCount = 0;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Initializing physics system"));
    
    // Cache physics settings
    CachedPhysicsSettings = GetMutableDefault<UPhysicsSettings>();
    
    // Initialize physics configuration
    InitializePhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Deinitializing physics system"));
    
    // Clean up terrain physics map
    TerrainPhysicsMap.Empty();
    
    // Clean up ragdoll actors array
    RagdollEnabledActors.Empty();
    
    CachedPhysicsSettings = nullptr;
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSettings()
{
    if (!CachedPhysicsSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Physics settings not available"));
        return;
    }
    
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics settings initialized"));
    UE_LOG(LogTemp, Warning, TEXT("  - Gravity Scale: %f"), CurrentPhysicsSettings.GravityScale);
    UE_LOG(LogTemp, Warning, TEXT("  - Max Physics Delta Time: %f"), CurrentPhysicsSettings.MaxPhysicsDeltaTime);
    UE_LOG(LogTemp, Warning, TEXT("  - Physics Quality: %d"), (int32)CurrentPhysicsSettings.PhysicsQuality);
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality NewQuality)
{
    CurrentPhysicsSettings.PhysicsQuality = NewQuality;
    
    // Adjust physics settings based on quality
    switch (NewQuality)
    {
        case ECore_PhysicsQuality::Low:
            CurrentPhysicsSettings.MaxPhysicsDeltaTime = 0.05f;
            CurrentPhysicsSettings.MaxSubsteps = 2;
            break;
        case ECore_PhysicsQuality::Medium:
            CurrentPhysicsSettings.MaxPhysicsDeltaTime = 0.04f;
            CurrentPhysicsSettings.MaxSubsteps = 4;
            break;
        case ECore_PhysicsQuality::High:
            CurrentPhysicsSettings.MaxPhysicsDeltaTime = 0.033f;
            CurrentPhysicsSettings.MaxSubsteps = 6;
            break;
        case ECore_PhysicsQuality::Ultra:
            CurrentPhysicsSettings.MaxPhysicsDeltaTime = 0.02f;
            CurrentPhysicsSettings.MaxSubsteps = 8;
            break;
    }
    
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics quality set to %d"), (int32)NewQuality);
}

void UCore_PhysicsSystemManager::SetGravityScale(float NewGravityScale)
{
    CurrentPhysicsSettings.GravityScale = FMath::Clamp(NewGravityScale, 0.1f, 5.0f);
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Gravity scale set to %f"), CurrentPhysicsSettings.GravityScale);
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    CurrentPhysicsSettings = NewSettings;
    ApplyPhysicsSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Physics settings updated"));
}

void UCore_PhysicsSystemManager::RegisterTerrainActor(AActor* TerrainActor, const FCore_TerrainPhysicsData& PhysicsData)
{
    if (!TerrainActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Cannot register null terrain actor"));
        return;
    }
    
    TerrainPhysicsMap.Add(TerrainActor, PhysicsData);
    
    // Apply physics material to terrain components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    TerrainActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        if (MeshComp)
        {
            // Set collision properties based on terrain type
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Registered terrain actor %s with type %d"), 
           *TerrainActor->GetName(), (int32)PhysicsData.TerrainType);
}

void UCore_PhysicsSystemManager::UnregisterTerrainActor(AActor* TerrainActor)
{
    if (TerrainPhysicsMap.Remove(TerrainActor) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Unregistered terrain actor %s"), 
               *TerrainActor->GetName());
    }
}

FCore_TerrainPhysicsData UCore_PhysicsSystemManager::GetTerrainPhysicsData(AActor* TerrainActor) const
{
    if (const FCore_TerrainPhysicsData* Data = TerrainPhysicsMap.Find(TerrainActor))
    {
        return *Data;
    }
    
    // Return default terrain data
    FCore_TerrainPhysicsData DefaultData;
    DefaultData.TerrainType = ECore_TerrainType::Grass;
    DefaultData.Friction = 0.7f;
    DefaultData.Restitution = 0.3f;
    DefaultData.Density = 1.0f;
    DefaultData.bAffectsMovement = true;
    DefaultData.MovementSpeedMultiplier = 1.0f;
    
    return DefaultData;
}

ECore_TerrainType UCore_PhysicsSystemManager::GetTerrainTypeAtLocation(const FVector& WorldLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return ECore_TerrainType::Grass;
    }
    
    // Perform line trace to find terrain at location
    FHitResult HitResult;
    FVector TraceStart = WorldLocation + FVector(0, 0, 1000);
    FVector TraceEnd = WorldLocation - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            if (const FCore_TerrainPhysicsData* Data = TerrainPhysicsMap.Find(HitActor))
            {
                return Data->TerrainType;
            }
        }
    }
    
    return ECore_TerrainType::Grass;
}

void UCore_PhysicsSystemManager::EnableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Cannot enable ragdoll for null actor"));
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
        {
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            SkeletalMesh->SetSimulatePhysics(true);
            SkeletalMesh->WakeAllRigidBodies();
            
            // Disable character movement
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->DisableMovement();
            }
            
            RagdollEnabledActors.AddUnique(Actor);
            
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Enabled ragdoll for %s"), *Actor->GetName());
        }
    }
}

void UCore_PhysicsSystemManager::DisableRagdollForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
        {
            SkeletalMesh->SetSimulatePhysics(false);
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            SkeletalMesh->PutAllRigidBodiesToSleep();
            
            // Re-enable character movement
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->SetMovementMode(MOVE_Walking);
            }
            
            RagdollEnabledActors.Remove(Actor);
            
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Disabled ragdoll for %s"), *Actor->GetName());
        }
    }
}

bool UCore_PhysicsSystemManager::IsRagdollEnabled(AActor* Actor) const
{
    return RagdollEnabledActors.Contains(Actor);
}

bool UCore_PhysicsSystemManager::ValidatePhysicsSetup() const
{
    bool bIsValid = true;
    
    // Validate physics settings
    if (!CachedPhysicsSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Physics settings not available"));
        bIsValid = false;
    }
    
    // Validate world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: World not available"));
        bIsValid = false;
    }
    
    // Validate terrain actors
    for (const auto& TerrainPair : TerrainPhysicsMap)
    {
        if (!IsValid(TerrainPair.Key))
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSystemManager: Invalid terrain actor in registry"));
            bIsValid = false;
        }
    }
    
    return bIsValid;
}

TArray<FString> UCore_PhysicsSystemManager::GetPhysicsValidationErrors() const
{
    TArray<FString> Errors;
    
    if (!CachedPhysicsSettings)
    {
        Errors.Add(TEXT("Physics settings not available"));
    }
    
    if (!GetWorld())
    {
        Errors.Add(TEXT("World not available"));
    }
    
    for (const auto& TerrainPair : TerrainPhysicsMap)
    {
        if (!IsValid(TerrainPair.Key))
        {
            Errors.Add(FString::Printf(TEXT("Invalid terrain actor: %s"), 
                       TerrainPair.Key ? *TerrainPair.Key->GetName() : TEXT("NULL")));
        }
    }
    
    return Errors;
}

float UCore_PhysicsSystemManager::GetCurrentPhysicsFrameTime() const
{
    return LastPhysicsFrameTime;
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsBodies() const
{
    return LastPhysicsBodiesCount;
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    MonitorPhysicsPerformance();
    
    // Auto-adjust quality based on performance
    if (LastPhysicsFrameTime > 20.0f) // Over 20ms physics frame time
    {
        if (CurrentPhysicsSettings.PhysicsQuality != ECore_PhysicsQuality::Low)
        {
            ECore_PhysicsQuality NewQuality = static_cast<ECore_PhysicsQuality>(
                static_cast<int32>(CurrentPhysicsSettings.PhysicsQuality) - 1);
            SetPhysicsQuality(NewQuality);
            
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSystemManager: Auto-reduced physics quality due to performance"));
        }
    }
}

void UCore_PhysicsSystemManager::ApplyPhysicsSettings()
{
    if (!CachedPhysicsSettings)
    {
        return;
    }
    
    // Apply gravity scale to world settings
    UWorld* World = GetWorld();
    if (World && World->GetWorldSettings())
    {
        World->GetWorldSettings()->GlobalGravityZ = -980.0f * CurrentPhysicsSettings.GravityScale;
    }
    
    // Apply physics solver settings
    CachedPhysicsSettings->MaxPhysicsDeltaTime = CurrentPhysicsSettings.MaxPhysicsDeltaTime;
    CachedPhysicsSettings->MaxSubsteps = CurrentPhysicsSettings.MaxSubsteps;
    
    UpdateTerrainPhysicsMaterials();
}

void UCore_PhysicsSystemManager::UpdateTerrainPhysicsMaterials()
{
    // Update physics materials for registered terrain
    for (const auto& TerrainPair : TerrainPhysicsMap)
    {
        AActor* TerrainActor = TerrainPair.Key;
        const FCore_TerrainPhysicsData& PhysicsData = TerrainPair.Value;
        
        if (!IsValid(TerrainActor))
        {
            continue;
        }
        
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        TerrainActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp)
            {
                // Apply physics material properties
                // Note: In a full implementation, you would create and assign actual UPhysicalMaterial assets
                UE_LOG(LogTemp, Log, TEXT("Updating physics material for %s: Friction=%f, Restitution=%f"), 
                       *TerrainActor->GetName(), PhysicsData.Friction, PhysicsData.Restitution);
            }
        }
    }
}

void UCore_PhysicsSystemManager::MonitorPhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get physics frame time (simplified - in real implementation would use proper profiling)
    LastPhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active physics bodies (simplified)
    LastPhysicsBodiesCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    LastPhysicsBodiesCount++;
                }
            }
        }
    }
}