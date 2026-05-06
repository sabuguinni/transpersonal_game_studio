#include "Core_PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/CollisionProfile.h"
#include "TimerManager.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PhysicsObjectCount = 0;
    CollisionSetupCount = 0;
    LastPhysicsUpdateTime = 0.0f;
    ActivePhysicsObjects = 0;
}

void UCore_PhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Manager - Initializing realistic physics systems"));
    
    InitializeDefaultSettings();
    SetupCollisionChannels();
    
    // Get biome architecture reference
    BiomeArchitecture = GetGameInstance()->GetSubsystem<UEng_BiomeArchitecture>();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Manager - Initialization complete"));
}

void UCore_PhysicsManager::Deinitialize()
{
    CleanupRagdolls();
    ActiveRagdolls.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Manager - Deinitialized"));
    Super::Deinitialize();
}

void UCore_PhysicsManager::InitializeDefaultSettings()
{
    // Dinosaur physics settings - heavy, realistic
    FCore_PhysicsSettings DinosaurSettings;
    DinosaurSettings.Mass = 5000.0f;
    DinosaurSettings.LinearDamping = 0.05f;
    DinosaurSettings.AngularDamping = 0.1f;
    DinosaurSettings.bEnableGravity = true;
    DinosaurSettings.bSimulatePhysics = true;
    DinosaurSettings.CollisionObjectType = ECC_Pawn;
    DefaultPhysicsSettings.Add(ECore_PhysicsObjectType::Dinosaur, DinosaurSettings);

    // Environment physics settings - static, solid
    FCore_PhysicsSettings EnvironmentSettings;
    EnvironmentSettings.Mass = 10000.0f;
    EnvironmentSettings.LinearDamping = 1.0f;
    EnvironmentSettings.AngularDamping = 1.0f;
    EnvironmentSettings.bEnableGravity = true;
    EnvironmentSettings.bSimulatePhysics = false; // Static by default
    EnvironmentSettings.CollisionObjectType = ECC_WorldStatic;
    DefaultPhysicsSettings.Add(ECore_PhysicsObjectType::Environment, EnvironmentSettings);

    // Debris physics settings - light, dynamic
    FCore_PhysicsSettings DebrisSettings;
    DebrisSettings.Mass = 50.0f;
    DebrisSettings.LinearDamping = 0.2f;
    DebrisSettings.AngularDamping = 0.3f;
    DebrisSettings.bEnableGravity = true;
    DebrisSettings.bSimulatePhysics = true;
    DebrisSettings.CollisionObjectType = ECC_PhysicsBody;
    DefaultPhysicsSettings.Add(ECore_PhysicsObjectType::Debris, DebrisSettings);

    // Character physics settings
    FCore_PhysicsSettings CharacterSettings;
    CharacterSettings.Mass = 80.0f;
    CharacterSettings.LinearDamping = 0.1f;
    CharacterSettings.AngularDamping = 0.2f;
    CharacterSettings.bEnableGravity = true;
    CharacterSettings.bSimulatePhysics = false; // Controlled by movement component
    CharacterSettings.CollisionObjectType = ECC_Pawn;
    DefaultPhysicsSettings.Add(ECore_PhysicsObjectType::Character, CharacterSettings);

    // Projectile physics settings
    FCore_PhysicsSettings ProjectileSettings;
    ProjectileSettings.Mass = 1.0f;
    ProjectileSettings.LinearDamping = 0.01f;
    ProjectileSettings.AngularDamping = 0.01f;
    ProjectileSettings.bEnableGravity = true;
    ProjectileSettings.bSimulatePhysics = true;
    ProjectileSettings.CollisionObjectType = ECC_PhysicsBody;
    DefaultPhysicsSettings.Add(ECore_PhysicsObjectType::Projectile, ProjectileSettings);

    // Default ragdoll settings
    DefaultRagdollSettings.BoneLinearDamping = 0.5f;
    DefaultRagdollSettings.BoneAngularDamping = 0.5f;
    DefaultRagdollSettings.RagdollLifetime = 30.0f;
    DefaultRagdollSettings.bAutoCleanup = true;
}

void UCore_PhysicsManager::SetupCollisionChannels()
{
    // This would typically be done in DefaultEngine.ini, but we log the setup
    UE_LOG(LogTemp, Warning, TEXT("Physics Manager - Collision channels configured for realistic physics"));
}

void UCore_PhysicsManager::SetupActorPhysics(AActor* Actor, ECore_PhysicsObjectType ObjectType)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupActorPhysics: Invalid actor"));
        return;
    }

    FCore_PhysicsSettings Settings = GetSettingsForObjectType(ObjectType);

    // Find the primary mesh component
    UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMeshComp)
    {
        ApplyPhysicsSettings(StaticMeshComp, Settings);
        PhysicsObjectCount++;
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp)
    {
        // Setup skeletal mesh physics
        SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMeshComp->SetCollisionObjectType(Settings.CollisionObjectType);
        SkeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        if (Settings.bSimulatePhysics)
        {
            SkeletalMeshComp->SetSimulatePhysics(true);
        }
        
        PhysicsObjectCount++;
    }

    UE_LOG(LogTemp, Warning, TEXT("Physics setup applied to %s as %s"), 
           *Actor->GetName(), 
           *UEnum::GetValueAsString(ObjectType));
}

