#include "Core_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Landscape/LandscapeComponent.h"
#include "Engine/StaticMeshActor.h"

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    OptimizationLevel = 2;
    bEnableAdvancedCollision = true;
    MaxPhysicsObjects = 500;
    PhysicsTimeStep = 0.016667f; // 60 FPS
}

void UCore_PhysicsOptimizer::OptimizeCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("OptimizeCharacterPhysics: Invalid character"));
        return;
    }

    // Optimize capsule collision for terrain interaction
    UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
    if (CapsuleComp)
    {
        // Set optimal collision responses for terrain interaction
        CapsuleComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        
        // Optimize collision detection method
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        
        UE_LOG(LogTemp, Log, TEXT("Optimized character physics for %s"), *Character->GetName());
    }

    // Optimize skeletal mesh physics
    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (SkeletalMesh)
    {
        // Prepare for potential ragdoll activation
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkeletalMesh->SetCollisionObjectType(ECC_Pawn);
        
        // Optimize physics asset settings if available
        if (SkeletalMesh->GetPhysicsAsset())
        {
            SkeletalMesh->SetAllBodiesSimulatePhysics(false); // Start with physics disabled
            SkeletalMesh->SetAllBodiesCollisionObjectType(ECC_PhysicsBody);
        }
    }

    // Update performance metrics
    PerformanceMetrics.Add(FString::Printf(TEXT("Character_%s"), *Character->GetName()), 1.0f);
}

void UCore_PhysicsOptimizer::OptimizeLandscapePhysics(ALandscape* Landscape)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("OptimizeLandscapePhysics: Invalid landscape"));
        return;
    }

    // Get landscape components for optimization
    TArray<ULandscapeComponent*> LandscapeComponents;
    Landscape->GetLandscapeComponents(LandscapeComponents);

    for (ULandscapeComponent* Component : LandscapeComponents)
    {
        if (Component)
        {
            // Optimize collision settings for performance
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionObjectType(ECC_WorldStatic);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            
            // Special handling for character interaction
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Optimized landscape physics for %s with %d components"), 
           *Landscape->GetName(), LandscapeComponents.Num());

    // Update performance metrics
    PerformanceMetrics.Add(FString::Printf(TEXT("Landscape_%s"), *Landscape->GetName()), 
                          static_cast<float>(LandscapeComponents.Num()));
}

void UCore_PhysicsOptimizer::PrepareRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrepareRagdollPhysics: Invalid skeletal mesh"));
        return;
    }

    // Ensure physics asset is available
    if (!SkeletalMesh->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Warning, TEXT("PrepareRagdollPhysics: No physics asset found for %s"), 
               *SkeletalMesh->GetName());
        return;
    }

    // Configure for ragdoll activation
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetCollisionObjectType(ECC_PhysicsBody);
    
    // Set up collision responses for ragdoll
    SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    
    // Prepare physics bodies (but don't activate yet)
    SkeletalMesh->SetAllBodiesSimulatePhysics(false);
    SkeletalMesh->SetAllBodiesCollisionObjectType(ECC_PhysicsBody);
    
    UE_LOG(LogTemp, Log, TEXT("Prepared ragdoll physics for %s"), *SkeletalMesh->GetName());

    // Update performance metrics
    PerformanceMetrics.Add(FString::Printf(TEXT("Ragdoll_%s"), *SkeletalMesh->GetName()), 1.0f);
}

void UCore_PhysicsOptimizer::OptimizeDestructiblePhysics(UStaticMeshComponent* StaticMesh)
{
    if (!StaticMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("OptimizeDestructiblePhysics: Invalid static mesh"));
        return;
    }

    // Configure for destruction system
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMesh->SetCollisionObjectType(ECC_WorldDynamic);
    
    // Set up collision responses
    StaticMesh->SetCollisionResponseToAllChannels(ECR_Block);
    StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    
    // Enable physics simulation for destruction
    StaticMesh->SetSimulatePhysics(true);
    
    // Optimize mass and physics properties
    StaticMesh->SetMassOverrideInKg(NAME_None, 100.0f, true);
    
    UE_LOG(LogTemp, Log, TEXT("Optimized destructible physics for %s"), *StaticMesh->GetName());

    // Update performance metrics
    PerformanceMetrics.Add(FString::Printf(TEXT("Destructible_%s"), *StaticMesh->GetName()), 1.0f);
}

FString UCore_PhysicsOptimizer::GetPhysicsPerformanceReport()
{
    FString Report = TEXT("=== PHYSICS PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Optimization Level: %d\n"), OptimizationLevel);
    Report += FString::Printf(TEXT("Max Physics Objects: %d\n"), MaxPhysicsObjects);
    Report += FString::Printf(TEXT("Physics Time Step: %.6f\n"), PhysicsTimeStep);
    Report += TEXT("\nOptimized Objects:\n");

    for (const auto& Metric : PerformanceMetrics)
    {
        Report += FString::Printf(TEXT("- %s: %.2f\n"), *Metric.Key, Metric.Value);
    }

    // Calculate current physics load
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (World)
    {
        float PhysicsLoad = CalculatePhysicsLoad(World);
        Report += FString::Printf(TEXT("\nCurrent Physics Load: %.2f%%\n"), PhysicsLoad * 100.0f);
    }

    Report += TEXT("=== END REPORT ===");
    return Report;
}

void UCore_PhysicsOptimizer::BatchOptimizeWorldPhysics()
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BatchOptimizeWorldPhysics: No valid world found"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Starting batch physics optimization..."));

    int32 OptimizedCount = 0;

    // Optimize all characters
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character)
        {
            OptimizeCharacterPhysics(Character);
            OptimizedCount++;
        }
    }

    // Optimize all landscapes
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            OptimizeLandscapePhysics(Landscape);
            OptimizedCount++;
        }
    }

    // Optimize static mesh actors for destruction
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticMeshActor = *ActorItr;
        if (StaticMeshActor && StaticMeshActor->GetStaticMeshComponent())
        {
            OptimizeDestructiblePhysics(StaticMeshActor->GetStaticMeshComponent());
            OptimizedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Batch optimization complete. Optimized %d objects."), OptimizedCount);
}

bool UCore_PhysicsOptimizer::ValidateCollisionSettings(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return false;
    }

    // Check if collision is properly configured
    ECollisionEnabled::Type CollisionEnabled = Component->GetCollisionEnabled();
    if (CollisionEnabled == ECollisionEnabled::NoCollision)
    {
        return false;
    }

    // Validate collision object type
    ECollisionChannel ObjectType = Component->GetCollisionObjectType();
    return (ObjectType != ECC_MAX);
}

float UCore_PhysicsOptimizer::CalculatePhysicsLoad(UWorld* World)
{
    if (!World)
    {
        return 0.0f;
    }

    int32 PhysicsObjectCount = 0;
    int32 TotalObjectCount = 0;

    // Count physics-enabled objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TotalObjectCount++;
            
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    PhysicsObjectCount++;
                    break; // Count actor only once
                }
            }
        }
    }

    // Calculate load as percentage of max physics objects
    if (MaxPhysicsObjects > 0)
    {
        return FMath::Clamp(static_cast<float>(PhysicsObjectCount) / static_cast<float>(MaxPhysicsObjects), 0.0f, 1.0f);
    }

    return 0.0f;
}