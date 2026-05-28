#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UCore_DestructionSystem::UCore_DestructionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    MaxHealth = 1000.0f;
    CurrentHealth = MaxHealth;
    bCanBeDestroyed = true;
    bIsDestroyed = false;
    
    // Set default destruction settings
    DestructionSettings.DestructionType = ECore_DestructionType::Fracture;
    DestructionSettings.DestructionThreshold = 500.0f;
    DestructionSettings.FragmentCount = 8;
    DestructionSettings.FragmentLifetime = 10.0f;
    DestructionSettings.bCreateDebris = true;
    DestructionSettings.bApplyImpulseToFragments = true;
}

void UCore_DestructionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDestruction();
    SetupCollisionResponse();
    
    // Reset health to max at start
    CurrentHealth = MaxHealth;
    bIsDestroyed = false;
}

void UCore_DestructionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor fragment cleanup
    if (CreatedFragments.Num() > 0)
    {
        // Remove null fragments
        CreatedFragments.RemoveAll([](AActor* Fragment) {
            return !IsValid(Fragment);
        });
    }
}

void UCore_DestructionSystem::InitializeDestruction()
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_DestructionSystem: No owner actor found"));
        return;
    }
    
    // Ensure health is properly initialized
    if (CurrentHealth <= 0.0f)
    {
        CurrentHealth = MaxHealth;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_DestructionSystem initialized for %s with %f health"), 
           *GetOwner()->GetName(), CurrentHealth);
}

void UCore_DestructionSystem::SetupCollisionResponse()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find static mesh component and bind collision
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->OnComponentHit.AddDynamic(this, &UCore_DestructionSystem::OnHit);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        
        UE_LOG(LogTemp, Log, TEXT("Collision response setup for %s"), *GetOwner()->GetName());
    }
}

void UCore_DestructionSystem::ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (!bCanBeDestroyed || bIsDestroyed)
    {
        return;
    }
    
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    
    UE_LOG(LogTemp, Log, TEXT("Damage applied to %s: %f (Health: %f/%f)"), 
           *GetOwner()->GetName(), DamageAmount, CurrentHealth, MaxHealth);
    
    // Check if destruction threshold is reached
    if (CurrentHealth <= 0.0f || DamageAmount >= DestructionSettings.DestructionThreshold)
    {
        TriggerDestruction(ImpactLocation, ImpactDirection);
    }
}

void UCore_DestructionSystem::TriggerDestruction(const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (bIsDestroyed || !bCanBeDestroyed)
    {
        return;
    }
    
    bIsDestroyed = true;
    
    UE_LOG(LogTemp, Log, TEXT("Destruction triggered for %s at location %s"), 
           *GetOwner()->GetName(), *ImpactLocation.ToString());
    
    // Create fragments if enabled
    if (DestructionSettings.bCreateDebris)
    {
        CreateFragments(ImpactLocation, ImpactDirection);
    }
    
    // Broadcast destruction event
    OnDestruction.Broadcast(GetOwner(), ImpactLocation);
    
    // Hide or destroy the original actor
    if (GetOwner())
    {
        GetOwner()->SetActorHiddenInGame(true);
        GetOwner()->SetActorEnableCollision(false);
        
        // Schedule cleanup
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(CleanupTimerHandle, 
                this, &UCore_DestructionSystem::CleanupFragments, 
                DestructionSettings.FragmentLifetime, false);
        }
    }
}