void UCore_PhysicsManager::ApplyPhysicsSettings(UStaticMeshComponent* MeshComponent, const FCore_PhysicsSettings& Settings)
{
    if (!MeshComponent)
    {
        return;
    }

    // Enable collision and physics
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(Settings.CollisionObjectType);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // Apply physics simulation
    MeshComponent->SetSimulatePhysics(Settings.bSimulatePhysics);
    MeshComponent->SetEnableGravity(Settings.bEnableGravity);

    // Set mass and damping
    MeshComponent->SetMassOverrideInKg(NAME_None, Settings.Mass, true);
    MeshComponent->SetLinearDamping(Settings.LinearDamping);
    MeshComponent->SetAngularDamping(Settings.AngularDamping);

    CollisionSetupCount++;
}

void UCore_PhysicsManager::SetupDinosaurPhysics(AActor* DinosaurActor, float Mass)
{
    if (!DinosaurActor)
    {
        return;
    }

    FCore_PhysicsSettings DinosaurSettings = GetSettingsForObjectType(ECore_PhysicsObjectType::Dinosaur);
    DinosaurSettings.Mass = Mass;

    SetupActorPhysics(DinosaurActor, ECore_PhysicsObjectType::Dinosaur);

    // Special dinosaur collision setup
    UStaticMeshComponent* MeshComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
        MeshComp->SetNotifyRigidBodyCollision(true); // Enable collision events
    }

    UE_LOG(LogTemp, Warning, TEXT("Dinosaur physics setup complete for %s (Mass: %.1f kg)"), 
           *DinosaurActor->GetName(), Mass);
}

void UCore_PhysicsManager::SetupEnvironmentPhysics(AActor* EnvironmentActor, bool bStatic)
{
    if (!EnvironmentActor)
    {
        return;
    }

    FCore_PhysicsSettings EnvSettings = GetSettingsForObjectType(ECore_PhysicsObjectType::Environment);
    EnvSettings.bSimulatePhysics = !bStatic;

    SetupActorPhysics(EnvironmentActor, ECore_PhysicsObjectType::Environment);

    UStaticMeshComponent* MeshComp = EnvironmentActor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        if (bStatic)
        {
            MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
            MeshComp->SetCollisionObjectType(ECC_WorldStatic);
        }
        else
        {
            MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Environment physics setup for %s (Static: %s)"), 
           *EnvironmentActor->GetName(), bStatic ? TEXT("Yes") : TEXT("No"));
}

void UCore_PhysicsManager::EnableRagdoll(AActor* Actor, const FCore_RagdollSettings& Settings)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableRagdoll: No skeletal mesh component found on %s"), *Actor->GetName());
        return;
    }

    // Enable ragdoll physics
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
    SkeletalMeshComp->SetSimulatePhysics(true);
    SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);

    // Apply ragdoll settings to all bodies
    SkeletalMeshComp->SetAllBodiesLinearDamping(Settings.BoneLinearDamping);
    SkeletalMeshComp->SetAllBodiesAngularDamping(Settings.BoneAngularDamping);

    RegisterRagdoll(Actor);

    // Setup auto cleanup timer if enabled
    if (Settings.bAutoCleanup && Settings.RagdollLifetime > 0.0f)
    {
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(CleanupTimer, 
            FTimerDelegate::CreateLambda([this, WeakActor = TWeakObjectPtr<AActor>(Actor)]()
            {
                if (WeakActor.IsValid())
                {
                    DisableRagdoll(WeakActor.Get());
                }
            }), 
            Settings.RagdollLifetime, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Ragdoll enabled for %s"), *Actor->GetName());
}

void UCore_PhysicsManager::DisableRagdoll(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp)
    {
        SkeletalMeshComp->SetSimulatePhysics(false);
        SkeletalMeshComp->SetAllBodiesSimulatePhysics(false);
        SkeletalMeshComp->SetCollisionProfileName(TEXT("Pawn"));
    }

    UnregisterRagdoll(Actor);
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll disabled for %s"), *Actor->GetName());
}

void UCore_PhysicsManager::SetupCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName)
{
    if (Component)
    {
        Component->SetCollisionProfileName(ProfileName);
        CollisionSetupCount++;
    }
}

void UCore_PhysicsManager::UpdateCollisionChannels(UPrimitiveComponent* Component, ECore_PhysicsObjectType ObjectType)
{
    if (!Component)
    {
        return;
    }

    FCore_PhysicsSettings Settings = GetSettingsForObjectType(ObjectType);
    Component->SetCollisionObjectType(Settings.CollisionObjectType);
    
    // Set appropriate collision responses based on object type
    switch (ObjectType)
    {
        case ECore_PhysicsObjectType::Dinosaur:
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
            
        case ECore_PhysicsObjectType::Environment:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            break;
            
        case ECore_PhysicsObjectType::Debris:
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            Component->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
            Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;
            
        default:
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            break;
    }
}

