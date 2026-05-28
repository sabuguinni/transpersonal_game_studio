#include "Core_PhysicsTestActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/CollisionProfile.h"

ACore_PhysicsTestActor::ACore_PhysicsTestActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(100.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

    // Create test mesh component
    TestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMesh"));
    TestMesh->SetupAttachment(CollisionSphere);
    TestMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TestMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

    // Load default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TestMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Set default physics properties
    bEnablePhysics = true;
    DestructionThreshold = 500.0f;
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    BiomeType = TEXT("Unknown");

    // Bind collision event
    CollisionSphere->OnComponentHit.AddDynamic(this, &ACore_PhysicsTestActor::OnHit);
}

void ACore_PhysicsTestActor::BeginPlay()
{
    Super::BeginPlay();

    // Enable physics simulation if requested
    if (bEnablePhysics && TestMesh)
    {
        TestMesh->SetSimulatePhysics(true);
        TestMesh->SetEnableGravity(true);
    }

    // Log initialization
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Physics Test Actor initialized in biome: %s"), *BiomeType));
    }
}

void ACore_PhysicsTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if actor has fallen below world bounds
    FVector CurrentLocation = GetActorLocation();
    if (CurrentLocation.Z < -1000.0f)
    {
        // Reset position to prevent infinite falling
        ResetTestActor();
    }
}

void ACore_PhysicsTestActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Calculate impact force
    float ImpactForce = NormalImpulse.Size();
    
    // Apply damage based on impact force
    if (ImpactForce > DestructionThreshold)
    {
        float DamageAmount = (ImpactForce - DestructionThreshold) * 0.1f;
        ApplyDamage(DamageAmount);

        // Log collision info
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                FString::Printf(TEXT("Physics Test: Impact force %.1f, Damage %.1f"), ImpactForce, DamageAmount));
        }
    }
}

void ACore_PhysicsTestActor::ApplyDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    // Check if destruction should be triggered
    if (CurrentHealth <= 0.0f)
    {
        TriggerDestruction();
    }
}

void ACore_PhysicsTestActor::TriggerDestruction()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
            FString::Printf(TEXT("Physics Test Actor destroyed in biome: %s"), *BiomeType));
    }

    // Simulate destruction by disabling collision and hiding mesh
    if (TestMesh)
    {
        TestMesh->SetVisibility(false);
        TestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        TestMesh->SetSimulatePhysics(false);
    }

    if (CollisionSphere)
    {
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Schedule reset after 5 seconds
    FTimerHandle ResetTimer;
    GetWorld()->GetTimerManager().SetTimer(ResetTimer, this, &ACore_PhysicsTestActor::ResetTestActor, 5.0f, false);
}

void ACore_PhysicsTestActor::ResetTestActor()
{
    // Reset health
    CurrentHealth = MaxHealth;

    // Reset mesh visibility and collision
    if (TestMesh)
    {
        TestMesh->SetVisibility(true);
        TestMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        if (bEnablePhysics)
        {
            TestMesh->SetSimulatePhysics(true);
        }
    }

    if (CollisionSphere)
    {
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Reset position to spawn location
    SetActorLocation(GetActorLocation() + FVector(0, 0, 500));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
            FString::Printf(TEXT("Physics Test Actor reset in biome: %s"), *BiomeType));
    }
}

bool ACore_PhysicsTestActor::IsPhysicsEnabled() const
{
    return bEnablePhysics && TestMesh && TestMesh->IsSimulatingPhysics();
}

void ACore_PhysicsTestActor::SetPhysicsEnabled(bool bEnabled)
{
    bEnablePhysics = bEnabled;
    
    if (TestMesh)
    {
        TestMesh->SetSimulatePhysics(bEnabled);
        TestMesh->SetEnableGravity(bEnabled);
    }
}

FString ACore_PhysicsTestActor::GetCollisionInfo() const
{
    FString Info = FString::Printf(TEXT("Biome: %s, Health: %.1f/%.1f, Physics: %s"),
        *BiomeType,
        CurrentHealth,
        MaxHealth,
        IsPhysicsEnabled() ? TEXT("Enabled") : TEXT("Disabled")
    );

    return Info;
}