void UCore_DestructionSystem::CreateFragments(const FVector& ImpactLocation, const FVector& ImpactDirection)
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    UStaticMeshComponent* OriginalMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
    {
        UE_LOG(LogTemp, Warning, TEXT("No static mesh found for fragmentation"));
        return;
    }
    
    // Get original bounds for fragment sizing
    FVector OriginalExtent = OriginalMesh->Bounds.BoxExtent;
    float BaseScale = FMath::Min(OriginalExtent.X, FMath::Min(OriginalExtent.Y, OriginalExtent.Z)) / 100.0f;
    
    // Create fragments around impact location
    for (int32 i = 0; i < DestructionSettings.FragmentCount; i++)
    {
        // Random position around impact
        FVector RandomOffset = FVector(
            FMath::RandRange(-OriginalExtent.X, OriginalExtent.X),
            FMath::RandRange(-OriginalExtent.Y, OriginalExtent.Y),
            FMath::RandRange(-OriginalExtent.Z, OriginalExtent.Z)
        ) * 0.5f;
        
        FVector FragmentLocation = ImpactLocation + RandomOffset;
        
        // Random velocity based on impact direction
        FVector RandomVelocity = ImpactDirection.GetSafeNormal() * FMath::RandRange(200.0f, 800.0f);
        RandomVelocity += FVector(
            FMath::RandRange(-300.0f, 300.0f),
            FMath::RandRange(-300.0f, 300.0f),
            FMath::RandRange(100.0f, 500.0f)
        );
        
        // Random scale for variety
        float FragmentScale = BaseScale * FMath::RandRange(0.3f, 0.8f);
        
        AActor* Fragment = CreateFragment(FragmentLocation, RandomVelocity, FragmentScale);
        if (Fragment)
        {
            CreatedFragments.Add(Fragment);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d fragments for %s"), 
           CreatedFragments.Num(), *GetOwner()->GetName());
}

AActor* UCore_DestructionSystem::CreateFragment(const FVector& Location, const FVector& Velocity, float Scale)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Create a simple fragment actor
    AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!Fragment)
    {
        return nullptr;
    }
    
    UStaticMeshComponent* FragmentMesh = Fragment->GetStaticMeshComponent();
    if (FragmentMesh)
    {
        // Try to use a simple cube mesh for fragments
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            FragmentMesh->SetStaticMesh(CubeMesh);
            FragmentMesh->SetWorldScale3D(FVector(Scale));
            
            // Enable physics
            FragmentMesh->SetSimulatePhysics(true);
            FragmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            FragmentMesh->SetCollisionResponseToAllChannels(ECR_Block);
            
            // Apply initial velocity if requested
            if (DestructionSettings.bApplyImpulseToFragments)
            {
                FragmentMesh->AddImpulse(Velocity, NAME_None, true);
            }
            
            // Set material to match original if possible
            UStaticMeshComponent* OriginalMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
            if (OriginalMesh && OriginalMesh->GetMaterial(0))
            {
                FragmentMesh->SetMaterial(0, OriginalMesh->GetMaterial(0));
            }
        }
    }
    
    // Schedule fragment destruction
    Fragment->SetLifeSpan(DestructionSettings.FragmentLifetime);
    
    return Fragment;
}

void UCore_DestructionSystem::CleanupFragments()
{
    for (AActor* Fragment : CreatedFragments)
    {
        if (IsValid(Fragment))
        {
            Fragment->Destroy();
        }
    }
    
    CreatedFragments.Empty();
    
    // Also destroy the original actor
    if (GetOwner())
    {
        GetOwner()->Destroy();
    }
}

void UCore_DestructionSystem::SetDestructionSettings(const FCore_DestructionSettings& NewSettings)
{
    DestructionSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Destruction settings updated for %s"), *GetOwner()->GetName());
}

float UCore_DestructionSystem::GetHealthPercentage() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    
    return CurrentHealth / MaxHealth;
}

bool UCore_DestructionSystem::IsDestroyed() const
{
    return bIsDestroyed;
}

void UCore_DestructionSystem::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bCanBeDestroyed || bIsDestroyed || !OtherActor)
    {
        return;
    }
    
    // Calculate damage based on impact force
    float ImpactForce = NormalImpulse.Size();
    float Damage = FMath::Max(0.0f, ImpactForce - 100.0f) * 0.1f; // Threshold and scaling
    
    if (Damage > 0.0f)
    {
        FVector ImpactDirection = NormalImpulse.GetSafeNormal();
        ApplyDamage(Damage, Hit.Location, ImpactDirection);
        
        UE_LOG(LogTemp, Log, TEXT("Impact damage: %f from %s"), Damage, *OtherActor->GetName());
    }
}