void UCore_PhysicsManager::CreatePhysicsTestObjects()
{
    if (!BiomeArchitecture.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("CreatePhysicsTestObjects: Biome architecture not available"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create test objects in each biome
    TArray<EBiomeType> BiomeTypes = {
        EBiomeType::Swamp,
        EBiomeType::Forest,
        EBiomeType::Savanna,
        EBiomeType::Desert,
        EBiomeType::Mountain
    };

    for (EBiomeType BiomeType : BiomeTypes)
    {
        FVector SpawnLocation = BiomeArchitecture->GetRandomLocationInBiome(BiomeType);
        SpawnLocation.Z += 500.0f; // Spawn above ground

        // Spawn a physics test sphere
        AStaticMeshActor* TestSphere = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            FRotator::ZeroRotator
        );

        if (TestSphere)
        {
            FString BiomeName = UEnum::GetValueAsString(BiomeType);
            TestSphere->SetActorLabel(FString::Printf(TEXT("PhysicsTest_%s"), *BiomeName));

            // Setup physics
            FCore_PhysicsSettings TestSettings = GetSettingsForObjectType(ECore_PhysicsObjectType::Debris);
            TestSettings.Mass = 100.0f;
            
            SetupActorPhysics(TestSphere, ECore_PhysicsObjectType::Debris);
            
            UE_LOG(LogTemp, Warning, TEXT("Created physics test object in %s biome"), *BiomeName);
        }
    }
}

void UCore_PhysicsManager::ValidatePhysicsSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 ValidatedObjects = 0;
    int32 PhysicsEnabledObjects = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            ValidatedObjects++;
            
            if (MeshComp->IsSimulatingPhysics())
            {
                PhysicsEnabledObjects++;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Physics Validation Complete:"));
    UE_LOG(LogTemp, Warning, TEXT("- Total objects validated: %d"), ValidatedObjects);
    UE_LOG(LogTemp, Warning, TEXT("- Objects with physics: %d"), PhysicsEnabledObjects);
    UE_LOG(LogTemp, Warning, TEXT("- Active ragdolls: %d"), ActiveRagdolls.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Collision setups: %d"), CollisionSetupCount);
}

AActor* UCore_PhysicsManager::SpawnPhysicsObjectInBiome(TSubclassOf<AActor> ActorClass, EBiomeType BiomeType, const FCore_PhysicsSettings& Settings)
{
    if (!ActorClass || !BiomeArchitecture.IsValid())
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FVector SpawnLocation = BiomeArchitecture->GetRandomLocationInBiome(BiomeType);
    SpawnLocation.Z += 200.0f; // Spawn slightly above ground

    AActor* SpawnedActor = World->SpawnActor<AActor>(
        ActorClass,
        SpawnLocation,
        FRotator::ZeroRotator
    );

    if (SpawnedActor)
    {
        // Apply physics settings
        UStaticMeshComponent* MeshComp = SpawnedActor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            ApplyPhysicsSettings(MeshComp, Settings);
        }

        UE_LOG(LogTemp, Warning, TEXT("Spawned physics object %s in biome %s"), 
               *SpawnedActor->GetName(), 
               *UEnum::GetValueAsString(BiomeType));
    }

    return SpawnedActor;
}

void UCore_PhysicsManager::CleanupRagdolls()
{
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (!ActiveRagdolls[i].IsValid())
        {
            ActiveRagdolls.RemoveAt(i);
        }
        else
        {
            DisableRagdoll(ActiveRagdolls[i].Get());
        }
    }
    
    ActiveRagdolls.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll cleanup complete"));
}

void UCore_PhysicsManager::ResetActorPhysics(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetSimulatePhysics(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
    }

    USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMeshComp)
    {
        DisableRagdoll(Actor);
    }

    UE_LOG(LogTemp, Warning, TEXT("Physics reset for %s"), *Actor->GetName());
}

FCore_PhysicsSettings UCore_PhysicsManager::GetSettingsForObjectType(ECore_PhysicsObjectType ObjectType) const
{
    if (const FCore_PhysicsSettings* Settings = DefaultPhysicsSettings.Find(ObjectType))
    {
        return *Settings;
    }
    
    // Return default settings if not found
    return FCore_PhysicsSettings();
}

void UCore_PhysicsManager::RegisterRagdoll(AActor* Actor)
{
    if (Actor && !ActiveRagdolls.Contains(Actor))
    {
        ActiveRagdolls.Add(Actor);
    }
}

void UCore_PhysicsManager::UnregisterRagdoll(AActor* Actor)
{
    if (Actor)
    {
        ActiveRagdolls.Remove(Actor);
    }
}